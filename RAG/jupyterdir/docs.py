import torch
import argparse
import time
import os
import platform
import numpy as np
import random
import asyncio
import threading
import httpx
import openai    
import uvicorn
import warnings
import uuid
import io

from enum import Enum
from typing import Union
from pydantic import BaseModel
from fastapi import FastAPI, File, UploadFile, Query, Cookie, Form, Request, HTTPException, BackgroundTasks
from fastapi.responses import JSONResponse, HTMLResponse
from fastapi.templating import Jinja2Templates
from PyPDF2 import PdfReader
from utils import MyUtils, weighted_reciprocal_rank_fusion, generate_text_GPT2, clustering_embedding
from es import My_ElasticSearch, make_max_query_script, make_max_query_vectors_script, make_max_query_vectors_and_avg_script
from langchain_community.embeddings import (
    HuggingFaceEmbeddings,
    HuggingFaceBgeEmbeddings,
)

from embedding import embedding_pdf
from vision import MY_Vision

#-----------------------------------------------
# Mpower Synap 추가
from os import sys
sys.path.append('../../MpowerAI')
from pympower.classes.mshaai import MShaAI

shaai = MShaAI()
#-----------------------------------------------

# settings.yaml 설정값 불러옴.
myutils = MyUtils(yam_file_path='./data/docs_settings.yaml')
settings = myutils.get_options()

#-----------------------------------------------
# vision 모델 로딩
myvision = MY_Vision(model_folder_path=settings['VISION_MODEL'], device=settings['VISION_DEVICE'])
#-----------------------------------------------
app=FastAPI() # app 인스턴스 생성
templates = Jinja2Templates(directory="template_files") # html 파일이 있는 경로를 지정.

class FieldsIn(BaseModel):
    fields: list       # uid(문서 고유id)->rfilename
    
UPLOAD_DIR = "./uploads"
os.makedirs(UPLOAD_DIR, exist_ok=True)

# 임베딩 모델 로딩
# ./cache/ 경로에 다운로드 받도록 설정
os.environ["HF_HOME"] = "./cache/"

from langchain_community.embeddings import (
    HuggingFaceEmbeddings,
    HuggingFaceBgeEmbeddings,
)

print(f'*임베딩 모델 {settings["EMBEDDING_MODEL"]} 로딩 시작==>')

# model_name = 없으면: 기본 'sentence-transformers/all-mpnet-base-v2' 모델임.
#embeddings = HuggingFaceEmbeddings()  # HuggingFace 임베딩을 생성합니다.
embedding = HuggingFaceEmbeddings(
    model_name=settings["EMBEDDING_MODEL"], 
    show_progress=True
)
print(f'*임베딩 모델: {embedding.model_name}')

# elastic search 설정
es_url=settings['ES_URL']
es_index_name=settings['ES_INDEX_NAME']
es_index_file_path=settings['ES_INDEX_FILE_PATH']
myes = My_ElasticSearch(es_url=es_url, index_name=es_index_name, index_file_path=es_index_file_path)
print(f'*ES정보: {myes}')

#---------------------------------------------------------------
@app.get("/")  # 경로 동작 데코레이터 작성
async def root(): # 경로 동작 함수 작성
	return {"msg": "Document classification"}

#---------------------------------------------------------------
# == 인덱싱 파일 목록 얻기 ==
@app.get("/list01")
async def list01(request:Request):
    start_time = time.time()
        
    script_query = {"match_all": {}}
    response = myes.search_docs(script_query=script_query, k=500)
    
    end_time = time.time()
    elapsed_time = "{:.2f}".format(end_time - start_time)
    
    response = {"num": f"{len(response)}", "response": f"{response}", "time": f"{elapsed_time}"}
    
    return JSONResponse(content=response) 

    return
