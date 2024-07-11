## 3번째 프로젝트 - docs 프로젝트
[source] **docs 폴더**
- 문서임베딩 하고 문서 검색 하는 프로젝트.
- vision 모델로 이미지도 임베딩할 수 있음.
- 문서 text 검출은 **문서필터** 이용 했음 -> **반드시 MpowerAI 검출필터가 설치된 서버(10.X.X.X )에서 구동**해야 함.
### 구동
1. 10.x.x.x 서버 접속
2. docs 폴더 복사
3. sh docs.sh start 실행
```
root@93dad73d3406:/# cd jupyterdir
root@93dad73d3406:/jupyterdir#
sh docs.sh start
Starting docs:app
docs:app started
root@93dad73d3406:/jupyterdir#
```
4. 접속 : 9002 포트로 접속
```
{ip}:9002/docs
```

![image](https://github.com/user-attachments/assets/df4af48f-b2e2-414e-b380-909323b66572)



## 2번째 프로젝트 - vision 모델
[source] **vision 폴더**
- paligemma-3b-mix-448, Phi-3-vision-128k-instruct vision 모델을 로딩하는 예제
  
## 1번째 프로젝트 - RAG 웹 프로젝트
[source] **RAG 폴더**
- 개인문서파일(*.pdf)를 웹을 통해 업로드 하고, RAG 방식으로 검색 하는 프로젝트
- ElasticSearch 벡터db 사용.
- bongsoo/kpf-sbert-128d-v1 허깅페이스 모델을 임베딩 모델로 사용
- 모아이 카카오톡과도 연동될 수 있음.

### 구동
1. RAG\jupyterdir\data\settings.yaml 파일 수정
2. RAG\jupyterdir\\.env 파일에 key들 입력
3. docker [bong9431/langchain:1.0](https://hub.docker.com/repository/docker/bong9431/langchain/general) 로 구동
```
docker compose -f ./compose.yaml up -d
```
4. sh rag.sh start 실행
```
docker exec -it rag-1 /bin/bash
root@93dad73d3406:/#
root@93dad73d3406:/# cd jupyterdir
root@93dad73d3406:/jupyterdir#
sh rag.sh start
Starting rag:app
rag:app started
root@93dad73d3406:/jupyterdir#
```
5. 웹 접속
```
localhost:9431/list
```
![image](https://github.com/kobongsoo/project/assets/93692701/239c9d08-12db-4ed9-b71f-409f46e229ae)
