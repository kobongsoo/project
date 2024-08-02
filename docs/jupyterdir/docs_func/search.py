import os
import time
import numpy as np
import sys

sys.path.append('..')
from embedding import embedding_pdf
from utils import weighted_reciprocal_rank_fusion, clustering_embedding
from es import make_max_query_script, make_max_query_vectors_script, make_max_query_vectors_and_avg_script, make_embedding_query_script

#---------------------------------------------------------------
# == 검색 처리 함수 ==
def search_docs01(instance:dict, file_path:str):
    assert file_path, f'file_path is empty'
        
    myutils = instance['myutils']
    myes = instance['myes']
    embedding = instance['embedding']
        
    start_time = time.time()
    upload_file_type = 1
    
    myutils.log_message(f'[search_docs01] file_path:{file_path}')

    settings = myutils.get_options()
    chunk_size = settings['CHUNK_SIZE']
    chunk_overlap = settings['CHUNK_OVERLAP']
    index_name = settings['ES_INDEX_NAME']
    es_batch_size = settings['ES_BATCH_SIZE']
    NUM_CLUSTERS = settings['NUM_CLUSTERS']        # 클러스터링 계수
    SEARCH_EMBED_TYPE = settings['SEARCH_EMBED_TYPE'] # 임베딩 검색 방식 : 0=클러스터링 임베딩 검색(MAX), 1=평균 임베딩 검색
    FLOAT_TYPE = settings['FLOAT_TYPE']    # 클러스터링 할때 벡터 타입(float32, float16)
    OUTMODE = settings['OUTMODE']          # 클러스터링 할때 타입: mean=평균, max=최대값
    SEARCH_K = settings['SEARCH_K']        # 검색 계수
    RRF_SEARCH = settings['RRF_SEARCH']    # 1=RRF 검색 적용, 0=임베딩 검색만 적용
    
    bm25_search_min_score = settings['BM25_SEARCH_MIN_SCORE']
    embedding_search_min_score = settings['EMBEDDING_SEARCH_MIN_SCORE']
    uid_embed_weigth = settings['RRF_BM25_WEIGTH']
    uid_bm25_weigth = settings['RRF_EMBED_WEIGTH']
    

    # ==== 임베딩 ==================
    try:
               
        # => langchain 이용. splitter 후 임베딩 함
        docs_vectors, docs = embedding_pdf(huggingfaceembeddings=embedding,
                              file_path=file_path, 
                              chunk_size=chunk_size, 
                              chunk_overlap=chunk_overlap,
                              upload_file_type=upload_file_type)
      
        docs_vectors_array = np.array(docs_vectors) # docks_vectors는 list이므로 array로 변경해 줌
        
        # docs_vectors_array 가 없으면 response에는 빈 리스트로 리턴.
        if len(docs_vectors_array) < 1:
            response = {"error":2000, "response": f"[]", "time": "0"}
            myutils.log_message(f'[info][/search01] {response}')
            return response
    
    except Exception as e:
        msg = f'*embedding_pdf is Fail!!..(error: {e})'
        myutils.log_message(f'[info][/search01] {msg}')
        response = {"error":1001, "response": f"{msg}", "time": "0"}
        return response
    # ==============================
    
    # ==ES 쿼리스크립트 만듬 ========
    if SEARCH_EMBED_TYPE == 0: # 다대다 쿼리(*쿼리 문서를 클러스터링해서 여러개 벡터 만들고 임베딩된문서의 다수 벡터와 비교)
        try:

            emb = clustering_embedding(embeddings=docs_vectors_array, 
                                    outmode=OUTMODE, num_clusters=NUM_CLUSTERS, seed=settings["SEED"]).astype(FLOAT_TYPE) 
            
            # NumPy 배열을 리스트로 변환
            emb_list = emb.tolist()
            uids = []
            
            # 쿼리를 만듬.
            script_query = make_max_query_vectors_script(query_vectors=emb_list, vectornum=NUM_CLUSTERS, uid_list=uids) 

        except Exception as e:
            msg = f'*clustering_embedding is Fail!!..(error: {e})'
            myutils.log_message(f'[search_docs01] {msg}')
            response = {"error":1002, "response": f"{msg}", "time": "0"}
            return response
    # ==============================
    elif SEARCH_EMBED_TYPE == 1: # 평균 쿼리 만듬.(*쿼리 문서의 평균을 구해서 임베딩된문서의 평균벡터와 비교)
        avg_emb = docs_vectors_array.mean(axis=0).reshape(1,-1) # 평균을 구함 : (128,) 배열을 (1,128) 형태로 만들기 위해 reshape 해줌          
        script_query = make_embedding_query_script(qr_vector = avg_emb[0]) # 쿼리를 만듬.   
    # ==============================
    else: # 1대다 쿼리 만듬.(*쿼리 문서의 평균을 구해서 임베딩된문서의 다수 벡터와 비교)
        avg_emb = docs_vectors_array.mean(axis=0).reshape(1,-1) # 평균을 구함 : (128,) 배열을 (1,128) 형태로 만들기 위해 reshape 해줌          
        script_query = make_max_query_script(query_vector=avg_emb[0], vectornum=NUM_CLUSTERS, uid_list=[]) # 쿼리를 만듬.   
    # ==============================
    
    # ==== ES로 쿼리 ===============
    # 임베딩 search
    embed_docs = myes.search_docs(script_query=script_query, k=SEARCH_K, min_score=embedding_search_min_score) 
   
    # 임베딩 SEARCH 만하는 경우에는 임베딩 DOCS 만 리턴 함.
    if RRF_SEARCH == 0: 
        end_time = time.time()
        elapsed_time = "{:.2f}".format(end_time - start_time)
        response = {"error": 0, "response": f"{embed_docs}", "time": f"{elapsed_time}"}
        return response 
    
    # RRF SEARCH 인 경우 
    # BM25 search 검색 
    query:Str = ''
    for doc in docs: # 내용을 query로 전송( query 최대 길이는 1024 넘으면 안됨)
        query += doc
        if len(query) > 1024:
            break
     
    if len(query) > 1024:
        query = query[:1024]
        myutils.log_message(f'\n[search_docs01] BM25_searchdoc=>query:\n{query}\n')
            
    bm25_docs = myes.BM25_search_docs(query=query, k=SEARCH_K, min_score=bm25_search_min_score)
    # ==============================
    
    # ==BM25 +임베딩검색 RRF 시킴===
    RRF_docs:list = []
    if len(embed_docs) > 0 and len(bm25_docs) > 0:
        embed_docs_name = [doc['rfile_name'] for doc in embed_docs]
        bm25_docs_name = [doc['rfile_name'] for doc in bm25_docs]
        
        RRF_scores=weighted_reciprocal_rank_fusion(lists=[embed_docs_name, bm25_docs_name], weights=[uid_embed_weigth, uid_bm25_weigth])

        # bm25_docs 와 embed_docs 두 리스트를 합쳐서 하나의 딕셔너리로 만듬.
        combined_docs = {doc['rfile_name']: doc for doc in embed_docs + bm25_docs}

        # RRF_scores에 있는 name과 일치하는 rfile_text 값을 combined_docs 리스트에서 찾아서, RRF_docs 리스트에 추가함.
        for name, RRF_score in RRF_scores:
            if name in combined_docs:
                RRF_doc = {
                    'rfile_name': combined_docs[name]['rfile_name'],  # combined_docs name
                    #'rfile_text': combined_docs[name]['rfile_text'],  # combined_docs rfile_text
                    'score': RRF_score
                }
                RRF_docs.append(RRF_doc)
     
    # ==============================
    end_time = time.time()
    elapsed_time = "{:.2f}".format(end_time - start_time)
    response = {"error": 0, "response": f"{RRF_docs}", "time": f"{elapsed_time}"}
    return response