#---------------------------------------------------------------
# == 임베딩 검색 ==
@app.get("/search01")
async def search01(request:Request, file_path:str):
    assert file_path, f'file_path is empty'
    myutils.log_message(f'[info][/search01] file_path:{file_path}')
        
    start_time = time.time()
    
    settings = myutils.get_options()
    chunk_size = settings['CHUNK_SIZE']
    chunk_overlap = settings['CHUNK_OVERLAP']
    index_name = settings['ES_INDEX_NAME']
    es_batch_size = settings['ES_BATCH_SIZE']
    
    upload_file_type = 1
    
    # ==== 임베딩 ==================
    try:
               
        # => langchain 이용. splitter 후 임베딩 함
        docs_vectors, docs = embedding_pdf(huggingfaceembeddings=embedding,
                              file_path=file_path, 
                              chunk_size=chunk_size, 
                              chunk_overlap=chunk_overlap,
                              upload_file_type=upload_file_type)
                
    except Exception as e:
        msg = f'*embedding_pdf is Fail!!..(error: {e})'
        myutils.log_message(f'[info][/search01] {msg}')
        response = {"error": f"{msg}"}
        return JSONResponse(content=response) 
    # ==============================
    
    # ==== 클러스터링 처리 =========
    try:
                
        FLOAT_TYPE = 'float16'  # float16형
        OUTMODE = "mean"        # 평균=mean, 최대값=max
        NUM_CLUSTERS = 10       # 클러스터링 계수
        docs_vectors_array = np.array(docs_vectors) # docks_vectors는 list이므로 array로 변경해 줌
                
        emb = clustering_embedding(embeddings=docs_vectors_array, 
                                outmode=OUTMODE, num_clusters=NUM_CLUSTERS, seed=settings["SEED"]).astype(FLOAT_TYPE) 
                
    except Exception as e:
        msg = f'*clustering_embedding is Fail!!..(error: {e})'
        myutils.log_message(f'[info][/search01] {msg}')
        response = {"error": f"{msg}"}
        return JSONResponse(content=response) 
    # ==============================
    
    # ==== ES로 쿼리 ===============
    uids = []
    # NumPy 배열을 리스트로 변환
    emb_list = emb.tolist()

  
    # max 쿼리를 만듬.
    script_query = make_max_query_vectors_script(query_vectors=emb_list, vectornum=10, uid_list=uids) 
    #myutils.log_message(f'\n[info][search01] *script_query:\n{script_query}\n')
        
    response = myes.search_docs(script_query=script_query, k=100)
    # ==============================
    
    end_time = time.time()
    elapsed_time = "{:.2f}".format(end_time - start_time)
    
    response = {"response": f"{response}", "time": f"{elapsed_time}"}
    
    return JSONResponse(content=response) 
        
#---------------------------------------------------------------
# == 각 파일에 임베딩 값을 구함 ==
@app.get("/embed01")
async def embed01(request:Request, user_id:str, file_folder:str='../../data11/docs', del_index:bool=False):
    assert user_id, f'user_id is empty'
    start_time = time.time()
    
    myutils.log_message(f'\n[info][embed01] user_id:{user_id}, del_index:{del_index}\n')
        
    settings = myutils.get_options()
    chunk_size = settings['CHUNK_SIZE']
    chunk_overlap = settings['CHUNK_OVERLAP']
    index_name = settings['ES_INDEX_NAME']
    es_batch_size = settings['ES_BATCH_SIZE']
    upload_file_type = 1
    
    # True이면 삭제후 재 생성함
    if del_index == True:
        myes.delete_index()
        myes.create_index()
        myutils.log_message(f'\n[info][embed01] **delete index**\n')
        
    num:int = 0
    esdocs:list=[]
    count:int = 0
    
    file_paths = myutils.getListOfFiles(file_folder) # 폴더에 파일 path 얻어옴.
    for idx, file_path in enumerate(file_paths):
        if '.ipynb_checkpoints' not in file_path:
            # ./files/out 폴더에 추출한 text 파일들을 불러와서 임베딩 벡터 생성함.
            
            # ==== 임베딩 ==================
            try:
               
                # => langchain 이용. splitter 후 임베딩 함
                docs_vectors, docs = embedding_pdf(huggingfaceembeddings=embedding,
                              file_path=file_path, 
                              chunk_size=chunk_size, 
                              chunk_overlap=chunk_overlap,
                              upload_file_type=upload_file_type)
                
            except Exception as e:
                msg = f'*embedding_pdf is Fail!!..(error: {e})'
                myutils.log_message(f'[info][/embed01] {msg}')
                response = {"error": f"{msg}"}
                return JSONResponse(content=response) 
            # ==============================
            
            # ==== 클러스터링 처리 =========
            try:
                
                FLOAT_TYPE = 'float16'  # float16형
                OUTMODE = "mean"        # 평균=mean, 최대값=max
                NUM_CLUSTERS = 10       # 클러스터링 계수
                docs_vectors_array = np.array(docs_vectors) # docks_vectors는 list이므로 array로 변경해 줌
                
                emb = clustering_embedding(embeddings=docs_vectors_array, 
                                           outmode=OUTMODE, num_clusters=NUM_CLUSTERS, seed=settings["SEED"]).astype(FLOAT_TYPE) 
                
            except Exception as e:
                msg = f'*clustering_embedding is Fail!!..(error: {e})'
                myutils.log_message(f'[info][/embed01] {msg}')
                response = {"error": f"{msg}"}
                return JSONResponse(content=response) 
            # ==============================
            myutils.log_message(f'==='*50)
            myutils.log_message(f'[info][/embed01] *file_path:{file_path}, docs 벡터 수:{len(docs_vectors)}, emb 벡터수: {len(emb)}\n')
            myutils.log_message(f'{emb}\n\n')
                
            # ==== ES 인덱싱 =================
            try:
                esdoc = {}
                
                file_name = os.path.basename(file_path) # 파일명만 뽑아냄
                esdoc['rfile_name'] = file_name
                
                with open(file_path, 'r', encoding='utf-8') as f: # 파일내용추가
                    data = f.read()
                esdoc['rfile_text'] = data
                
                # vector 1~xx 까지 값을 0으로 초기화 해줌.
                dim_size = len(emb[0])
                myutils.log_message(f'*dim_size: {dim_size}\n')
                for i in range(NUM_CLUSTERS):
                    esdoc["vector"+str(i+1)] = np.zeros((dim_size))
            
                # vector 값들을 담음.
                for i, dense_vector in enumerate(emb):
                    esdoc["vector"+str(i+1)] = dense_vector
            
                esdoc['_op_type'] = "index"
                esdoc['_index'] = index_name
                
                #myutils.log_message(f'[info][/embed01] *esdoc\n{esdoc}')
                esdocs.append(esdoc)
                count += 1
                
                # batch_size 만큼씩 한꺼번에 es로 insert 시킴.
                if count % es_batch_size == 0:
                    num += 1
                    myes.RAG_bulk(docs=esdocs)
                    myutils.log_message(f'[info][/embed01] *bulk_{num}:{len(esdocs)}')
                    esdocs = []
                # ==============================
            except Exception as e:
                msg = f'*RAG_bulk is Fail!!..(error: {e})'
                myutils.log_message(f'[info][/embed01] {msg}')
                response = {"error": f"{msg}"}
                return JSONResponse(content=response) 
            
    # = 마지막에 한번더 남아있는거 인덱싱 ==
    if esdocs:
        num += 1
        myes.RAG_bulk(docs=esdocs)
        myutils.log_message(f'[info][/embed01] *bulk_{num}(last):{len(esdocs)}')
    # ====================================
            
    end_time = time.time()
    elapsed_time = "{:.2f}".format(end_time - start_time)
    
    response = {"*index_file_num": f"{count}", "time": f"{elapsed_time}"}
    
    return JSONResponse(content=response)

