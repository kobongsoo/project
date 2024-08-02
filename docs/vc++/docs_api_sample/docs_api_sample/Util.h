#pragma once

#include <string> // std::string�� ����ϱ� ���� ��� ����

/////////////////////////////////////////////////////////////////////////////
#define MAX_OBJ_PATH_W 520
#define MAX_OBJ_PATH_A (520 * sizeof(WCHAR))
#define MAX_SIZE (512 * sizeof(WCHAR))

// ���繮���˻�������
typedef struct _SIMILAR_DOC_DATA
{
	DOUBLE score;						// ���ھ�
	WCHAR rfilename[MAX_OBJ_PATH_A];	// rfilename(���ϸ�=����ũ�Ѱ�)
} SIMILAR_DOC_DATA, *PSIMILAR_DOC_DATA;

// ���繮���Ӻ�������
typedef struct _UPLOAD_EMBED_DATA
{
	CString strExtrafilePath;	            // text ����� ���� ���
	CString strMineType;                      // mime Ÿ�� 
	CString strRes;                            // rfile_name, rfile_text, vector0~9 ��  
} UPLOAD_EMBED_DATA, *PUPLOAD_EMBED_DATA;

// �Ӻ�����������
typedef struct _RFILE_NAME_INFO
{
	WCHAR RFileName[MAX_OBJ_PATH_A]; // rfilename
	WCHAR RFileText[MAX_SIZE];			 // rfiletext
	WCHAR Vector0[MAX_SIZE];				 // ��պ���
} RFILE_NAME_INFO, *P_RFILE_NAME_INFO;
/////////////////////////////////////////////////////////////////////////////
// URL ���ڵ� ���� �Լ�
int toHex(const unsigned char& x);
const std::string URLEncodeA(const std::string& sIn);
CString URLEncode( CString strPlainText);

// GET/POST �Լ�
DWORD 
Request(BOOLEAN bPost,			// TRUE=POST, FALSE=GET
		IN CString strIp,		// IP = _T("192.168.0.61")
		IN UINT nPort,			// Port = 9431
		IN CString strObject,	// strObject = _T("/test/post?user_id=BONG")
		IN CString PostData,	// PostData = _T("{\"fields\": [\"test\"]}")
		OUT CStringW& strResponseW);

// ���� Ȯ���ڿ� ���� MIME Ÿ���� ��ȯ�ϴ� �Լ�
std::string GetMimeType(const CString& filePath);

// UTF-8 ��ȯ �Լ�
std::string CStringToUTF8(const CString& str);

// �Էµ� ������ ������ �����ϴ� ����...
DWORD 
FileSend(IN CString strServerName,		// _T("10.10.4.10")
		IN UINT nServerPort,			// 9002
		IN CString strObject,			// _T("/upload_search01?file_folder={������}")
		IN CString strFilePath,			// _T("E:\\docs\\�Ӻ��� API ����-Admin ���̵�-202310.docx")
		OUT CStringW& Response);		// �������䵥����	

// �Ӻ���������Ͼ��(/list01) �Ľ�
DWORD 
Parse_List01(IN CStringW strResponseW,   // �Ľ��� CString��
			OUT UINT& nNum,				 // ��� ���
			OUT CList<SIMILAR_DOC_DATA, SIMILAR_DOC_DATA&>* pSimilarDocList); //�Ľ��� ����� ���繮��������

// �������ε�+�˻�(/upload_search01) �Ľ�
DWORD 
Parse_Upload_Search01(IN CStringW strResponseW,   // �Ľ��� CString��
					 OUT CList<SIMILAR_DOC_DATA, SIMILAR_DOC_DATA&>* pSimilarDocList); //�Ľ��� ����� ���繮��������

// �������ε�+�Ӻ���(/upload_embed01) �Ľ�
DWORD 
Parse_Upload_Embed01(IN CStringW strResponseW,   // �Ľ��� CString��
					OUT UPLOAD_EMBED_DATA& pUploadEmbedData); //�Ľ��� ����� ���繮��������

// �Ӻ��������������(/rfile_name?rfile_name={���ϸ�})
DWORD
Parse_RFileName01(IN CStringW strResponseW,
				OUT CList<RFILE_NAME_INFO, RFILE_NAME_INFO&>* pRFileNameInfoList); // �Ľ��� ����� ����Ʈ