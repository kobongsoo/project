
// docs_api_sampleDlg.h : ��� ����
//
#include "Util.h"
#pragma once


// Cdocs_api_sampleDlg ��ȭ ����
class Cdocs_api_sampleDlg : public CDialogEx
{
// �����Դϴ�.
public:
	Cdocs_api_sampleDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DOCS_API_SAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
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

	
	CList<SIMILAR_DOC_DATA, SIMILAR_DOC_DATA&> m_SimilarDocDataList;// ���繮������������ List
	CList<RFILE_NAME_INFO, RFILE_NAME_INFO&> m_RFileNameInfoList;// �Ӻ����ȹ������������� List

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
	
	DWORD GetRFileNameInfo(IN CString strRFileName);	// RFileName�� �Է¹޾Ƽ� ��������(���ϸ�,����,����(vector0) ��� 

	afx_msg void OnNMRClickDocList(NMHDR *pNMHDR, LRESULT *pResult);

	CString m_strTotalSearchDocNum;
	afx_msg void OnNMRClickSearchDocList(NMHDR *pNMHDR, LRESULT *pResult);

	// �޴�
	void OnMenuRFileNameInfo();			// ���ϸ��>������������޴�
	void OnMenuSearchRFileNameInfo();	// �˻�>������������޴�

};
