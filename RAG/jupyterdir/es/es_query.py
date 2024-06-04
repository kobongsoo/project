import torch
import time
import os
import numpy as np

############################################################
## bm25 쿼리 스크립트 만들기
############################################################
def make_bm25_query_script(query:str, user_id:str=None)->str:
    # user_id와 rfile_text 비교를 위해 must 이용함.
    if user_id:
        script_query={
            "bool": {
                "must": [
                    {"match": {"user_id": user_id}},
                    {"match": {"rfile_text": query}}
                ]
            }
        }
    else: # user_id가 없으면 rfile_text만 교교
        script_query={
            "match": {
                "rfile_text": query
            }
        }

    return script_query
 
############################################################
## 임베딩 쿼리 스크립트 만들기
############################################################
def make_embedding_query_script(qr_vector, user_id:str=None, uid_list:list=None)->str:
    
    # 문단별 10개의 벡터와 쿼리벡터를 서로 비교하여 최대값 갖는 문단들중 가장 유사한  문단 출력
    # => script """ 안에 코드는 java 임.
    # => "queryVectorMag": 0.1905 일때 100% 일치하는 값은 9.98임(즉 10점 만점임)

    # uid_list가 있는 경우에는 해당하는 목록만 검색함
    if uid_list:
        query = { "bool" :{ "must": [ { "terms": { "_id": uid_list } } ] } }
    elif user_id:
        query = { "match": {"user_id": user_id } }
    else: # uid_list가 있는 경우에는 해당하는 목록만 검색함
        query = { "match_all" : {} }

    script_query = {
        "script_score":{
            "query":query,
            "script":{
                "source": "cosineSimilarity(params.qrVector, 'vector1') + 1.0",  # 뒤에 1.0 은 코사인유사도 측정된 값 + 1.0을 더해준 출력이 나옴
                "params": {"qrVector": qr_vector}
            }
        }
    }

    return script_query
    
############################################################
## 필드 목록 계수 얻기(*중복 제거)
## => 검색할 필드는 반드시 keyword 타입이어야 함.(*text 타입이면 에러 발생)
############################################################
def make_list_query_script(field_name:str, user_id:str=None)->str:
    assert field_name, f'field_name is empty!!'

    data = {}

    if user_id:
        data = {"term": { "user_id": user_id }}
    else:
        data = {"match_all": {}}

    body = {
            "size": 0,
            "query": data,
                "aggs": {
                    "unique_field": {
                        "terms": {
                        "field": field_name,
                        "size": 1000  
                        }
                    }
                }
            }

    return body

############################################################
## 삭제 스크립트
## => user_id와 rfile_name 리스트 항복에 있는 데이터 삭제
############################################################
def make_del_query_script(fields:list, user_id:str=None)->str:
    assert len(fields) > 0, f'fields is empty!!'

    if user_id:
        body = {
          "query": {
            "bool": {
              "must": [
                {
                  "term": {
                    "user_id": user_id
                  }
                },
                {
                  "terms": {
                    "rfile_name": fields
                  }
                }
              ]
            }
          }
        }
    else:
        body = {
          "query": {
            "terms": {
              "rfile_name": fields
            }
          }
        }

    return body

    
        