#include "stdafx.h"
#include "strsafe.h"
#include "Util.h"

//================================
// rapidjson 헤더만 추가하면 됨
//================================
//#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"

using namespace rapidjson;
using namespace std;
//================================

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <afxinet.h>
#include <iostream>
#include <fstream>
#include <vector> // std::vector를 사용하기 위한 헤더 파일
#include <string> // std::string을 사용하기 위한 헤더 파일
#include <map>  // std::map 헤더 파일 추가

#pragma comment(lib, "wininet.lib")

// URL 인코딩 함수
int toHex(const unsigned char& x)
{
	return x > 9 ? x + 55: x + 48;
}

const std::string URLEncodeA(const std::string& sIn)
{
	int nLen = sIn.length() + 1;	


	unsigned char* buffer = new unsigned char[nLen * 3];
	memset(buffer, 0x00, nLen * 3);

	register unsigned char* p = buffer;

	register unsigned char* q = (unsigned char*)sIn.c_str();

	while (*q)
	{
		if (isalnum(*q))
		{
			*p++ = *q;
		}
		else
		{
			if (isspace(*q))
				*p++ = '+';
			else if (*q == '_')
				*p++ = '_';
			else if (*q == '-')
				*p++ = '-';
			else if (*q == '.')
				*p++ = '.';
			else
			{
				unsigned char temp = *q;
				*p++ = '%';
				*p++ = (char)toHex(temp>>4);
				*p++ = (char)toHex(temp%16);
			}
		}
		q++;
	}
	*(++p) = '\0';

	std::string strReturn = (char*)buffer;

	delete[] buffer; 
	buffer = NULL;

	return strReturn;
}


CString URLEncode( CString strPlainText)
{
	TCHAR* szPlain = strPlainText.GetBuffer(0);
	int c_size = WideCharToMultiByte(CP_UTF8, 0, szPlain, -1, NULL, 0, NULL, NULL);
	char* buffer = new char[c_size+1];
	memset(buffer, 0x00, c_size + 1);
	WideCharToMultiByte(CP_UTF8, 0, szPlain, -1, buffer, c_size, NULL, NULL);
	strPlainText.ReleaseBuffer();

	std::string plainString = buffer;
	const std::string encodeString = URLEncodeA(plainString.c_str());
	int w_size = MultiByteToWideChar(CP_UTF8, 0, encodeString.c_str(), -1, NULL, 0);
	TCHAR* buf = new TCHAR[w_size + 1];
	memset(buf, 0x00, w_size + 1);
	MultiByteToWideChar(CP_UTF8, 0, encodeString.c_str(), -1, buf, w_size);

	CString strResult = buf;

	delete[] buffer;
	buffer = NULL;

	delete[] buf;
	buf = NULL;

	return strResult;
}

