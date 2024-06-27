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
from fastapi.responses import HTMLResponse
from fastapi.templating import Jinja2Templates
from PyPDF2 import PdfReader
from utils import MyUtils, weighted_reciprocal_rank_fusion, generate_text_GPT2
from es import My_ElasticSearch
from langchain_community.embeddings import (
    HuggingFaceEmbeddings,
    HuggingFaceBgeEmbeddings,
)

from embedding import embedding_pdf


# settings.yaml 설정값 불러옴.
myutils = MyUtils(yam_file_path='./data/settings.yaml')
settings = myutils.get_options()

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

# GPT 모델 - GPT 3.5 Turbo 지정 : => 모델 목록은 : https://platform.openai.com/docs/models/gpt-4 참조                                                
openai.api_key = settings['GPT_TOKEN']# **GPT  key 지정

# pdf 파일을 불러와서 추출한 text 리턴
def output_pdf_text(file_path:str):
    assert file_path, f"[output_pdf_text] file_path is empty!"
    
    out_text = ""       
    # PDF 내용 읽기 및 page별루 Markdown tag ## 추가함
    with open(file_path, "rb") as f:
        pdf_reader = PdfReader(f)
        for page_num in range(len(pdf_reader.pages)):
            page = pdf_reader.pages[page_num]
            text = page.extract_text()

            # page 별루 Markdown tag ## 추가함
            if text:
                out_text += f"## Page {page_num + 1}\n\n{text}\n\n"
                
    return out_text

#---------------------------------------------------------------
@app.get("/")  # 경로 동작 데코레이터 작성
async def root(): # 경로 동작 함수 작성
	return {"msg": "RAG 개인문서검색 예제 World"}

#---------------------------------------------------------------
# index 목록 불러오기 
@app.get("/list")
async def list(request:Request, user_id:str='Enter UserID'):
    settings = myutils.get_options()
    return templates.TemplateResponse("list.html", {"request": request, "user_id": user_id})
#---------------------------------------------------------------   
# index 목록 검색 후 얻어오기 
@app.get("/list/search")
async def list01(request:Request, user_id:str):
    #assert user_id, f'user_id is empty'
    settings = myutils.get_options()
    field_name = settings['ES_LIST_FIELD_NAME']
    myutils.log_message(f'[info][/list/search] *user_id:{user_id}, field_name:{field_name}')

    # user_id가 '*.*'이면 모든 검색
    if user_id == '*.*':
        user_id = ""
        
    fields:list = []
    counts:list = []
    fields, counts = myes.get_list(field_name=field_name, user_id=user_id)
    myutils.log_message(f'[info][/list/search] *fields_len:{len(fields)}\n*fields:{fields}')
    myutils.log_message(f'[info][/list/search] *counts_len:{len(counts)}\n*counts:{counts}')
    
    return fields, counts
#---------------------------------------------------------------    
# 목록 삭제
@app.post("/list/del")
async def list02(request:Request, user_id:str, Data:FieldsIn):
    fields = Data.fields 
    
    myutils.log_message(f'[info][post][/list/del] *user_id:{user_id}')
    myutils.log_message(f'[info][post][/list/del] *field_len:{len(fields)}\n*fields:{fields}')

    # 1. user_id와 fields 리스트를 가지고 삭제 ES 쿼리를 만든다.
    res = myes.del_list(fields=fields, user_id=user_id)

    # res=0 성공, 그외 에러 
    return res
    
#---------------------------------------------------------------    
# 검색 창 
@app.get("/search")
async def search(request:Request, user_id:str):
     return templates.TemplateResponse("search.html", {"request": request, "user_id": user_id})
