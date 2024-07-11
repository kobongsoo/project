#pragma once

#include <string> // std::string�� ����ϱ� ���� ��� ����

// URL ���ڵ� ���� �Լ�
int toHex(const unsigned char& x);
const std::string URLEncodeA(const std::string& sIn);
CString URLEncode( CString strPlainText);

// GET/POST �Լ�
void Request(BOOLEAN bPost,		// TRUE=POST, FALSE=GET
			IN CString strIp,   // IP = _T("192.168.0.61")
			IN UINT nPort,		// Port = 9431
			IN CString strObject, // strObject = _T("/test/post?user_id=BONG")
			IN CString PostData);// PostData = _T("{\"fields\": [\"test\"]}")

// ���� Ȯ���ڿ� ���� MIME Ÿ���� ��ȯ�ϴ� �Լ�
std::string GetMimeType(const CString& filePath);

// UTF-8 ��ȯ �Լ�
std::string CStringToUTF8(const CString& str);

// �Էµ� ������ ������ �����ϴ� �Լ�...
void FileSend(IN CString strServerName,		// _T("10.10.4.10")
	IN UINT nServerPort,				// 9002
	IN CString strObject,				// _T("/upload01")
	IN CString strFilePath);			// _T("E:\\docs\\�Ӻ��� API ����-Admin ���̵�-202310.docx")