DWORD 
Request(BOOLEAN bPost,			// TRUE=POST, FALSE=GET
		IN CString strIp,		// IP = _T("192.168.0.61")
		IN UINT nPort,			// Port = 9431
		IN CString strObject,	// strObject = _T("/test/post?user_id=BONG")
		IN CString PostData,	// PostData = _T("{\"fields\": [\"test\"]}")
		OUT CStringW& strResponseW)
{
	DWORD Status = ERROR_SUCCESS;

	CInternetSession session;
	CHttpConnection* pConnection = NULL;
	CHttpFile* pFile = NULL;
	CString strMsg;

	try
	{
		// Open a connection to the server
		pConnection = session.GetHttpConnection(strIp, nPort, _T(""), _T(""));

		if(bPost == TRUE)
		{
			// POST: Prepare the HTTP request
			pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST, strObject, NULL, 1, NULL, NULL, INTERNET_FLAG_RELOAD);
		}
		else
		{
			// GET: Prepare the HTTP request
			pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject, NULL, 1, NULL, NULL, INTERNET_FLAG_RELOAD);

		}

		// Set headers
		pFile->AddRequestHeaders(_T("Accept: application/json"));

		if(bPost == TRUE)
		{
			pFile->AddRequestHeaders(_T("Content-Type: application/json"));
		}


		if(bPost == TRUE)
		{
			// JSON data to be sent in the POST request
			CString strJson = PostData;

			// Calculate the content length
			CString strContentLength;
			strContentLength.Format(_T("Content-Length: %d"), strJson.GetLength() * sizeof(TCHAR));
			pFile->AddRequestHeaders(strContentLength);

			// Send the request
			pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)strJson, strJson.GetLength() * sizeof(TCHAR));
		}
		else
		{
			// Send the request
			pFile->SendRequest();
		}

		DWORD dwStatus = 0; 
		// Get the response status code
		pFile->QueryInfoStatusCode(dwStatus);

		//strMsg.Format(_T("status: %d"), dwStatusCode);
		//AfxMessageBox(strMsg);
		
		if (dwStatus == HTTP_STATUS_OK)
		{
			// Read the response body
			CStringA strResponseA;
			char szBuff[2048];
			UINT nRead = 0;

			while ((nRead = pFile->Read(szBuff, sizeof(szBuff) - 1)) > 0)
			{
				szBuff[nRead] = '\0';
				strResponseA  += szBuff;
			}

			// Convert UTF-8 response to Unicode
			int nLength = MultiByteToWideChar(CP_UTF8, 0, strResponseA, -1, NULL, 0);
			MultiByteToWideChar(CP_UTF8, 0, strResponseA, -1, strResponseW.GetBuffer(nLength), nLength);
			strResponseW.ReleaseBuffer();

			// Display response body
			//AfxMessageBox(_T("Response body:\n") + strResponseW);
		}
		else
		{
			// Clean up
			pFile->Close();
			pConnection->Close();

			// Handle HTTP errors
			strMsg.Format(_T("HTTP error: %d"), dwStatus);
			AfxMessageBox(strMsg);
			return dwStatus;
		}

		// Clean up
		pFile->Close();
		pConnection->Close();
	}
	catch (CInternetException* pEx)
	{
		TCHAR szErr[2048];
		pEx->GetErrorMessage(szErr, 2048);
		Status = pEx->m_dwError;
		strMsg.Format(_T("error: %d\r\n%s"), Status, szErr);
		AfxMessageBox(strMsg);
		pEx->Delete();
		
		return Status;
	}

	// Clean up pointers
	if (pFile)
		delete pFile;

	if (pConnection)
		delete pConnection;

	session.Close();

	return Status;
}

// 파일 확장자에 따라 MIME 타입을 반환하는 함수
std::string GetMimeType(const CString& filePath)
{
	std::map<CString, std::string> mimeTypes;

	// MIME 타입 매핑, 필요에 따라 더 많은 MIME 타입 추가
	mimeTypes[_T(".pdf")] = "application/pdf";
	mimeTypes[_T(".pptx")] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	mimeTypes[_T(".ppt")] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	mimeTypes[_T(".docx")] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	mimeTypes[_T(".doc")] = "application/msword";
	mimeTypes[_T(".jpg")] = "image/jpeg";
	mimeTypes[_T(".png")] = "image/png";
	mimeTypes[_T(".txt")] = "text/plain";

	// 파일의 확장자 추출
	CString ext = filePath.Right(filePath.GetLength() - filePath.ReverseFind('.'));
	auto it = mimeTypes.find(ext);

	if (it != mimeTypes.end())
	{
		return it->second;
	}
	return "application/octet-stream"; // 기본 MIME 타입
}

// UTF-8 변환 함수
std::string CStringToUTF8(const CString& str)
{
	CT2CA pszConvertedAnsiString(str, CP_UTF8);
	std::string utf8Str(pszConvertedAnsiString);
	return utf8Str;
}

