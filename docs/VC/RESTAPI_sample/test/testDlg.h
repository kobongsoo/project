
// testDlg.h : ��� ����
//

#pragma once


// CtestDlg ��ȭ ����
class CtestDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CtestDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_TEST_DIALOG };

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
	afx_msg void OnBnClickedPostBtn();
	afx_msg void OnBnClickedGetBtn();
	BOOL m_bPostRadio;
	afx_msg void OnBnClickedRequestBtn();
	CString m_strIP;
	CString m_strPort;
	CString m_strGetData;
	CString m_strPostData;
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRequestTestBtn();
	afx_msg void OnBnClickedFileSelectBtn();
	CString m_FilePath;
	CString m_strFileUploadGetData;
	CString m_strFileIndexingGetData;
	afx_msg void OnBnClickedEmbeddingBtn();
	BOOL m_bNewIndexCheck;
	afx_msg void OnBnClickedSearch01Btn();
	CString m_strSearch01_FilePath;
};