#---------------------------------------------------------------
# 이미지 type인지 체크
def check_image_type(mime_type:str):
    
    assert mime_type, f"mime_type is empty!!"
    
    bimage:bool = False
    if mime_type == "image/jpeg" or mime_type == "image/png":
        bimage = True
    return bimage
#---------------------------------------------------------------
# == 파일업로드 후 search text ==
@app.post("/upload_search01")
async def upload01(file: UploadFile = File(...), folder_path:str='../../data11/docs'):
    
    start_time = time.time()
    tmp_folder
    # ==이미지 타입인지 확인================
    bimage = check_image_type(file.content_type)    
    myutils.log_message(f'[info][/upload_search01] *mime_type:{file.content_type}')
    
    # =====원본 파일 저장===================
    if not os.path.exists(folder_path):
        os.makedirs(folder_path)
        
    # 파일명을 얻어와서 저장함.
    file_location = f"{folder_path}/{file.filename}"
    with open(file_location, "wb") as f:
        content = await file.read()
        f.write(content)
    # ====================================

    extract_folder = f"{folder_path}/extract"
    tgtPath = f"{extract_folder}/{file.filename}"
    myutils.log_message(f'[info][/upload_search01] *tgtPath:{tgtPath}')
    
    if not os.path.exists(extract_folder):
        os.makedirs(extract_folder)
            
    # =====파일에서 text 추출/저장=========
    # 파일이 있고 이미지 파일이 아닌 경우에만 text 추출함.
    if os.path.isfile(file_location) and bimage == False:
        myutils.log_message(f'[info][/upload_search01] *shaai.extract:{tgtPath}')
        shaai.extract(srcPath=file_location, tgtPath=tgtPath)
    # ====================================
    
    # ==이미지는 vision모델로 text 생성/저장==
    if bimage == True:
        res = myvision.generate(image_path=file_location)
        
        # text 생성이 너무 짧은경우 1번더 다른 prompt로 text 생성함.
        if len(res) < 20:
            res += myvision.generate(image_path=file_location, prompt="What does the image show?")
        
        # 파일로 저장
        with open(tgtPath, 'w', encoding='utf-8') as file:
            file.write(res)
        
        myutils.log_message(f'[info][/upload_search01] *myvision.generate:\n{res}')
    # ====================================
    
    # ==== 임베딩 ==================
    settings = myutils.get_options()
    chunk_size = settings['CHUNK_SIZE']
    chunk_overlap = settings['CHUNK_OVERLAP']
    index_name = settings['ES_INDEX_NAME']
    es_batch_size = settings['ES_BATCH_SIZE']
    upload_file_type = 1
    
    try:
               
        # => langchain 이용. splitter 후 임베딩 함
        docs_vectors, docs = embedding_pdf(huggingfaceembeddings=embedding,
                              file_path=tgtPath, 
                              chunk_size=chunk_size, 
                              chunk_overlap=chunk_overlap,
                              upload_file_type=upload_file_type)
                
    except Exception as e:
        msg = f'*embedding_pdf is Fail!!..(error: {e})'
        myutils.log_message(f'[info][/upload_search01] {msg}')
        response = {"error": f"{msg}"}
        return JSONResponse(content=response) 
    # ==============================
    
    # ==== 클러스터링 처리 =========
    try:
                
        FLOAT_TYPE = 'float16'  # float16형
        OUTMODE = "mean"        # 평균=mean, 최대값=max
        NUM_CLUSTERS = 10       # 클러스터링 계수
        docs_vectors_array = np.array(docs_vectors) # docks_vectors는 list이므로 array로 변경해 줌
                
        emb = clustering_embedding(embeddings=docs_vectors_array, 
                                outmode=OUTMODE, num_clusters=NUM_CLUSTERS, seed=settings["SEED"]).astype(FLOAT_TYPE) 
                
    except Exception as e:
        msg = f'*clustering_embedding is Fail!!..(error: {e})'
        myutils.log_message(f'[info][/upload_search01] {msg}')
        response = {"error": f"{msg}"}
        return JSONResponse(content=response) 
    # ==============================
    
    # ==== ES로 쿼리 ===============
    uids = []
    # NumPy 배열을 리스트로 변환
    emb_list = emb.tolist()
    
    # max 쿼리를 만듬.
    script_query = make_max_query_vectors_and_avg_script(query_vectors=emb_list, vectornum=10, uid_list=uids) 
    #myutils.log_message(f'\n[info][search01] *script_query:\n{script_query}\n')
        
    response = myes.search_docs(script_query=script_query, k=100)
    # ==============================
    
    end_time = time.time()
    elapsed_time = "{:.2f}".format(end_time - start_time)
    
    response = {"response": f"{response}", "time": f"{elapsed_time}"} 
    return JSONResponse(content=response) 