DWORD 
Parse(IN CStringW strResponseW,
	 OUT rapidjson::Document& jsondata)
{
	DWORD Status = ERROR_SUCCESS;

	// JSONG 파싱
	USES_CONVERSION;
	char* strParseResponse = T2A(strResponseW);
	//rapidjson::Document jsondata;
	rapidjson::ParseResult parseResult = jsondata.Parse(strParseResponse);
	//jsondata.Parse(strParseResponse);

	if (!parseResult) 
	{
		AfxMessageBox(_T("[Parse_RFileName01]\r\nFailed to parse JSON#1"));
		Status = ERROR_INVALID_DATA;
		return Status;
	}

	if (!jsondata.IsObject()) 
	{
		AfxMessageBox(_T("[Parse_RFileName01]\r\nJSON is not an object"));
		Status = ERROR_INVALID_DATA;
		return Status;
	}

	return Status;
}

// 임베딩문서정보얻기(/rfile_name?rfile_name={파일명})
DWORD
Parse_RFileName01(IN CStringW strResponseW,
				 OUT CList<RFILE_NAME_INFO, RFILE_NAME_INFO&>* pRFileNameInfoList) // 파싱후 출력할 리스트
{
	DWORD Status = ERROR_SUCCESS;
	CString response = _T("");
	CString strMsg = _T("");

	USES_CONVERSION;
	rapidjson::Document jsondata;
	if(Status != Parse(strResponseW, jsondata))
	{
		return Status;
	}

	pRFileNameInfoList->RemoveAll();

	// resoponse 출력
	if (jsondata.HasMember("response") && jsondata["response"].IsString()) 
	{
		response = A2T(jsondata["response"].GetString());

		// Replace single quotes with double quotes
		CStringW correctedResponse = response;
		correctedResponse.Replace(_T('\''), _T('\"'));

		// Parse the list inside the response
		rapidjson::Document listData;
		rapidjson::ParseResult listParseResult = listData.Parse(T2A(correctedResponse));

		if (!listParseResult)
		{
			AfxMessageBox(_T("[Parse_Upload_Search01]\r\nFailed to parse list inside response"));
			Status = ERROR_INVALID_DATA;
			return Status;
		}

		if (listData.IsArray()) 
		{
			RFILE_NAME_INFO RFileNameInfo;

			const rapidjson::Value& array = listData.GetArray();
			for (rapidjson::SizeType i = 0; i < array.Size(); ++i) 
			{
				const rapidjson::Value& item = array[i];

				if (item.HasMember("rfile_name") && item["rfile_name"].IsString() &&
					item.HasMember("rfile_text") && item["rfile_text"].IsString())
				{
					ZeroMemory(&RFileNameInfo, sizeof(RFILE_NAME_INFO));

					CString strRFileName = A2T(item["rfile_name"].GetString());
					StringCchCopyW(RFileNameInfo.RFileName, MAX_PATH, strRFileName.GetBuffer(0));	// rfilename(파일명=유니크한값)

					CString strRFileText = A2T(item["rfile_text"].GetString());
					StringCchCopyW(RFileNameInfo.RFileText, MAX_SIZE, strRFileText.GetBuffer(0));	// rfiletext(파일내용)

					CString strVector0 = A2T(item["vector0"].GetString());
					StringCchCopyW(RFileNameInfo.Vector0, MAX_SIZE, strVector0.GetBuffer(0));		// vector0(평균벡터)

					/*
					if (item.HasMember("vector0") && item["vector0"].IsArray())
					{
						CString strVector0;
						const auto& vectorArray = item["vector0"].GetArray(); // Using auto to get the correct type
						for (rapidjson::SizeType j = 0; j < vectorArray.Size(); ++j)
						{
							if (vectorArray[j].IsNumber())
							{
								CString temp;
								temp.Format(_T("%f"), vectorArray[j].GetDouble());
								strVector0 += temp;
								if (j != vectorArray.Size() - 1)
								{
									strVector0 += _T(", ");
								}
							}
						}
						
						RFileNameInfo.strVector0 = strVector0;
					}*/

					pRFileNameInfoList->AddTail(RFileNameInfo);		// 리스트에 추가	

					//CString result;
					//result.Format(_T("File: %s, Score: %f"), rfileName, score);
					//AfxMessageBox(result);
				}
				else 
				{
					//AfxMessageBox(_T("[Parse_Upload_Search01]\r\nMissing expected members or incorrect types in item"));
				}
			}
		}
	}

	return Status;
}

