#include "stdafx.h"
#include "Util.h"

//================================
// rapidjson ����� �߰��ϸ� ��
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
#include <vector> // std::vector�� ����ϱ� ���� ��� ����
#include <string> // std::string�� ����ϱ� ���� ��� ����
#include <map>  // std::map ��� ���� �߰�

#pragma comment(lib, "wininet.lib")

// URL ���ڵ� �Լ�
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

void Request(BOOLEAN bPost,		// TRUE=POST, FALSE=GET
	IN CString strIp,   // IP = _T("192.168.0.61")
	IN UINT nPort,		// Port = 9431
	IN CString strObject, // strObject = _T("/test/post?user_id=BONG")
	IN CString PostData)// PostData = _T("{\"fields\": [\"test\"]}")
{
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
			CStringW strResponseW;
			int nLength = MultiByteToWideChar(CP_UTF8, 0, strResponseA, -1, NULL, 0);
			MultiByteToWideChar(CP_UTF8, 0, strResponseA, -1, strResponseW.GetBuffer(nLength), nLength);
			strResponseW.ReleaseBuffer();

			// Display response body
			AfxMessageBox(_T("Response body:\n") + strResponseW);
			//===================================================
			// JSONG �Ľ�
			// CString strResponseW = L"{\"res\":\"test\", \"user_id\": \"1111\"}";
			USES_CONVERSION;
			char* strParseResponse = T2A(strResponseW);
			rapidjson::Document jsondata;
			rapidjson::ParseResult parseResult = jsondata.Parse(strParseResponse);
			//jsondata.Parse(strParseResponse);

			if (!parseResult) 
			{
				AfxMessageBox(_T("Failed to parse JSON#1"));
				return;
			}

			if (!jsondata.IsObject()) 
			{
				AfxMessageBox(_T("JSON is not an object"));
				return;
			}

			// response ���
			//if (jsondata.HasMember("response") && jsondata["response"].IsInt()) 
			if (jsondata.HasMember("res") && jsondata["user_id"].IsString()) 
			{
				//int creditsLeft = jsondata["response"].GetInt();
				CString response = A2T(jsondata["res"].GetString());
				strMsg.Format(_T("response: %s"), response);
				AfxMessageBox(strMsg);
			} 

			// user_id ���
			if (jsondata.HasMember("user_id") && jsondata["user_id"].IsString()) 
			{
				CString user_id = A2T(jsondata["user_id"].GetString());
				strMsg.Format(_T("user_id: %s"), user_id);
				AfxMessageBox(strMsg);
			}
			//===================================================
		}
		else
		{
			// Handle HTTP errors
			strMsg.Format(_T("HTTP error: %d"), dwStatusCode);
			AfxMessageBox(strMsg);
		}

		// Clean up
		pFile->Close();
		pConnection->Close();
	}
	catch (CInternetException* pEx)
	{
		TCHAR szErr[2048];
		pEx->GetErrorMessage(szErr, 2048);
		AfxMessageBox(szErr);
		pEx->Delete();
	}

	// Clean up pointers
	if (pFile)
		delete pFile;

	if (pConnection)
		delete pConnection;
	session.Close();
}

// ���� Ȯ���ڿ� ���� MIME Ÿ���� ��ȯ�ϴ� �Լ�
std::string GetMimeType(const CString& filePath)
{
	std::map<CString, std::string> mimeTypes;

	// MIME Ÿ�� ����, �ʿ信 ���� �� ���� MIME Ÿ�� �߰�
	mimeTypes[_T(".pdf")] = "application/pdf";
	mimeTypes[_T(".pptx")] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	mimeTypes[_T(".ppt")] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	mimeTypes[_T(".docx")] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	mimeTypes[_T(".doc")] = "application/msword";
	mimeTypes[_T(".jpg")] = "image/jpeg";
	mimeTypes[_T(".png")] = "image/png";
	mimeTypes[_T(".txt")] = "text/plain";

	// ������ Ȯ���� ����
	CString ext = filePath.Right(filePath.GetLength() - filePath.ReverseFind('.'));
	auto it = mimeTypes.find(ext);

	if (it != mimeTypes.end())
	{
		return it->second;
	}
	return "application/octet-stream"; // �⺻ MIME Ÿ��
}

// UTF-8 ��ȯ �Լ�
std::string CStringToUTF8(const CString& str)
{
	CT2CA pszConvertedAnsiString(str, CP_UTF8);
	std::string utf8Str(pszConvertedAnsiString);
	return utf8Str;
}

