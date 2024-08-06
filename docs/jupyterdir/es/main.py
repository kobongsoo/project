import torch
import time
import os
import numpy as np

from elasticsearch import Elasticsearch
from elasticsearch.helpers import bulk
from typing import Dict, List, Optional
from .es_query import make_bm25_query_script, make_embedding_query_script, make_list_query_script, make_del_query_script, make_rfile_name_query_script
#import logging

class My_ElasticSearch:

    def __init__(self, es_url:str, index_name:str, index_file_path:str, create:bool=False):
        
        assert es_url, f'es_url is empty'
        assert index_name, f'index_name is empty'
        assert index_file_path, f'index_file_path is empty'
        
        self.es_url = es_url
        self.index_name = index_name
        self.index_file_path = index_file_path   # 인덱스 mapping 값 (json)
        self.create = create
        
        try:
            # 1.elasticsearch 접속
            #logging.getLogger('elasticsearch').setLevel(logging.ERROR)  # ERROR 레벨 이상만 로깅
            self.es = Elasticsearch(self.es_url) 
        except Exception as e:
            msg = f'Elasticsearch:{self.es_url}=>{e}'
            print(msg)
            
        self.create_index() # 인덱스만듬
        
        return

    ###########################################################
    # 인덱스 생성/삭제
    ###########################################################
    ## 인덱스 생성 => 매핑
    def create_index(self):
        
        if self.create == True or not self.es.indices.exists(self.index_name):
            self.es.indices.delete(index=self.index_name, ignore=[404])
            print(f'*[create_index] self.es.indices.delete')
        
            # 인덱스 생성
            with open(self.index_file_path) as index_file:
                source = index_file.read().strip()
                print(f'*[create_index] self.es.indices.create')
                return self.es.indices.create(index=self.index_name, body=source)
            

    ## 인덱스 자체 삭제
    def delete_index(self):
        if self.es.indices.exists(index=self.index_name):
            return self.es.indices.delete(index=self.index_name)

    ###########################################################
    # 인데스에 데이터 추가 
    ###########################################################
    def insert(self, doc:dict, doc_type:str="_doc"):
        return self.es.index(index=self.index_name, doc_type=doc_type, body=doc)

    # 한꺼번에 
    def RAG_bulk(self, docs:list):
        return bulk(self.es, docs)
        
    def RAG_delete_insert_doc(self, doc:dict, doc_type:str="_doc"):
        doc1 = doc
        error:int = 0
        
        try:
            print(f'*[RAG_delete_insert_doc] start==>\n')
            
            # 인덱스 생성
            self.create_index()
            print(f'*[RAG_delete_insert_doc] self.create_index\n')
            
            vector1 = doc1['vector1']
            user_id = doc1['user_id']
            
            print(f'*[RAG_delete_insert_doc] user_id:{user_id}\n')
            
            script_query = make_embedding_query_script(qr_vector=vector1, user_id=user_id) 

            print(f'*[RAG_delete_insert_doc] script_query:{script_query}\n')
            
            # 4. 실제 ES로 검색 쿼리 날림
            response = self.es.search(
                index=self.index_name,
                body={
                    "size": 1,
                    "query": script_query,
                    "_source":{"includes": ["user_id", "rfile_name", "rfile_text"]}
                }
            )

            print(f'*[RAG_delete_insert_doc] self.es.search:{response}\n')
              
            # 동일한 id이면 삭제
            for hit in response["hits"]["hits"]: 
                doc_id = hit["_id"]
                doc_score = hit['_score']
                doc_user_id = hit["_source"]["user_id"]
                doc_rfile_name = hit["_source"]["rfile_name"]
               
                print(f'*[RAG_delete_insert_doc] user_id:{doc_user_id}, _id:{doc_id}, score:{doc_score}, rfile_name:{doc_rfile_name}')
                print(f'==='*30)

                # score와 user_id를 비교하여 삭제함
                # => score가 2.0 이라는 것은 내용이 똑같다는 말이므로 삭제함.
                if doc_score >= 2.0 and doc_user_id == user_id:
                    res = self.delete_by_id(esid=doc_id)   
                    #print(f'*[delete_insert_doc] delete_by_id:{res}')

            # 추가하기 
            res = self.es.index(index=self.index_name, doc_type=doc_type, body=doc1)
            print(f'*[RAG_delete_insert_doc] self.es.index:{res}\n')
            return res, error
            
        except Exception as e:
            error = 1002
            msg = f'delete_insert_doc:=>{e}'
            print(f'*[RAG_delete_insert_doc] Exception:{msg}\n')
            return msg, error
   
    ############################################################
    ## 검색
    ############################################################
    def search(self, body):
        assert body, f"body is empty"
        #print(f'*[search] body:{body}\n')
        res = self.es.search(index=self.index_name, body=body)
        return res
    
    ############################################################
    ## 인덱스 내의 데이터 업데이트=>_id 에 데이터 업데이트
    ############################################################
    def update(self, esid, doc, doc_type):

        body = {
            'doc': doc
        }

        res=self.es.update(index=self.index_name, id=esid, body=body, doc_type=doc_type)
        return res

    ############################################################
    ## 인덱스 내의 데이터 삭제 => query 이용(예: data = {'title': '제주도'})
    ############################################################
    def delete(self, doc:dict):
        if doc is None:  # data가 없으면 모두 삭제
            data = {"match_all":{}}
        else:
            data = {"match": doc}

        body = {"query": data}
        return self.es.delete_by_query(index=self.index_name, body=body)

    ############################################################
    ## 인덱스 내의 데이터 삭제 => id 이용
    ############################################################
    def delete_by_id(self, esid):
        return self.es.delete(index=self.index_name, id=esid)
        
    ############################################################
    ## BM25 검색
    # => query: 쿼리, k=검색계수 
    ############################################################
    def BM25_search(self, query:str, k:int=3, min_score:float=0.9, user_id:str=None):
        assert query, f'query is empty'
        assert k > 1, f'k < 1'

        # 1. 쿼리 만듬.
        script_query = make_bm25_query_script(query=query, user_id=user_id)
        body = {
            "size": k,
            "query": script_query,
            "_source":{"includes": ["user_id", "rfile_name", "rfile_text"]}
        }
        #print(f'*body:\n{body}\n')
        
        response = None
        response = self.es.search(index=self.index_name, body=body)

        docs:list = []
        for hit in response["hits"]["hits"]: 
            doc = {}  #dict 선언
            score = hit["_score"]
            print(f'*[BM25_search] score:{score}\rfile_name:{hit["_source"]["rfile_name"]}\n')
            
            # score가 min_score이상인 경우에만 추가 
            if score > min_score: 
                doc['user_id'] = hit["_source"]["user_id"]            # user_id 담음
                doc['rfile_name'] = hit["_source"]["rfile_name"]      # contextid 담음
                doc['rfile_text'] = hit["_source"]["rfile_text"]      # text 담음.
                doc['score'] = score
                docs.append(doc)

        return docs
    
    ############################################################
    ## BM25 docs 검색
    # => query: 쿼리, k=검색계수 
    ############################################################
    def BM25_search_docs(self, query:str, k:int=3, min_score:float=0.9):
        assert query, f'query is empty'
        assert k > 1, f'k < 1'

        # 1. 쿼리 만듬.
        script_query={
            "match": {
                "rfile_text": query
            }
        }
            
        body = {
            "size": k,
            "query": script_query,
            "_source":{"includes": ["rfile_name", "rfile_text"]}
        }
        #print(f'*body:\n{body}\n')
        
        response = None
        response = self.es.search(index=self.index_name, body=body)
        #print(f'*[BM25_search_docs] response: {response}')
        docs:list = []
        for hit in response["hits"]["hits"]: 
            doc = {}  #dict 선언
            score = hit["_score"]
            hit["_source"]["rfile_name"]
            #print(f'*[BM25_search_docs] score:{score}\nrfile_name:{hit["_source"]["rfile_name"]}\n')
            
            # score가 min_score이상인 경우에만 추가 
            if score > min_score: 
                doc['rfile_name'] = hit["_source"]["rfile_name"]      # contextid 담음
                #doc['rfile_text'] = hit["_source"]["rfile_text"]      # text 담음.
                doc['score'] = score
                docs.append(doc)

        return docs
    
    ############################################################
    ## 임베딩된 인덱스 rfile_name 로 검색
    # => script_query: 쿼리스크립트
    ############################################################
    def search_rfile_name_docs(self, rfile_name:str):
        assert rfile_name, f'rfile_name is empty'
        
        script_query = make_rfile_name_query_script(query=rfile_name)
        
        body = {
            "query": script_query,
            "_source":{"includes": ["rfile_name", "rfile_text", "vector0"]}
        }
        
        #print(f'*body:\n{body}\n')
        
        # es로 쿼리 날림.
        response = None
        response = self.es.search(index=self.index_name, body=body)
        
        docs:list = []
        for hit in response["hits"]["hits"]: 
            doc = {}  #dict 선언
            doc['rfile_name'] = hit["_source"]["rfile_name"]      # contextid 담음
            
            # rfile_text는 너무길수 있으므로 1024까지만 출력함
            rfile_text = hit["_source"]["rfile_text"] 
            if len(rfile_text) > 1024:
                rfile_text = rfile_text[:1024]
                
            doc['rfile_text'] = rfile_text
           
            vector0 = hit["_source"]["vector0"]
            #print(f'vector0: {type(vector0)}')
            # 리스트를 numpy 배열로 변환
            # vector0_array = np.array(vector0)
            doc['vector0'] = str(vector0) # str형으로 전달
            docs.append(doc)

        return docs
    ############################################################
    ## 임베딩 검색
    # => script_query: 쿼리스크립트, k=검색계수 
    ############################################################
    def search_docs(self, script_query:str, k:int=5, min_score:float=0.0, user_id:str=None):
        assert script_query, f'script_query is empty'
        
        body = {
            "size": k,
            "query": script_query,
            "_source":{"includes": ["rfile_name", "rfile_text"]}
        }
        
        #print(f'*body:\n{body}\n')
        
         # es로 쿼리 날림.
        response = None
        response = self.es.search(index=self.index_name, body=body)
        
        docs:list = []
        for hit in response["hits"]["hits"]: 
            doc = {}  #dict 선언
            score = hit["_score"]
            #print(f'*[search_docs] score:{score}\nrfile_name:{hit["_source"]["rfile_name"]}\n')
            
            # score가 min_score이상인 경우에만 추가 
            if score > min_score: 
                doc['rfile_name'] = hit["_source"]["rfile_name"]       # contextid 담음
                doc['id'] = hit["_id"]   # 문서id
                #doc['rfile_text'] = hit["_source"]["rfile_text"]      # text 담음.
                doc['score'] = score
                #print(f'*[search_docs] doc:{doc}\n')
                docs.append(doc)

        return docs
    
    ############################################################
    ## 임베딩 검색
    # => query: 쿼리, k=검색계수 
    ############################################################
    def Embedding_search(self, huggingfaceembeddings, query:str, k:int=3, min_score:float=0.5, user_id:str=None):
        assert query, f'query is empty'
        assert k > 1, f'k < 1'

        # 1. query_vector 구함.
        query_vector = huggingfaceembeddings.embed_query(query)

        # 2. 기본 벡터 쿼리 만듬
        script_query = make_embedding_query_script(qr_vector=query_vector, user_id=user_id) 
        body = {
            "size": k,
            "query": script_query,
            "_source":{"includes": ["user_id", "rfile_name", "rfile_text"]}
        }
        print(f'*body:\n{body}\n')
        
        # 3. es로 쿼리 날림.
        response = None
        response = self.es.search(index=self.index_name, body=body)

        docs:list = []
        for hit in response["hits"]["hits"]: 
            doc = {}  #dict 선언
            score = hit["_score"]
            print(f'*[Embedding_search] score:{score}\nrfile_text:{hit["_source"]["rfile_text"]}\n')
            
            # score가 min_score이상인 경우에만 추가 
            if score > min_score: 
                doc['user_id'] = hit["_source"]["user_id"]            # user_id 담음
                doc['rfile_name'] = hit["_source"]["rfile_name"]      # contextid 담음
                doc['rfile_text'] = hit["_source"]["rfile_text"]      # text 담음.
                doc['score'] = score
                docs.append(doc)

        return docs
    ############################################################
    ## rfile_nama 몯록 얻기(*중복제거)
    # => user_id : 사용자 id, field_name: 목록 얻을 필드명(rfile_name)
    ############################################################
    def get_list(self, field_name:str, user_id:str=None):
        assert field_name, f'field_name is empty'
        
        # 1. list 쿼리 만듬 
        body = make_list_query_script(field_name=field_name, user_id=user_id)

        # 2. es로 쿼리 날림.
        response = None
        response = self.es.search(index=self.index_name, body=body)

        # 3. 파싱
        buckets = response['aggregations']['unique_field']['buckets']
        fields:list = [] # 필드명칭
        counts:list = [] # 카운터 계수 
        for bucket in buckets:
            fields.append(bucket['key'])
            counts.append(bucket['doc_count'])

        return fields, counts

    ############################################################
    ## rfile_nama 리스트에 해당하는 목록 삭제
    # => user_id : 사용자 id, field_name: 삭제할 필드명(rfile_name)
    ############################################################
    def del_list(self, fields:list, user_id:str=None):
        assert len(fields) > 0, f'fields is empty'
        error:int = 0
        
        # 1. list 쿼리 만듬 
        body = make_del_query_script(fields=fields, user_id=user_id)

        try:
            # 2. es로 쿼리 날림.
            response = None
            response = self.es.delete_by_query(index=self.index_name, body=body)
            
        except Exception as e:
            error = 1002
            msg = f'delete_by_query:=>{e}'
            print(f'*[del_list] Exception:{msg}\n')
             
        return error

    
            
        
            
        
        
        
        
        
        