// 임베딩문서목록얻기(/list01) 파싱
DWORD 
Parse_List01(IN CStringW strResponseW,   // 파싱할 CString형
			OUT UINT& nNum,				 // 목록 계수
			OUT CList<SIMILAR_DOC_DATA, SIMILAR_DOC_DATA&>* pSimilarDocList) //파싱후 출력할 유사문서데이터
{
	DWORD Status = ERROR_SUCCESS;
	CString response = _T("");
	CString strMsg = _T("");

	USES_CONVERSION;
	rapidjson::Document jsondata;
	if(Status != Parse(strResponseW, jsondata))
	{
		return Status;
	}

	// error 출력
	if (jsondata.HasMember("num") && jsondata["num"].IsString()) 
	{
		response = A2T(jsondata["num"].GetString());
		if(response.IsEmpty() == FALSE)
		{
			nNum = _ttoi(response);
		}
	} 

	pSimilarDocList->RemoveAll();

	// resoponse 출력
	if (jsondata.HasMember("response") && jsondata["response"].IsString()) 
	{
		response = A2T(jsondata["response"].GetString());

		// Replace single quotes with double quotes
		CStringW correctedResponse = response;
		correctedResponse.Replace(_T('\''), _T('\"'));

		// Parse the list inside the response
		rapidjson::Document listData;
		rapidjson::ParseResult listParseResult = listData.Parse(T2A(correctedResponse));

		if (!listParseResult)
		{
			AfxMessageBox(_T("[Parse_Upload_Search01]\r\nFailed to parse list inside response"));
			Status = ERROR_INVALID_DATA;
			return Status;
		}

		if (listData.IsArray()) 
		{
			SIMILAR_DOC_DATA SimilarDocData;

			const rapidjson::Value& array = listData.GetArray();
			for (rapidjson::SizeType i = 0; i < array.Size(); ++i) 
			{
				const rapidjson::Value& item = array[i];
				if (item.HasMember("rfile_name") && item["rfile_name"].IsString() &&
					item.HasMember("score") && item["score"].IsNumber())
				{
					ZeroMemory(&SimilarDocData, sizeof(SIMILAR_DOC_DATA));

					CString rfileName = A2T(item["rfile_name"].GetString());
					StringCchCopyW(SimilarDocData.rfilename, MAX_PATH, rfileName.GetBuffer(0));	// rfilename(파일명=유니크한값)

					double score = item["score"].GetDouble();
					SimilarDocData.score = score;

					pSimilarDocList->AddTail(SimilarDocData);		// 리스트에 추가	

					//CString result;
					//result.Format(_T("File: %s, Score: %f"), rfileName, score);
					//AfxMessageBox(result);
				}
				else 
				{
					//AfxMessageBox(_T("[Parse_Upload_Search01]\r\nMissing expected members or incorrect types in item"));
				}
			}
		}
	}

	return Status;
}