// �Էµ� ������ ������ �����ϴ� ����...
void FileSend(IN CString strServerName,		// _T("10.10.4.10")
			IN UINT nServerPort,			// 9002
			IN CString strObject,			// _T("/upload01")
			IN CString strFilePath)			// _T("E:\\docs\\�Ӻ��� API ����-Admin ���̵�-202310.docx")
{
	CInternetSession session;
	CHttpConnection* pConnection = NULL;
	CHttpFile* pFile = NULL;
	CString strMsg;

	try
	{
		// REST API ���� ����
		INTERNET_PORT nPort = nServerPort;

		// Open a connection to the server
		pConnection = session.GetHttpConnection(strServerName, nPort, _T(""), _T(""));

		// POST: Prepare the HTTP request
		pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST, strObject, NULL, 1, NULL, NULL, INTERNET_FLAG_RELOAD);

		// PPT ���� �б�
		//CString filePath = _T("E:\\docs\\�Ӻ��� API ����-Admin ���̵�-202310.docx");
		std::ifstream inputFile(strFilePath, std::ios::binary);
		if (!inputFile)
		{
			AfxMessageBox(_T("Failed to open file."));
			return;
		}

		std::vector<char> buffer((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
		inputFile.close();

		// ��û �ٵ� ���� (multipart/form-data)
		std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
		std::string mimeType = GetMimeType(strFilePath);
		std::string utf8FileName = CStringToUTF8(strFilePath);
		std::string body;
		body += "--" + boundary + "\r\n";
		body += "Content-Disposition: form-data; name=\"file\"; filename=\"" + utf8FileName + "\"\r\n";
		body += "Content-Type: " + mimeType + "\r\n\r\n";
		body.insert(body.end(), buffer.begin(), buffer.end());
		body += "\r\n--" + boundary + "--\r\n";

		// ��û ��� ����
		CString headers;
		headers.Format(_T("Content-Type: multipart/form-data; boundary=%s"), CString(boundary.c_str()));
		pFile->AddRequestHeaders(headers);

		// ���� ����
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
			CStringW strResponseW;
			int nLength = MultiByteToWideChar(CP_UTF8, 0, strResponseA, -1, NULL, 0);
			MultiByteToWideChar(CP_UTF8, 0, strResponseA, -1, strResponseW.GetBuffer(nLength), nLength);
			strResponseW.ReleaseBuffer();

			// Display response body
			AfxMessageBox(_T("Response body:\n") + strResponseW);
			//===================================================
			// JSONG �Ľ�
			// CString strResponseW = L"{\"res\":\"test\", \"user_id\": \"1111\"}";
			USES_CONVERSION;
			char* strParseResponse = T2A(strResponseW);
			rapidjson::Document jsondata;
			rapidjson::ParseResult parseResult = jsondata.Parse(strParseResponse);
			//jsondata.Parse(strParseResponse);

			if (!parseResult) 
			{
				AfxMessageBox(_T("Failed to parse JSON#1"));
				return;
			}

			if (!jsondata.IsObject()) 
			{
				AfxMessageBox(_T("JSON is not an object"));
				return;
			}

			// response ���
			//if (jsondata.HasMember("response") && jsondata["response"].IsInt()) 
			if (jsondata.HasMember("res") && jsondata["user_id"].IsString()) 
			{
				//int creditsLeft = jsondata["response"].GetInt();
				CString response = A2T(jsondata["res"].GetString());
				strMsg.Format(_T("response: %s"), response);
				AfxMessageBox(strMsg);
			} 

			// user_id ���
			if (jsondata.HasMember("user_id") && jsondata["user_id"].IsString()) 
			{
				CString user_id = A2T(jsondata["user_id"].GetString());
				strMsg.Format(_T("user_id: %s"), user_id);
				AfxMessageBox(strMsg);
			}
			//===================================================
		}
		else
		{
			// Handle HTTP errors
			strMsg.Format(_T("HTTP error: %d"), dwStatusCode);
			AfxMessageBox(strMsg);
		}

		// Clean up
		pFile->Close();
		pConnection->Close();
	}
	catch (CInternetException* pEx)
	{
		TCHAR szErr[2048];
		pEx->GetErrorMessage(szErr, 2048);
		AfxMessageBox(szErr);
		pEx->Delete();
	}

	// Clean up pointers
	if (pFile)
		delete pFile;

	if (pConnection)
		delete pConnection;

	session.Close();
}