#---------------------------------------------------------------
# == 업로드 text ==
@app.post("/upload01")
async def upload01(file: UploadFile = File(...), file_folder:str='../../data11/docs'):
    
    start_time = time.time()
    
    # ==이미지 타입인지 확인================
    bimage = check_image_type(file.content_type)    
    myutils.log_message(f'[info][/upload01] *mime_type:{file.content_type}')
    
    # =====원본 파일 저장===================
    # 파일명을 얻어와서 저장함.
    org_folder = f"{file_folder}/org"
    if not os.path.exists(org_folder):
        os.makedirs(org_folder)
            
    file_location = f"{org_folder}/{file.filename}"
    with open(file_location, "wb") as f:
        content = await file.read()
        f.write(content)
    # ====================================

    # 추출 및 생성한 text 저장할 폴더 지정
    extract_folder = f"{file_folder}/extract"
    tgtPath = f"{extract_folder}/{file.filename}"
    if not os.path.exists(extract_folder):
        os.makedirs(extract_folder)
            
    # =====파일에서 text 추출/저장=========
    # 파일이 있고 이미지 파일이 아닌 경우에만 text 추출함.
    if os.path.isfile(file_location) and bimage == False:
        myutils.log_message(f'[info][/upload01] *tgtPath:{tgtPath}')
        shaai.extract(srcPath=file_location, tgtPath=tgtPath)
    # ====================================
    
    # ==이미지는 vision모델로 text 생성/저장==
    if bimage == True:
        res = myvision.generate(image_path=file_location)
        
        # text 생성이 너무 짧은경우 1번더 다른 prompt로 text 생성함.
        if len(res) < 20:
            res += myvision.generate(image_path=file_location, prompt="What does the image show?")
        
        # 파일로 저장
        with open(tgtPath, 'w', encoding='utf-8') as file:
            file.write(res)
        
        myutils.log_message(f'[info][/upload01] *myvision.generate:\n{res}')
    # ====================================
    
    end_time = time.time()
    elapsed_time = "{:.2f}".format(end_time - start_time)
 
    # 출력
    if bimage == False:
        response = {"file": f"{file_location}", "mime_type":f"{file.content_type}", "time": f"{elapsed_time}"}
    else:
        response = {"file": f"{file_location}", "text":f"{res}", "time": f"{elapsed_time}"}
        
    return JSONResponse(content=response)
#---------------------------------------------------------------