// 문서업로드+검색(/upload_search01) 파싱
DWORD 
Parse_Upload_Search01(IN CStringW strResponseW,   // 파싱할 CString형
					  OUT CList<SIMILAR_DOC_DATA, SIMILAR_DOC_DATA&>* pSimilarDocList) //파싱후 출력할 유사문서데이터
{
	DWORD Status = ERROR_SUCCESS;
	CString response = _T("");
	CString strMsg = _T("");

	USES_CONVERSION;
	rapidjson::Document jsondata;
	if(Status != Parse(strResponseW, jsondata))
	{
		return Status;
	}

	pSimilarDocList->RemoveAll();

	// resoponse 출력
	if (jsondata.HasMember("response") && jsondata["response"].IsString()) 
	{
		response = A2T(jsondata["response"].GetString());
		
		// Replace single quotes with double quotes
		CStringW correctedResponse = response;
		correctedResponse.Replace(_T('\''), _T('\"'));

		// Parse the list inside the response
		rapidjson::Document listData;
		rapidjson::ParseResult listParseResult = listData.Parse(T2A(correctedResponse));

		if (!listParseResult)
		{
			AfxMessageBox(_T("[Parse_Upload_Search01]\r\nFailed to parse list inside response"));
			Status = ERROR_INVALID_DATA;
			return Status;
		}

		if (listData.IsArray()) 
		{
			SIMILAR_DOC_DATA SimilarDocData;

			const rapidjson::Value& array = listData.GetArray();
			for (rapidjson::SizeType i = 0; i < array.Size(); ++i) 
			{
				const rapidjson::Value& item = array[i];
				if (item.HasMember("rfile_name") && item["rfile_name"].IsString() &&
					item.HasMember("score") && item["score"].IsNumber())
				{
					ZeroMemory(&SimilarDocData, sizeof(SIMILAR_DOC_DATA));

					CString rfileName = A2T(item["rfile_name"].GetString());
					StringCchCopyW(SimilarDocData.rfilename, MAX_PATH, rfileName.GetBuffer(0));	// rfilename(파일명=유니크한값)

					double score = item["score"].GetDouble();
					SimilarDocData.score = score;

					pSimilarDocList->AddTail(SimilarDocData);		// 리스트에 추가	

					//CString result;
					//result.Format(_T("File: %s, Score: %f"), rfileName, score);
					//AfxMessageBox(result);
				}
				else 
				{
					//AfxMessageBox(_T("[Parse_Upload_Search01]\r\nMissing expected members or incorrect types in item"));
				}
			}
		}
	}

	return Status;
}

// 문서업로드+임베딩(/upload_embed01) 파싱
/*
{
"error": "0",
"extrafilePath": "../../data11/docs-extra/extract/304_초거대 AI 모델 학습데이터 문서 선정 방법  - 복사본.pptx",
"mime_type": "doc",
"res": "{'rfile_name': '304_초거대 AI 모델 학습데이터 문서 선정 방법  - 복사본.pptx', 
		  'rfile_text': \"..PAGE:1\\n초거대 AI 모델 학습데이터 문서 선정 방법',
		  'vector0;\;\':xxxx
		}
}
*/

DWORD 
Parse_Upload_Embed01(IN CStringW strResponseW,   // 파싱할 CString형
					OUT UPLOAD_EMBED_DATA& pUploadEmbedData) //파싱후 출력할 유사문서데이터
{
	DWORD Status = ERROR_SUCCESS;
	CString response = _T("");
	CString strMsg = _T("");

	USES_CONVERSION;
	rapidjson::Document jsondata;
	if(Status != Parse(strResponseW, jsondata))
	{
		return Status;
	}

	// error 출력
	if (jsondata.HasMember("error") && jsondata["error"].IsString()) 
	{
		//int creditsLeft = jsondata["response"].GetInt();
		response = A2T(jsondata["error"].GetString());
		Status = _ttoi(response.GetBuffer(0));
		if(Status != ERROR_SUCCESS)
		{
			strMsg.Format(_T("[Parse_Upload_Embed01]\r\nResponse is error: %d"), Status);
			AfxMessageBox(strMsg);
			return Status;
		}
	} 

	// extrafilePath 출력
	if (jsondata.HasMember("extrafilePath") && jsondata["extrafilePath"].IsString()) 
	{
		response = A2T(jsondata["extrafilePath"].GetString());
		pUploadEmbedData.strExtrafilePath.Format(_T("%s"), response);
	}

	// mime_type 출력
	if (jsondata.HasMember("mime_type") && jsondata["mime_type"].IsString()) 
	{
		response = A2T(jsondata["mime_type"].GetString());
		pUploadEmbedData.strMineType.Format(_T("%s"), response);
	}

	// res 출력
	if (jsondata.HasMember("res") && jsondata["res"].IsString()) 
	{
		response = A2T(jsondata["res"].GetString());
		pUploadEmbedData.strRes.Format(_T("%s"), response);
	}

	return Status;
}

