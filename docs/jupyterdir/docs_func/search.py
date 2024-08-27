import os
import time
import numpy as np
import sys

sys.path.append('..')
from embedding import embedding_pdf
from utils import weighted_reciprocal_rank_fusion, clustering_embedding
from es import make_max_query_script, make_max_query_vectors_script, make_max_query_vectors_and_avg_script, make_embedding_query_script

#---------------------------------------------------------------
# == 이미 임베딩된 벡터로 검색 하는 함수 ==
# => rfile_name을 입력하면 인덱스에서 검색해서 저장된 벡터를 불러와서, 벡터를 가지고 유사한 문서 검색함.
# in: instance
# in: rfile_name : 검색할 rfile_name(문서명(식별자))
#---------------------------------------------------------------
def embed_search_docs01(instance:dict, rfile_name:str):
    assert rfile_name, f'rfile_name is empty'
    
    start_time = time.time()
    
    myutils = instance['myutils']
    myes = instance['myes']
    embedding = instance['embedding']
     
    settings = myutils.get_options()
    SEARCH_EMBED_TYPE = settings['SEARCH_EMBED_TYPE'] # 임베딩 검색 방식 : 0=다대다: 클러스터링 임베딩 검색(mean), 1=일대일:평균 임베딩 검색, 2=1:일대다:평균 임베딩 검색
    NUM_CLUSTERS = settings['NUM_CLUSTERS']        # 클러스터링 계수
    SEARCH_K = settings['SEARCH_K'] + 1       # 검색 계수 +1 해줌(*자신도 검색되므로,1=2로 검색해야 함)
    embedding_search_min_score = settings['EMBEDDING_SEARCH_MIN_SCORE']
     
    # ==== rfle_name으로 검색===============    
    # => term으로 해서 rfile_name 풀경로가 모두 일치하는 경우만 검색함.
    body = {
        "size": 1,
        "query": {"term": {"rfile_name": rfile_name}},
        "_source": {"includes": ["rfile_name", "vector0", "vector1", "vector2", "vector3", "vector4", "vector5", "vector6", "vector7", "vector8", "vector9"]}
    }
    
    response = None
    response = myes.search(body=body)    
    #print(f"*embed_search_doc01: response:{response}")
    
    # 검색된값이 없으면 리턴
    hits = response["hits"]["hits"]
    if len(hits) < 1:
        end_time = time.time()
        elapsed_time = "{:.2f}".format(end_time - start_time)
        response = {"error": 2, "response": f"{hits}", "time": f"{elapsed_time}"}
        return response 
        
    emb_list:list = []
    docid:str="" # id 정의
    uids = []   
    
    # ==== 검색방식에 따라 쿼리 스크립트 생성 ===============
    if SEARCH_EMBED_TYPE == 0: # 다대다 쿼리(*쿼리 문서를 클러스터링해서 여러개 벡터 만들고 임베딩된문서의 다수 벡터와 비교)
        for hit in hits: 
            docid = hit["_id"]   # 문서id
            for i in range(1, 10):
                vector = hit["_source"][f"vector{i}"]
                emb_list.append(vector)  
            break
                        
        # 쿼리를 만듬.
        script_query = make_max_query_vectors_script(query_vectors=emb_list, vectornum=NUM_CLUSTERS, uid_list=uids) 
    else:
        for hit in hits: 
            docid = hit["_id"]   # 문서id
            vector = hit["_source"][f"vector0"]
            emb_list.append(vector)  
            break
             
        if SEARCH_EMBED_TYPE == 1: # 평균 쿼리 만듬.(*쿼리 문서의 평균을 구해서 임베딩된문서의 평균벡터와 비교)
            script_query = make_embedding_query_script(qr_vector=emb_list[0]) # 쿼리를 만듬.  
        else: # 1대다 쿼리 만듬.(*쿼리 문서의 평균을 구해서 임베딩된문서의 다수 벡터와 비교)
            script_query = make_max_query_script(query_vector=emb_list[0], vectornum=NUM_CLUSTERS, uid_list=[]) # 쿼리를 만듬.   

    print(f"*embed_search_doc01: docid:{docid}")
    print(f"*embed_search_doc01: script_query:{script_query}")
    
    # ==== ES로 쿼리 ===============
    # 임베딩 search
    embed_docs = myes.search_docs(script_query=script_query, k=SEARCH_K, min_score=embedding_search_min_score) 
    
    docs:list = []
    for embed_doc in embed_docs:
        doc={}
        search_docid = embed_doc['id']
        if docid != search_docid:         #**검색할 문서id와 같은경우는 pass, 다른경우에만 저장
            doc['rfile_name'] = embed_doc["rfile_name"]       
            doc['id'] = embed_doc["id"]   # 문서id
            doc['score'] = embed_doc["score"]
            docs.append(doc)

    end_time = time.time()
    elapsed_time = "{:.2f}".format(end_time - start_time)
    response = {"error": 0, "response": f"{docs}", "time": f"{elapsed_time}"}
    
    return response 
    
