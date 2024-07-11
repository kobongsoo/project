
// testDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "test.h"
#include "testDlg.h"
#include "afxdialogex.h"

#include "Util.h"

// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CtestDlg ��ȭ ����
CtestDlg::CtestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CtestDlg::IDD, pParent)
	, m_bPostRadio(TRUE)
	, m_strIP(_T("10.10.4.10"))
	, m_strPort(_T("9002"))
	, m_strGetData(_T("/embed01?user_id=alpha1"))
	, m_strPostData(_T("{\"fields\": [\"field01\", \"field02\"]}"))
	, m_FilePath(_T(""))
	, m_strFileUploadGetData(_T("/upload01"))
	, m_strFileIndexingGetData(_T("/embed01"))
	, m_bNewIndexCheck(TRUE)
	, m_strSearch01_FilePath(_T("../../data11/docs/extract/2022��_������_RND(�����)_����.xlsx"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CtestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_bPostRadio);
	DDX_Text(pDX, IDC_IP_EDIT, m_strIP);
	DDX_Text(pDX, IDC_PORT_EDIT, m_strPort);
	DDX_Text(pDX, IDC_GET_DATA_EDIT, m_strGetData);
	DDX_Text(pDX, IDC_POST_DATA_EDIT, m_strPostData);
	DDX_Text(pDX, IDC_FILE_EDIT, m_FilePath);
	DDX_Text(pDX, IDC_FILE_UPLOAD_GET_DATA_EDIT, m_strFileUploadGetData);
	DDX_Text(pDX, IDC_FILE_EMBEDDING_GET_DATA_EDIT, m_strFileIndexingGetData);
	DDX_Check(pDX, IDC_DEL_INDEX_CHECK, m_bNewIndexCheck);
	DDX_Text(pDX, IDC_SEARCH_DOCS_EDIT, m_strSearch01_FilePath);
}

BEGIN_MESSAGE_MAP(CtestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_REQUEST_BTN, &CtestDlg::OnBnClickedRequestBtn)
	ON_BN_CLICKED(IDC_RADIO1, &CtestDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CtestDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_REQUEST_TEST_BTN, &CtestDlg::OnBnClickedRequestTestBtn)
	ON_BN_CLICKED(IDC_FILE_SELECT_BTN, &CtestDlg::OnBnClickedFileSelectBtn)
	ON_BN_CLICKED(IDC_EMBEDDING_BTN, &CtestDlg::OnBnClickedEmbeddingBtn)
	ON_BN_CLICKED(IDC_SEARCH01_BTN, &CtestDlg::OnBnClickedSearch01Btn)
END_MESSAGE_MAP()


// CtestDlg �޽��� ó����

BOOL CtestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CtestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CtestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CtestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CtestDlg::OnBnClickedRequestBtn()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);

	if(m_strIP.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("IP�� �Է��� �ֽʽÿ�."));
		GetDlgItem(IDC_IP_EDIT)->SetFocus();
		return;
	}

	if(m_strPort.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Port�� �Է��� �ֽʽÿ�."));
		GetDlgItem(IDC_PORT_EDIT)->SetFocus();
		return;
	}

	if(m_strGetData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("DATA�� �Է��� �ֽʽÿ�."));
		GetDlgItem(IDC_GET_DATA_EDIT)->SetFocus();
		return;
	}

	if(m_bPostRadio == 1 && m_strPostData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Post �����͸� �Է��� �ֽʽÿ�."));
		GetDlgItem(IDC_POST_DATA_EDIT)->SetFocus();
		return;
	}

	if(m_bPostRadio == 1) //post �������� ���
	{
		Request(m_bPostRadio, m_strIP, _ttoi(m_strPort),
			m_strGetData,
			m_strPostData);
	}
	else				//get �������� ���
	{
		Request(m_bPostRadio, m_strIP, _ttoi(m_strPort),
			m_strGetData,
			_T(""));
	}
	

	return;
}

void CtestDlg::OnBnClickedRadio1()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
	GetDlgItem(IDC_POST_DATA_EDIT)->EnableWindow(FALSE);
}


