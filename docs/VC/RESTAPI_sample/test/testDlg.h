
// testDlg.h : 헤더 파일
//

#pragma once


// CtestDlg 대화 상자
class CtestDlg : public CDialogEx
{
// 생성입니다.
public:
	CtestDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_TEST_DIALOG };

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
