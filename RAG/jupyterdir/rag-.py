import argparse
import time
import os
import platform
import numpy as np
import random
import asyncio
import threading
import httpx
import uvicorn
import warnings

from os import sys
from typing import Union, Dict, List, Optional
from typing_extensions import Annotated
from fastapi import FastAPI, Query, Cookie, Form, Request, HTTPException, BackgroundTasks
from fastapi.templating import Jinja2Templates

app=FastAPI() # app 인스턴스 생성

print(f'*FastAP() 호출 완료')

#---------------------------------------------------------------
@app.get("/")  # 경로 동작 데코레이터 작성
async def root(): # 경로 동작 함수 작성
	return {"msg": "RAG 개인문서검색 예제 World"}

#---------------------------------------------------------------

#============================================================
def main():
    # 메인 함수의 코드를 여기에 작성합니다.
    return
   
if __name__ == "__main__":
    # 스크립트가 직접 실행될 때만 main 함수를 호출합니다.
    main()
#============================================================