#---------------------------------------------------------------
# search
@app.get("/search/query")
async def search01(request:Request, user_id:str, query:str):
    assert user_id, f'user_id is empty'
    assert query, f'query is empty'
    print(f'*[search] user_id: {user_id}, query: {query}\n')
    
    settings = myutils.get_options()
    rag_prompt_context = settings['RAG_PROMPT_CONTEXT']
    qa_prmpt_context = settings['QA_PROMPT_CONTEXT']
    
    k = settings['SEARCH_K']
    uid_embed_weigth = settings['RRF_BM25_WEIGTH']
    uid_bm25_weigth = settings['RRF_EMBED_WEIGTH']
    bm25_search_min_score = settings['BM25_SEARCH_MIN_SCORE']
    embedding_search_min_score = settings['EMBEDDING_SEARCH_MIN_SCORE']
        
    gpt_model:str = settings['GPT_MODEL']    
    system_prompt = settings['SYSTEM_PROMPT']
    max_tokens = settings.get('GPT_MAX_TOKENS', 1024)
    temperature = settings.get('GPT_TEMPERATURE', 1.0)
    top_p = settings.get('GPT_TOP_P', 0.1)
    stream = settings.get('GTP_STREAM', False)

    # user_id가 '*.*'이면 모든 검색
    if user_id == '*.*':
        user_id = ""
        
    bm25_docs:list = []
    embed_docs:list = []
    
    # 1.ES로 BM25 검색
    bm25_docs = myes.BM25_search(query=query, user_id=user_id, k=k, min_score=bm25_search_min_score)

    # 2.ES로 임베딩 검색
    embed_docs = myes.Embedding_search(huggingfaceembeddings=embedding, query=query, user_id=user_id, k=k, min_score=embedding_search_min_score)

    # 3. BM25 + 임베딩검색 RRF 시킴
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
                    'user_id': combined_docs[name]['user_id'],
                    'rfile_name': combined_docs[name]['rfile_name'],  # combined_docs name
                    'rfile_text': combined_docs[name]['rfile_text'],  # combined_docs rfile_text
                    'score': RRF_score
                }
                RRF_docs.append(RRF_doc)
        
        
    # 4.프롬프릍 생성.
    context:str = ""
    doc_names:str = ""
    context_score:str = ""
    
    # RRF_docs 있으면 context를 문서제목+문서text 식으로 만듬
    count:int = 0
    idx:int = 0
    if len(RRF_docs) > 0:
        for idx, RRF_doc in enumerate(RRF_docs):
            count = idx+1
            doc_names += RRF_doc['rfile_name'] + '<br>'
            context += RRF_doc['rfile_text'] + '\n\n'
            context_score += f"{count}. (score:{RRF_doc['score']:.3f}){RRF_doc['rfile_name']}<br><br>{RRF_doc['rfile_text']}<br><br>"
    else: # RRF_docs 없으면 embed_doc를 context를 문서제목+문서text 식으로 만듬
        for embed_doc in embed_docs:
            count = idx+1
            doc_names += embed_doc['rfile_name'] + '<br>'
            context += embed_doc['rfile_text'] + '\n\n'
            context_score += f"{count}. (sore:{embed_doc['score']:.3f}){embed_doc['rfile_name']}<br><br>{embed_doc['rfile_text']}<br><br>"
            idx = idx+1
            
    myutils.log_message(f'========================================')
    myutils.log_message(f'\n[info][/search] *RRF_docs Len:{len(RRF_docs)}\n*BM24_doc Len:{len(bm25_docs)}\n*embed_docs Len:{len(embed_docs)}\n*doc_name:\n{doc_names}\n*context_score:\n{context_score}\n')
    myutils.log_message(f'========================================')
                        
    prompt:str = ""
    if context: # 검색된 docs가 있는 경우 rag 프롬프트 생성
        prompt = rag_prompt_context.format(query=query, context=context)
    else:       # 검색이 없는 경우 Q&A 프롬프트 생성
        prompt = qa_prmpt_context.format(query=query)

    # 4.GPT로 쿼리
    response, status = generate_text_GPT2(gpt_model=gpt_model, prompt=prompt, system_prompt=system_prompt, 
                                          assistants=[], stream=stream, timeout=20,
                                          max_tokens=max_tokens, temperature=temperature, top_p=top_p) 
    
    myutils.log_message(f'\n[info][/search] *generate_text_GPT2=>status:{status}\nresponse:\n{response}\n')
    myutils.log_message(f'========================================')
    
    # 5. 결과 출력 (응답결과, context+score, status 에러값, 문서명들)
    return response, context_score, status, doc_names
#---------------------------------------------------------------
# 업로드 파일 선택 창 
@app.get("/upload/file")
async def upload(request:Request, user_id:str):
    assert user_id, f'user_id is empty'

    myutils.log_message(f'\n[info][/upload/file] user_id:{user_id}\n')
    
    return templates.TemplateResponse("upload_file.html", {"request": request, "user_id":user_id})
    
