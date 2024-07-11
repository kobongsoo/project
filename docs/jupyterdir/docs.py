import torch
import time
import os
import numpy as np
import random
import asyncio
import threading
import httpx
import uvicorn
import io

from enum import Enum
from typing import Union
from pydantic import BaseModel
from fastapi import FastAPI, File, UploadFile, Query, Cookie, Form, Request, HTTPException, BackgroundTasks
from fastapi.responses import JSONResponse, HTMLResponse
from fastapi.templating import Jinja2Templates
from utils import MyUtils, delete_local_file
from es import My_ElasticSearch
from langchain_community.embeddings import (
    HuggingFaceEmbeddings,
    HuggingFaceBgeEmbeddings,
)

from vision import MY_Vision
from docs_func import check_mime_type, getfilePath_doc01, extract_save_doc01, search_docs01, embedding_file_doc01, embedding_folder_doc01

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

# global 인스턴스 dict로 정의
global_instance:dict = {'myutils': myutils, 'settings': settings, 'myvision': myvision, 'shaai': shaai, "embedding": embedding, "myes": myes}

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
    
    elapsed_time = "{:.2f}".format(time.time() - start_time)
    
    response = {"num": f"{len(response)}", "response": f"{response}", "time": f"{elapsed_time}"}
    return JSONResponse(content=response)       
#---------------------------------------------------------------
# == 임베딩 검색 ==
@app.get("/search01")
async def search01(request:Request, file_path:str):
    assert file_path, f'file_path is empty'
    myutils.log_message(f'[/search01] file_path:{file_path}')
    return JSONResponse(content=search_docs01(instance=global_instance, file_path=file_path))        
#---------------------------------------------------------------
# == 폴더에 있는 모든 파일 임베딩 값을 구함 ==
@app.get("/embed01")
async def embed01(request:Request, user_id:str, file_folder:str='../../data11/docs', del_index:bool=False):
    assert user_id, f'user_id is empty'
    myutils.log_message(f'\n[info][embed01] user_id:{user_id}, del_index:{del_index}\n')
 
    return JSONResponse(content=embedding_folder_doc01(global_instance, file_folder, del_index))
#---------------------------------------------------------------
# == 파일업로드 후 검색함 ==================
@app.post("/upload_search01")
async def upload_search01(file: UploadFile = File(...), file_folder:str='../../data11/docs'):
    
    assert file_folder, f'file_folder is empty'

    # ==이미지 타입인지 확인================
    mime_type = check_mime_type(file.content_type)    
    
    # ==원본파일 경로 얻어와서 파일저장================
    srcPath, tgtPath = getfilePath_doc01(file.filename, file_folder, mime_type)
    myutils.log_message(f'[/upload01] srcPath:{srcPath}, tgtPath:{tgtPath}, mime_type:{mime_type}')   
    with open(srcPath, "wb") as f:
        content = await file.read()
        f.write(content)
    
    # ==원본파일에서 text 추출 후 파일로 저장============
    error, imageToText = extract_save_doc01(global_instance, srcPath, tgtPath, mime_type)
    if error != 0:
        delete_local_file(srcPath) # 검색 후 추출한 파일 삭제
        #delete_local_file(tgtPath) # 검색후 추출한 파일 삭제
        return JSONResponse(content={"error": f"{error}", "msg": f"{imageToText}"}) 
    
    # == 임베딩 & 검색==============
    response = search_docs01(instance=global_instance, file_path=tgtPath)
    
    delete_local_file(srcPath) # 검색 후 추출한 파일 삭제
    #delete_local_file(tgtPath) # 검색후 추출한 파일 삭제
 
    return JSONResponse(content=response) 
#---------------------------------------------------------------
# == 업로드 text ==
@app.post("/upload01")
async def upload01(file: UploadFile = File(...), file_folder:str='../../data11/docs', bisEmbedding:bool=False):
    assert file_folder, f'file_folder is empty'
    start_time = time.time()
    
    # ==이미지 타입인지 확인================
    mime_type = check_mime_type(file.content_type)    
    
    # ==원본파일 경로 얻어와서 파일저장================
    srcPath, tgtPath = getfilePath_doc01(file.filename, file_folder, mime_type)
    myutils.log_message(f'[/upload01] srcPath:{srcPath}, tgtPath:{tgtPath}, mime_type:{mime_type}')   
    with open(srcPath, "wb") as f:
        content = await file.read()
        f.write(content)
    
    # ==원본파일에서 text 추출 후 파일로 저장============
    error, imageToText = extract_save_doc01(global_instance, srcPath, tgtPath, mime_type)
   
    # == 임베딩 처리 =========================================
    if bisEmbedding == True:
        error,esdoc=embedding_file_doc01(global_instance, tgtPath) # error:int, esdoc:dict={}
        if error == 0:
            myes.RAG_bulk(docs=[esdoc])
            myutils.log_message(f'[/upload01] *bulk:{len([esdoc])}')
        
        elapsed_time = "{:.2f}".format(time.time() - start_time)  
        response = {"error": f"{error}", "extrafilePath": f"{tgtPath}", "mime_type": f"{mime_type}", "res": f"{esdoc}", "time": f"{elapsed_time}"}
        
        delete_local_file(tgtPath) # 임베딩 후에는 추출한 파일 삭제
    else:
        elapsed_time = "{:.2f}".format(time.time() - start_time)  
        response = {"error": f"{error}", "extrafilePath": f"{tgtPath}", "mime_type": f"{mime_type}", "res": f"{imageToText}", "time": f"{elapsed_time}"}
    
    delete_local_file(srcPath)    # 원본파일 삭제
    
    return JSONResponse(content=response)   
#---------------------------------------------------------------