import os
import time
import numpy as np
import sys
import json

sys.path.append('..')
from embedding import embedding_pdf
from utils import weighted_reciprocal_rank_fusion, clustering_embedding

#------------------------------------------------------------------
# == file 1개 임베딩 ============
def embedding_file_doc01(instance:dict, file_path:str):
    
    assert file_path, f"file_path is empty!!"
    esdocs:list=[]
    
    myutils = instance['myutils']
    myes = instance['myes']
    embedding = instance['embedding']
    
    settings = myutils.get_options()
    chunk_size = settings['CHUNK_SIZE']
    chunk_overlap = settings['CHUNK_OVERLAP']
    index_name = settings['ES_INDEX_NAME']
    es_batch_size = settings['ES_BATCH_SIZE']
    upload_file_type = 1
    NUM_CLUSTERS = settings['NUM_CLUSTERS']        # 클러스터링 계수
    FLOAT_TYPE = settings['FLOAT_TYPE']    # 클러스터링 할때 벡터 타입(float32, float16)
    OUTMODE = settings['OUTMODE']          # 클러스터링 할때 타입: mean=평균, max=최대값
    
    # ==== 임베딩 ==================
    try:
        # => langchain 이용. splitter 후 임베딩 함
        docs_vectors, docs = embedding_pdf(huggingfaceembeddings=embedding,
                              file_path=file_path, 
                              chunk_size=chunk_size, 
                              chunk_overlap=chunk_overlap,
                              upload_file_type=upload_file_type)
                
        docs_vectors_array = np.array(docs_vectors) # docks_vectors는 list이므로 array로 변경해 줌
                
    except Exception as e:
        msg = f'*embedding_pdf is Fail!!..(error: {e})'
        myutils.log_message(f'[embedding_file_doc01] {msg}')
        return 1001, msg
    # ==============================
    
    # ==== 클러스터링 처리 =========
    try:
        emb = clustering_embedding(embeddings=docs_vectors_array, 
                                   outmode=OUTMODE, num_clusters=NUM_CLUSTERS, seed=settings["SEED"]).astype(FLOAT_TYPE) 
                
    except Exception as e:
        msg = f'*clustering_embedding is Fail!!..(error: {e})'
        myutils.log_message(f'[embedding_file_doc01] {msg}')
        return 1002, msg
    # ==============================
    
    myutils.log_message(f'==='*50)
    myutils.log_message(f'[embedding_file_doc01] *file_path:{file_path}, docs 벡터 수:{len(docs_vectors)}, emb 벡터수: {len(emb)}\n')
    #myutils.log_message(f'{emb}\n\n')
    
    # ==== ES 인덱싱 =================
    try:
        esdoc = {}
                
        file_name = os.path.basename(file_path) # 파일명만 뽑아냄
        esdoc['rfile_name'] = file_name
                
        with open(file_path, 'r', encoding='utf-8') as f: # 파일내용추가
            data = f.read()
        esdoc['rfile_text'] = data
                
        # vector 0~xx 까지 값을 0으로 초기화 해줌.
        dim_size = len(emb[0])
        myutils.log_message(f'*dim_size: {dim_size}\n')
        for i in range(NUM_CLUSTERS+1):
            esdoc["vector"+str(i)] = np.zeros((dim_size))
            
        # vector0에는 평균 임베딩 값을 담음.
        avg_emb = docs_vectors_array.mean(axis=0).reshape(1,-1) #(128,) 배열을 (1,128) 형태로 만들기 위해 reshape 해줌
        esdoc["vector0"] = avg_emb[0]
                
        # vector 1~xx 까지 vector 값들을 담음.
        for i, dense_vector in enumerate(emb):
            esdoc["vector"+str(i+1)] = dense_vector
            
        esdoc['_op_type'] = "index"
        esdoc['_index'] = index_name
       
        return 0, esdoc 
        
    except Exception as e:
        msg = f'*create docs vector is Fail!!..(error: {e})'
        myutils.log_message(f'[embedding_file_doc01] {msg}')
        return 1003, msg
     
#------------------------------------------------------------------------------------
# == folder에 파일 목록을 얻어와서 임베딩처리
def embedding_folder_doc01(instance:dict, file_folder:str, del_index:bool=False):
           
    assert file_folder, f"file_folder is empty"
    start_time = time.time()
    
    myutils = instance['myutils']
    myes = instance['myes']
        
    settings = myutils.get_options()
    es_batch_size = settings['ES_BATCH_SIZE']
    
    # True이면 삭제후 재 생성함
    if del_index == True:
        myes.delete_index()
        myes.create_index()
        myutils.log_message(f'\n[info][embedding_folder_doc01] **delete index**\n')
        
    num:int = 0
    esdocs:list=[]
    count:int = 0
    
    file_paths = myutils.getListOfFiles(file_folder) # 폴더에 파일 path 얻어옴.
    for idx, file_path in enumerate(file_paths):
        if '.ipynb_checkpoints' not in file_path:
            # ./files/out 폴더에 추출한 text 파일들을 불러와서 임베딩 벡터 생성함.
            # 파일별 임베딩 함
            try:
                error, doc = embedding_file_doc01(instance, file_path)
                if error == 0:
                    count += 1
                    if len(doc) > 0:
                        esdocs.append(doc)
                        
                # batch_size 만큼씩 한꺼번에 es로 insert 시킴.
                if count % es_batch_size == 0:
                    num += 1
                    myes.RAG_bulk(docs=esdocs)
                    myutils.log_message(f'[embedding_folder_doc01] *bulk_{num}:{len(esdocs)}')
                    esdocs = []
                
            except Exception as e:
                msg = f'*RAG_bulk is Fail!!..(error: {e})'
                myutils.log_message(f'[embedding_folder_doc01] {msg}')
                response = {"error":1003, "response": f"{msg}", "time": "0"}
                return response
            
    # = 마지막에 한번더 남아있는거 인덱싱 ==
    if esdocs:
        num += 1
        myes.RAG_bulk(docs=esdocs)
        myutils.log_message(f'[embedding_folder_doc01] *bulk_{num}(last):{len(esdocs)}')
    # ====================================         
                        
    elapsed_time = "{:.2f}".format(time.time() - start_time)
    response = {"error":0, "response": f"{count}", "time": f"{elapsed_time}"}
    return response
    