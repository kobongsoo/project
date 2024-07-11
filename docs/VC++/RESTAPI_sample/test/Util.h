#pragma once

#include <string> // std::string을 사용하기 위한 헤더 파일

// URL 인코딩 관련 함수
int toHex(const unsigned char& x);
const std::string URLEncodeA(const std::string& sIn);
CString URLEncode( CString strPlainText);

// GET/POST 함수
void Request(BOOLEAN bPost,		// TRUE=POST, FALSE=GET
			IN CString strIp,   // IP = _T("192.168.0.61")
			IN UINT nPort,		// Port = 9431
			IN CString strObject, // strObject = _T("/test/post?user_id=BONG")
			IN CString PostData);// PostData = _T("{\"fields\": [\"test\"]}")

// 파일 확장자에 따라 MIME 타입을 반환하는 함수
std::string GetMimeType(const CString& filePath);

// UTF-8 변환 함수
std::string CStringToUTF8(const CString& str);

// 입력된 파일을 서버로 전송하는 함수...
void FileSend(IN CString strServerName,		// _T("10.10.4.10")
	IN UINT nServerPort,				// 9002
	IN CString strObject,				// _T("/upload01")
	IN CString strFilePath);			// _T("E:\\docs\\임베딩 API 서버-Admin 가이드-202310.docx")