// 입력된 파일을 서버로 전송하는 예제...
DWORD 
FileSend(IN CString strServerName,		// _T("10.10.4.10")
		IN UINT nServerPort,				// 9002
		IN CString strObject,				// _T("/upload_search01?file_folder={폴더명}")
		IN CString strFilePath,				// _T("E:\\docs\\임베딩 API 서버-Admin 가이드-202310.docx")
		OUT CStringW& strResponseW)				// 서버응답데이터		
{
	DWORD Status = ERROR_SUCCESS;

	CInternetSession session;
	CHttpConnection* pConnection = NULL;
	CHttpFile* pFile = NULL;
	CString strMsg;

	try
	{
		// REST API 서버 정보
		INTERNET_PORT nPort = nServerPort;

		// Open a connection to the server
		pConnection = session.GetHttpConnection(strServerName, nPort, _T(""), _T(""));

		// POST: Prepare the HTTP request
		pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST, strObject, NULL, 1, NULL, NULL, INTERNET_FLAG_RELOAD);

		// PPT 파일 읽기
		//CString filePath = _T("E:\\docs\\임베딩 API 서버-Admin 가이드-202310.docx");
		std::ifstream inputFile(strFilePath, std::ios::binary);
		if (!inputFile)
		{
			AfxMessageBox(_T("Failed to open file."));
			Status = GetLastError();
			return Status;
		}

		std::vector<char> buffer((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
		inputFile.close();

		// 요청 바디 생성 (multipart/form-data)
		std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
		std::string mimeType = GetMimeType(strFilePath);
		std::string utf8FileName = CStringToUTF8(strFilePath);
		std::string body;
		body += "--" + boundary + "\r\n";
		body += "Content-Disposition: form-data; name=\"file\"; filename=\"" + utf8FileName + "\"\r\n";
		body += "Content-Type: " + mimeType + "\r\n\r\n";
		body.insert(body.end(), buffer.begin(), buffer.end());
		body += "\r\n--" + boundary + "--\r\n";

		// 요청 헤더 설정
		CString headers;
		headers.Format(_T("Content-Type: multipart/form-data; boundary=%s"), CString(boundary.c_str()));
		pFile->AddRequestHeaders(headers);

		// 파일 전송
		pFile->SendRequest(NULL, 0, (LPVOID)body.c_str(), body.size());

		// Get the response status code
		DWORD dwStatusCode;
		pFile->QueryInfoStatusCode(dwStatusCode);

		//strMsg.Format(_T("status: %d"), dwStatusCode);
		//AfxMessageBox(strMsg);

		if (dwStatusCode == HTTP_STATUS_OK)
		{
			// Read the response body
			CStringA strResponseA;
			char szBuff[2048];
			UINT nRead = 0;

			while ((nRead = pFile->Read(szBuff, sizeof(szBuff) - 1)) > 0)
			{
				szBuff[nRead] = '\0';
				strResponseA  += szBuff;
			}

			// Convert UTF-8 response to Unicode
			int nLength = MultiByteToWideChar(CP_UTF8, 0, strResponseA, -1, NULL, 0);
			MultiByteToWideChar(CP_UTF8, 0, strResponseA, -1, strResponseW.GetBuffer(nLength), nLength);
			strResponseW.ReleaseBuffer();
		}
		else
		{
			// Clean up
			pFile->Close();
			pConnection->Close();

			// Handle HTTP errors
			strMsg.Format(_T("HTTP error: %d"), dwStatusCode);
			AfxMessageBox(strMsg);

			Status = dwStatusCode;
			return Status;
		}

		// Clean up
		pFile->Close();
		pConnection->Close();
	}
	catch (CInternetException* pEx)
	{
		TCHAR szErr[2048];
		pEx->GetErrorMessage(szErr, 2048);
		Status = pEx->m_dwError;

		strMsg.Format(_T("error: %d\r\n%s"), Status, szErr);
		AfxMessageBox(strMsg);
		pEx->Delete();

		return Status;
	}

	// Clean up pointers
	if (pFile)
		delete pFile;

	if (pConnection)
		delete pConnection;

	session.Close();

	return Status;
}