void CtestDlg::OnBnClickedRadio2()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
	GetDlgItem(IDC_POST_DATA_EDIT)->EnableWindow(TRUE);
}

// ���ε� �׽�Ʈ ��ư
void CtestDlg::OnBnClickedRequestTestBtn()
{
	//FileSend(_T("10.10.4.10"), 9002, _T("/upload01"), _T("E:\\docs\\(2�ϰ���)JADECROSS_istio_����(k8s-v1.28+istio-v1.20.3)_20240313"));

	UpdateData(TRUE);

	if(m_strIP.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("IP�� �Է��� �ֽʽÿ�."));
		GetDlgItem(IDC_IP_EDIT)->SetFocus();
		return;
	}

	if(m_strPort.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Port�� �Է��� �ֽʽÿ�."));
		GetDlgItem(IDC_PORT_EDIT)->SetFocus();
		return;
	}

	if(m_strFileUploadGetData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("DATA�� �Է��� �ֽʽÿ�."));
		GetDlgItem(IDC_FILE_UPLOAD_GET_DATA_EDIT)->SetFocus();
		return;
	}

	if(m_FilePath.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("���ϸ� �Է��� �ֽʽÿ�."));
		GetDlgItem(IDC_FILE_EDIT)->SetFocus();
		return;
	}

	// ���� ���ε� (m_strGetData = _T("/upload01")
	FileSend(m_strIP, _ttoi(m_strPort), m_strFileUploadGetData, m_FilePath);

	return;
}

// ���ε��� ���� ����
void CtestDlg::OnBnClickedFileSelectBtn()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	WCHAR szFile[260]; // ���õ� ���� ��θ� ������ ����
	OPENFILENAME ofn;  // OPENFILENAME ����ü

	// ���� �ʱ�ȭ
	ZeroMemory(szFile, sizeof(szFile));
	ZeroMemory(&ofn, sizeof(ofn));

	// OPENFILENAME ����ü �ʱ�ȭ
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL; // ��ȭ ������ ������ ������ �ڵ�
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
	ofn.lpstrFilter = _T("All Files\0*.*\0Text Files\0*.TXT\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	// ���� ���� ��ȭ ���� ǥ��
	if (GetOpenFileName(&ofn) == TRUE) 
	{
		m_FilePath = ofn.lpstrFile;

		// [bong][2019-11-06] ���� \�� ������ ����
		if (m_FilePath[m_FilePath.GetLength() - 1] == '\\')
		{
			m_FilePath.SetAt(m_FilePath.GetLength() - 1, '\0');
		}

		UpdateData(FALSE);
	}
}

// �ε��� ��ư
void CtestDlg::OnBnClickedEmbeddingBtn()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);

	CString strMsg;
	//strMsg.Format(_T("����: %d"), m_bNewIndexCheck);
	//AfxMessageBox(strMsg);

	if(m_strFileIndexingGetData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("DATA�� �Է��� �ֽʽÿ�."));
		GetDlgItem(IDC_FILE_EMBEDDING_GET_DATA_EDIT)->SetFocus();
		return;
	}

	CString strFileGetData;

	if(m_bNewIndexCheck == TRUE)
	{
		strFileGetData.Format(_T("%s?user_id=test&del_index=true"), m_strFileIndexingGetData);
	}
	else
	{
		strFileGetData.Format(_T("%s?user_id=test&del_index=false"), m_strFileIndexingGetData);
	}
	

	Request(FALSE, m_strIP, _ttoi(m_strPort),strFileGetData,_T(""));
}

// �˻�
void CtestDlg::OnBnClickedSearch01Btn()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
	
	CString strUrlData = _T("");

	// ����path�� URL���ڵ� ����
	CString strURLEncodeFilePath = _T("");
	strURLEncodeFilePath.Format(_T("%s"), m_strSearch01_FilePath);

	strUrlData.Format(_T("/search01?file_path=%s"), URLEncode(strURLEncodeFilePath));
	//AfxMessageBox(strUrlData);

	Request(FALSE, m_strIP, _ttoi(m_strPort), strUrlData,_T(""));
}
