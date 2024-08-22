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
from docs_func import check_mime_type, getfilePath_doc01, extract_save_doc01, search_docs01, embedding_file_doc01, embedding_folder_doc01, embed_search_docs01

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
# ==rfile_name으로 인덱스 정보 얻기==
# => in:rfile_name = 검색할 rfile_name(*keywaord 타입이어야함)
#---------------------------------------------------------------
@app.get("/rfile_name01")
async def rfile_name01(request:Request, rfile_name:str):
    assert rfile_name, f'rfile_name is empty!'
    start_time = time.time()
        
    response = myes.search_rfile_name_docs(rfile_name=rfile_name)
    
    elapsed_time = "{:.2f}".format(time.time() - start_time)
    
    response = {"response": f"{response}", "time": f"{elapsed_time}"}
    return JSONResponse(content=response) 
#---------------------------------------------------------------
# ==이미 인덱싱된 파일 목록 얻기==
#---------------------------------------------------------------
@app.get("/list01")
async def list01(request:Request):
    start_time = time.time()
        
    script_query = {"match_all": {}}
    response = myes.search_docs(script_query=script_query, k=500)
    
    elapsed_time = "{:.2f}".format(time.time() - start_time)
    
    response = {"num": f"{len(response)}", "response": f"{response}", "time": f"{elapsed_time}"}
    return JSONResponse(content=response)       
#---------------------------------------------------------------
# == 서버 파일 임베딩 검색 ==
# => 서버에 있는 파일 경로를 지정해서 임베딩 검색
# => in: file_path=서버에 있는 유사문서 검색할 파일 경로.
#---------------------------------------------------------------
@app.get("/search01")
async def search01(request:Request, file_path:str):
    assert file_path, f'file_path is empty'
    myutils.log_message(f'[/search01] file_path:{file_path}')
    return JSONResponse(content=search_docs01(instance=global_instance, file_path=file_path))        
#---------------------------------------------------------------
# ==서버 임베딩된 문서 벡터로 검색==
# => in:rfile_name = 검색할 rfile_name(*keywaord 타입이어야함)
#---------------------------------------------------------------
@app.get("/embed_search01")
async def embed_search01(request:Request, rfile_name:str):
    assert rfile_name, f'rfile_name is empty!'
    start_time = time.time()
        
    response = embed_search_docs01(instance=global_instance, rfile_name=rfile_name)
    
    elapsed_time = "{:.2f}".format(time.time() - start_time)
    
    response = {"response": f"{response}", "time": f"{elapsed_time}"}
    return JSONResponse(content=response) 
#---------------------------------------------------------------
# == 폴더에 있는 모든 파일 임베딩 값을 구함 ==
# => 해당 폴더에 있는 모든 파일을 임베딩함. 해당 파일들은 이미 text 추출 되어 있어야 함.
# => in:file_folder = 임베딩할 text 추출된 파일들이 있는 경로.
# => in:del_index = True이면 기존 index명과 동일한 인덱스가 ES에 있으면 제거하고 다시 임베딩함., False면 기존거 유지해서 추가 임베딩 함.
#---------------------------------------------------------------
@app.get("/embed01")
async def embed01(request:Request, file_folder:str='../../data11/docs', del_index:bool=False):
    assert file_folder, f'file_folder is empty'
    myutils.log_message(f'\n[info][embed01] file_folder:{file_folder}, del_index:{del_index}\n')
 
    return JSONResponse(content=embedding_folder_doc01(global_instance, file_folder, del_index))
#---------------------------------------------------------------
# == 파일업로드 후 검색함 ==================
# => 파일을 선택해서 업로드 하면 file_foloder 경로 + "/org" 폴더파일을 저장하고,
# 이후 저장된 파일 text 추출후 file_foloder 경로 + "/extra" 폴더에 저장후, 임베딩 후 검색함.
# => in: file_folder = 업로드된 파일이 저장될 서버폴더 경로.
#---------------------------------------------------------------
@app.post("/upload_search01")
async def upload_search01(file: UploadFile = File(...), file_folder:str='../../data11/docs'):
    
    assert file_folder, f'file_folder is empty'

    # ==이미지 타입인지 확인================
    mime_type = check_mime_type(file.content_type)    
    
    # ==원본파일 경로 얻어와서 파일저장================
    # getfilePath_doc01() 함수에서 file_folder + /org 폴더에 입력된 파일 원본이 저장되고,
    # text 추출된 파일은 file_folder + /extra 폴더에 저장된다.
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
# == 파일 업로드 하고 임베딩===
# => 파일을 선택해서 업로드 하면 file_foloder 경로 + "/org" 폴더파일을 저장하고,
# 이후 저장된 파일 text 추출후 file_foloder 경로 + "/extra" 폴더에 저장 후, 임베딩(옴션) 함. 
# 임베딩은 bisEmbedding=True 일때 임베딩 하고, False이면 text 추출만 함.
# => in: file_folder = 업로드된 파일이 저장될 서버폴더 경로.
# => in: bisEmbedding = True 이면 임베딩 함, False이면 임베딩 안하고 text 추출만 함.
#---------------------------------------------------------------
@app.post("/upload_embed01")
async def upload_embed01(file: UploadFile = File(...), file_folder:str='../../data11/docs', bisEmbedding:bool=False):
    assert file_folder, f'file_folder is empty'
    start_time = time.time()
    
    # ==이미지 타입인지 확인================
    mime_type = check_mime_type(file.content_type)    
    
    # ==원본파일 경로 얻어와서 파일저장================
    # getfilePath_doc01() 함수에서 file_folder + /org 폴더에 입력된 파일 원본이 저장되고,
    # text 추출된 파일은 file_folder + /extra 폴더에 저장된다.
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
# == 소스원본파일에서 text 추출하여 타켓폴더에 파일 저장===
# => 서버폴더에 있는 text 추출할 원본파일들을 text 추출하여 tgtFolder 경로 파일로 저장
# => in: srcFolder = text 추출할 파일이 있는 소스폴더 경로
# => in: tgtFolder = text 추출후 파일을 저장할 타겟폴더 경로
#---------------------------------------------------------------
@app.get("/extract01")
async def extract01(srcFolder:str, tgtFolder:str):
    assert srcFolder, f'srcFolder is empty'
    assert tgtFolder, f'tgtFolder is empty'
    start_time = time.time()
    
    srcFilePaths = myutils.getListOfFiles(srcFolder) # 폴더에 파일 path 얻어옴.
    
    # 파일풀경로에서 파일명만 얻어옴.
    #fileNames = [os.path.basename(path) for path in filepaths]
    
    error_count:int = 0
    
    for srcFilePath in srcFilePaths:
        fileName = os.path.basename(srcFilePath) # 파일 풀경로에서 파일명만 얻어옴.
        tgtFilePath = f"{tgtFolder}/{fileName}"
        
        # text 추출 실패나도 로그만 남기고 다음거 진행함.
        try:
            shaai.extract(srcPath=srcFilePath, tgtPath=tgtFilePath)
        except Exception as e:
            msg = f'*extract is Fail!!..(error: {e})'
            myutils.log_message(f'[info][/extract] {msg}')
            error_count += 1
   
    elapsed_time = "{:.2f}".format(time.time() - start_time)  

    response = {"count": f"{len(srcFilePaths)}", "error_count": f"{error_count}", "time": f"{elapsed_time}"}
    return JSONResponse(content=response)   
    
    