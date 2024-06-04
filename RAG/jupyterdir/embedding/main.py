from enum import Enum
from typing import Union
import io
import os
import uuid

from langchain_community.embeddings import (
    HuggingFaceEmbeddings,
    HuggingFaceBgeEmbeddings,
)

##langchain ######################
from langchain.document_loaders import PyPDFLoader
from langchain.text_splitter import RecursiveCharacterTextSplitter
##################################

# embedding doc
# => langchain을 이용해서 doc을 splitter 하고 embedding 시킴.
def embedding_pdf(huggingfaceembeddings,     # HuggingFaceEmbedding 임베딩 모델 인스턴스     
                  file_path:str,             # 임베딩할 파일 경로
                  chunk_size:int=500,        # chunk 사이즈
                  chunk_overlap:int=0        # chunk 오버랩
                 ):

    assert huggingfaceembeddings, f"[embedding_pdf] huggingfaceembeddings is empty!"
    assert file_path, f"[embedding_pdf] file_path is empty!"
    assert chunk_size > 1, f"[embedding_pdf] chunk_size < 1!"   
    assert chunk_overlap > -1, f"[embedding_pdf] chunk_overlap < 0!"   
    
     # => pdf 로딩 후 RecursiveSplitter로 분할함.
    loader = PyPDFLoader(file_path=file_path)
    text_splitter = RecursiveCharacterTextSplitter(chunk_size=chunk_size, chunk_overlap=chunk_overlap)
    split_docs = loader.load_and_split(text_splitter=text_splitter)

    # split 문서를 docs 리스트에 담음.
    docs:list=[]
    for i in range(len(split_docs)):
        #print(split_docs[i].page_content)
        #print(f'--'*20)
        docs.append(split_docs[i].page_content)

    # *임베딩 모델 설정 
    # => 여기서 다시 임베딩 모델 호출하면 오래걸림. 따라서 메인에서 1번만 로딩하고, 메인에서 로딩한 모델 인스턴스 재사용함. 
    embedding = huggingfaceembeddings
    print(f'*[embedding_pdf] 임베딩 모델: {embedding.model_name}')

    # 임베딩 실행
    docs_vectors = embedding.embed_documents(docs)
    
    return docs_vectors, docs