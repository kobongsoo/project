
// docs_api_sampleDlg.h : 헤더 파일
//
#include "Util.h"
#pragma once


// Cdocs_api_sampleDlg 대화 상자
class Cdocs_api_sampleDlg : public CDialogEx
{
// 생성입니다.
public:
	Cdocs_api_sampleDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DOCS_API_SAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_strIP;
	CString m_strPort;
	CString m_strUploadSearchDocEdit;
	CString m_strUploadSearchDoc_SaveServerFolderPath;
	afx_msg void OnBnClickedUploadSearchDocSelectBtn();
	afx_msg void OnBnClickedUploadSearchDocBtn();

	
	CList<SIMILAR_DOC_DATA, SIMILAR_DOC_DATA&> m_SimilarDocDataList;// 유사문서정보저장할 List
	CList<RFILE_NAME_INFO, RFILE_NAME_INFO&> m_RFileNameInfoList;// 임베딩된문서정보저장할 List

	UPLOAD_EMBED_DATA UploadEmbedData;

	afx_msg void OnClose();
	BOOL m_bEmbedCheck;
	CString m_strUploadEmbedDocEdit;
	CString m_strUploadEmbedDoc_SaveServerFolderPath;
	afx_msg void OnBnClickedUploadEmbedDocBtn();
	afx_msg void OnBnClickedUploadEmbedDocSelectBtn();
	afx_msg void OnBnClickedEmbedDocListBtn();
	CString m_strTotalDocNum;
	CListCtrl m_DocViewList;
	CListCtrl m_SearchDocViewList;

	void InsertItemInList(CListCtrl *pListCtrl, int nItem, int nSubItem, LPCTSTR Text);
	
	DWORD GetRFileNameInfo(IN CString strRFileName);	// RFileName을 입력받아서 파일정보(파일명,내용,벡터(vector0) 출력 

	afx_msg void OnNMRClickDocList(NMHDR *pNMHDR, LRESULT *pResult);

	CString m_strTotalSearchDocNum;
	afx_msg void OnNMRClickSearchDocList(NMHDR *pNMHDR, LRESULT *pResult);

	// 메뉴
	void OnMenuRFileNameInfo();			// 파일목록>파일정보보기메뉴
	void OnMenuSearchRFileNameInfo();	// 검색>파일정보보기메뉴

};
