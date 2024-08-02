#pragma once

#include <string> // std::string을 사용하기 위한 헤더 파일

/////////////////////////////////////////////////////////////////////////////
#define MAX_OBJ_PATH_W 520
#define MAX_OBJ_PATH_A (520 * sizeof(WCHAR))
#define MAX_SIZE (512 * sizeof(WCHAR))

// 유사문서검색데이터
typedef struct _SIMILAR_DOC_DATA
{
	DOUBLE score;						// 스코어
	WCHAR rfilename[MAX_OBJ_PATH_A];	// rfilename(파일명=유니크한값)
} SIMILAR_DOC_DATA, *PSIMILAR_DOC_DATA;

// 유사문서임베딩정보
typedef struct _UPLOAD_EMBED_DATA
{
	CString strExtrafilePath;	            // text 추출된 파일 경로
	CString strMineType;                      // mime 타입 
	CString strRes;                            // rfile_name, rfile_text, vector0~9 등  
} UPLOAD_EMBED_DATA, *PUPLOAD_EMBED_DATA;

// 임베딩문서정보
typedef struct _RFILE_NAME_INFO
{
	WCHAR RFileName[MAX_OBJ_PATH_A]; // rfilename
	WCHAR RFileText[MAX_SIZE];			 // rfiletext
	WCHAR Vector0[MAX_SIZE];				 // 평균벡터
} RFILE_NAME_INFO, *P_RFILE_NAME_INFO;
/////////////////////////////////////////////////////////////////////////////
// URL 인코딩 관련 함수
int toHex(const unsigned char& x);
const std::string URLEncodeA(const std::string& sIn);
CString URLEncode( CString strPlainText);

// GET/POST 함수
DWORD 
Request(BOOLEAN bPost,			// TRUE=POST, FALSE=GET
		IN CString strIp,		// IP = _T("192.168.0.61")
		IN UINT nPort,			// Port = 9431
		IN CString strObject,	// strObject = _T("/test/post?user_id=BONG")
		IN CString PostData,	// PostData = _T("{\"fields\": [\"test\"]}")
		OUT CStringW& strResponseW);

// 파일 확장자에 따라 MIME 타입을 반환하는 함수
std::string GetMimeType(const CString& filePath);

// UTF-8 변환 함수
std::string CStringToUTF8(const CString& str);

// 입력된 파일을 서버로 전송하는 예제...
DWORD 
FileSend(IN CString strServerName,		// _T("10.10.4.10")
		IN UINT nServerPort,			// 9002
		IN CString strObject,			// _T("/upload_search01?file_folder={폴더명}")
		IN CString strFilePath,			// _T("E:\\docs\\임베딩 API 서버-Admin 가이드-202310.docx")
		OUT CStringW& Response);		// 서버응답데이터	

// 임베딩문서목록얻기(/list01) 파싱
DWORD 
Parse_List01(IN CStringW strResponseW,   // 파싱할 CString형
			OUT UINT& nNum,				 // 목록 계수
			OUT CList<SIMILAR_DOC_DATA, SIMILAR_DOC_DATA&>* pSimilarDocList); //파싱후 출력할 유사문서데이터

// 문서업로드+검색(/upload_search01) 파싱
DWORD 
Parse_Upload_Search01(IN CStringW strResponseW,   // 파싱할 CString형
					 OUT CList<SIMILAR_DOC_DATA, SIMILAR_DOC_DATA&>* pSimilarDocList); //파싱후 출력할 유사문서데이터

// 문서업로드+임베딩(/upload_embed01) 파싱
DWORD 
Parse_Upload_Embed01(IN CStringW strResponseW,   // 파싱할 CString형
					OUT UPLOAD_EMBED_DATA& pUploadEmbedData); //파싱후 출력할 유사문서데이터

// 임베딩문서정보얻기(/rfile_name?rfile_name={파일명})
DWORD
Parse_RFileName01(IN CStringW strResponseW,
				OUT CList<RFILE_NAME_INFO, RFILE_NAME_INFO&>* pRFileNameInfoList); // 파싱후 출력할 리스트