#---------------------------------------------------------------
# 업로드한  pdf 파일 읽어와서 처리
@app.post("/upload")
async def upload(request: Request, file: UploadFile = File(...)):
    form = await request.form()
    user_id = form.get("user_id")

    settings = myutils.get_options()
    chunk_size = settings['CHUNK_SIZE']
    chunk_overlap = settings['CHUNK_OVERLAP']
    index_name = settings['ES_INDEX_NAME']
    es_batch_size = settings['ES_BATCH_SIZE']
    
    myutils.log_message(f'[info][/upload] *user_id:{user_id}, chunk_zie:{chunk_size}, chunk_overlap:{chunk_overlap}')
    
    # MIME 타입 확인
    if file.content_type != "application/pdf":
        return HTMLResponse(content=f"""
        <html>
            <head>
                <title>Error</title>
            </head>
            <body>
                <h1>Upload PDF</h1>
                <p style="color:red;">PDF 파일이 아닙니다.</p>
                <a href="/upload/file?user_id={user_id}">Go back</a>
            </body>
        </html>
        """, status_code=400)

    # 입력 buffer에서 읽어 올때 
    #content = await file.read()
    #pdf_reader = PdfReader(io.BytesIO(content))
    
    # 1. pdf 파일 저장
    original_filename = file.filename
    myutils.log_message(f'[info][/upload] *original_filename:{original_filename}')
    
    # user_id 서브 폴더 생성
    user_dir = os.path.join(UPLOAD_DIR, user_id)
    os.makedirs(user_dir, exist_ok=True)
    
    # 랜덤 문자열 생성 후 파일 저장
    # random_str = uuid.uuid4().hex
    #file_extension = os.path.splitext(original_filename)[1]
    #save_filename = f"{os.path.splitext(original_filename)[0]}_{random_str}{file_extension}"

    # 원본 그대로 저장.
    save_filename = original_filename
    save_path = os.path.join(user_dir, save_filename)
    
    # Check if the file already exists
    if os.path.exists(save_path):
        # If the file exists, remove it
        os.remove(save_path)
            
    with open(save_path, "wb") as buffer:
        buffer.write(await file.read())

    try:
        # 2. 임베딩 
        # => langchain 이용. splitter 후 임베딩 함
        docs_vectors, docs = embedding_pdf(huggingfaceembeddings=embedding,
                      file_path=save_path, 
                      chunk_size=chunk_size, 
                      chunk_overlap=chunk_overlap)
    
        myutils.log_message(f'[info][/upload] *docs 벡터 수:{len(docs_vectors)}')
    except Exception as e:
        msg = f'*embedding_pdf is Fail!!..(error: {e})'
        myutils.log_message(f'[info][/upload] {msg}')
        return templates.TemplateResponse("upload_file.html", {"request": request, "user_id":user_id, "msg":msg})
         
    # 3. ES 저장
    # => docs_vectors 들을 elasticsearch dense_vector로 지정 후 저장
    esdocs:list = []
    count:int = 0
    num:int = 0
    for i, doc_vector in enumerate(docs_vectors):
        count += 1
        esdoc = {}
        esdoc['user_id'] = user_id
        esdoc['rfile_name'] = original_filename  # 파일명
        esdoc['rfile_text'] = docs[i]            # text
        esdoc['vector1'] = doc_vector            # 벡터  
        esdoc['_op_type'] = "index"
        esdoc['_index'] = index_name
        esdocs.append(esdoc)

        # batch_size 만큼씩 한꺼번에 es로 insert 시킴.
        if count % es_batch_size == 0:
            num +=1
            myes.RAG_bulk(docs=esdocs)
            myutils.log_message(f'[info][/upload] *RAG_bulk_{num}:{len(esdocs)}')
            esdocs = []

    if esdocs:
        num +=1
        myes.RAG_bulk(docs=esdocs)
        myutils.log_message(f'[info][/upload] *RAG_bulk:{len(esdocs)}')
        
        #response, error = myes.RAG_delete_insert_doc(doc=esdoc)
        #myutils.log_message(f'\n[info][/upload] *myes.RAG_delete_insert_doc:{response},{error}\n')

    msg:str = ''
    if num == 0:
        msg:str = '*파일을 추가할수 없습니다..'
    else:  
        msg:str = f'파일이 추가되었습니다..(index:{count})'
        # PDF 내용 읽기 및 page별루 Markdown tag ## 추가함
        out_text = output_pdf_text(file_path=save_path)
        
        # 추출된 파일도 txt로 저장함 => 이때 out_text에 대해 'utf-8' 인코딩 해줘야 함.
        #save_filename = f"{os.path.splitext(original_filename)[0]}_{random_str}.txt"
        save_filename = f"{os.path.splitext(original_filename)[0]}.txt"
                     
        save_path = os.path.join(user_dir, save_filename)
        
        # Check if the file already exists
        if os.path.exists(save_path):
            # If the file exists, remove it
            os.remove(save_path)
    
        with open(save_path, "wb") as buffer:
            buffer.write(out_text.encode('utf-8'))
    
        if len(out_text) > 3000:
           out = out_text[:2999]
        else:
           out = out_text

    return templates.TemplateResponse("upload_file.html", {"request": request, "user_id":user_id, "msg":msg})
    
    #return templates.TemplateResponse("upload_file_text.html", 
    #                                  {"request": request, "user_id":user_id, "file_name":original_filename, "text":out})
   