#---------------------------------------------------------------
# == 검색 처리 함수 ==
#---------------------------------------------------------------
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
    SEARCH_EMBED_TYPE = settings['SEARCH_EMBED_TYPE'] # 임베딩 검색 방식 : 0=다대다: 클러스터링 임베딩 검색(mean), 1=일대일:평균 임베딩 검색, 2=1:일대다:평균 임베딩 검색
    FLOAT_TYPE = settings['FLOAT_TYPE']    # 클러스터링 할때 벡터 타입(float32, float16)
    OUTMODE = settings['OUTMODE']          # 클러스터링 할때 타입: mean=평균, max=최대값
    SEARCH_K = settings['SEARCH_K']        # 검색 계수
    RRF_SEARCH = settings['RRF_SEARCH']    # 1=RRF 검색 적용, 0=임베딩 검색만 적용
    
    bm25_search_min_score = settings['BM25_SEARCH_MIN_SCORE']
    embedding_search_min_score = settings['EMBEDDING_SEARCH_MIN_SCORE']
    uid_embed_weigth = settings['RRF_BM25_WEIGTH']
    uid_bm25_weigth = settings['RRF_EMBED_WEIGTH']
    
    ##################################################################################
    # == 검색문서 split 및 임베딩 ==
    ##################################################################################
    if RRF_SEARCH == 0 or RRF_SEARCH == 1: # 임베딩검색(0) 혹은 RRF SEARCH(1)인 경우에는 임베딩함.
        no_embedding = 0
    else: #BM25 검색(2)인 경우에는 임베딩 안함.
        no_embedding = 1
        
    docs_vectors:list = []
    docs:list = []
    try:

        # => langchain 이용. splitter 후 임베딩 함
        docs_vectors, docs = embedding_pdf(huggingfaceembeddings=embedding,
                                  file_path=file_path, 
                                  chunk_size=chunk_size, 
                                  chunk_overlap=chunk_overlap,
                                  upload_file_type=upload_file_type,
                                  no_embedding = no_embedding  # 임베딩할지 안할지
                                  )

        docs_vectors_array = np.array(docs_vectors) # docks_vectors는 list이므로 array로 변경해 줌

        # docs_vectors_array 가 없으면 response에는 빈 리스트로 리턴.
        if len(docs_vectors_array) < 1 and no_embedding == 0:
            response = {"error":2000, "response": f"[]", "time": "0"}
            myutils.log_message(f'[info][/search01] {response}')
            return response

    except Exception as e:
        msg = f'*embedding_pdf is Fail!!..(error: {e})'
        myutils.log_message(f'[info][/search01] {msg}')
        response = {"error":1001, "response": f"{msg}", "time": "0"}
        return response
    ##################################################################################
    
    ##################################################################################
    # =임베딩 검색=
    # =>임베딩검색(0) 혹은 RRF SEARCH(1)인 경우에만 임베딩검색 함.
    ##################################################################################
    if RRF_SEARCH == 0 or RRF_SEARCH == 1: 

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
    ##################################################################################
    
    ##################################################################################
    # ==BM25 검색==
    # =>RRF SEARCH(1) 혹은 BM25 검색(2)인 경우에만 BM25검색 함.
    bm25_docs:list=[]
    if RRF_SEARCH == 1 or RRF_SEARCH == 2: 
        
        # BM25 search 검색 
        query:Str = ''
        for doc in docs: # 내용을 query로 전송( query 최대 길이는 1024 넘으면 안됨)
            query += doc
            if len(query) > 1024:
                break

        if len(query) > 1024:
            query = query[:1024]
            myutils.log_message(f'\n[search_docs01] BM25_searchdoc=>query:\n{query}\n')
            
        if len(query) > 0:
            bm25_docs = myes.BM25_search_docs(query=query, k=SEARCH_K, min_score=bm25_search_min_score)
    # ==============================
    
    # ==BM25 검색(2)인 경우 bm25결과만 리턴.==
    if RRF_SEARCH == 2:
        end_time = time.time()
        elapsed_time = "{:.2f}".format(end_time - start_time)
        response = {"error": 0, "response": f"{bm25_docs}", "time": f"{elapsed_time}"}
        return response
    ##################################################################################
    
    ##################################################################################
    # =RRF=
    # =>BM25 +임베딩검색 RRF 스코어 구함
    ##################################################################################
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
     
    ##################################################################################
    
    end_time = time.time()
    elapsed_time = "{:.2f}".format(end_time - start_time)
    response = {"error": 0, "response": f"{RRF_docs}", "time": f"{elapsed_time}"}
    return response