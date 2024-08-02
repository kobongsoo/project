
// docs_api_sampleDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "docs_api_sample.h"
#include "docs_api_sampleDlg.h"
#include "afxdialogex.h"

#include "Global.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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


// Cdocs_api_sampleDlg ��ȭ ����




Cdocs_api_sampleDlg::Cdocs_api_sampleDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cdocs_api_sampleDlg::IDD, pParent)
	, m_strIP(_T("10.10.4.10"))
	, m_strPort(_T("9002"))
	, m_strUploadSearchDocEdit(_T(""))
	, m_strUploadSearchDoc_SaveServerFolderPath(_T("../../data11/docs_search"))
	, m_bEmbedCheck(TRUE)
	, m_strUploadEmbedDocEdit(_T(""))
	, m_strUploadEmbedDoc_SaveServerFolderPath(_T("../../data11/docs_embed"))
	, m_strTotalDocNum(_T(""))
	, m_strTotalSearchDocNum(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cdocs_api_sampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_IP_EDIT, m_strIP);
	DDX_Text(pDX, IDC_PORT_EDIT, m_strPort);
	DDX_Text(pDX, IDC_UPLOAD_SEARCH_DOC_EDIT, m_strUploadSearchDocEdit);
	DDV_MaxChars(pDX, m_strUploadSearchDocEdit, 250);
	DDX_Text(pDX, IDC_UPLOAD_SEARCH_DOC_SAVE_SEARVER_FOLDER_PATH_EDIT, m_strUploadSearchDoc_SaveServerFolderPath);
	DDV_MaxChars(pDX, m_strUploadSearchDoc_SaveServerFolderPath, 250);
	DDX_Check(pDX, IDC_EMBED_CHECK, m_bEmbedCheck);
	DDX_Text(pDX, IDC_UPLOAD_EMBED_DOC_EDIT, m_strUploadEmbedDocEdit);
	DDV_MaxChars(pDX, m_strUploadEmbedDocEdit, 250);
	DDX_Text(pDX, IDC_UPLOAD_EMBED_DOC_SAVE_EMBED_FOLDER_PATH_EDIT, m_strUploadEmbedDoc_SaveServerFolderPath);
	DDV_MaxChars(pDX, m_strUploadEmbedDoc_SaveServerFolderPath, 250);
	DDX_Text(pDX, IDC_TOTAL_DOC_NUM_STATIC, m_strTotalDocNum);
	DDX_Control(pDX, IDC_DOC_LIST, m_DocViewList);
	DDX_Text(pDX, IDC_TOTAL_SEARCH_DOC_NUM_STATIC, m_strTotalSearchDocNum);
	DDX_Control(pDX, IDC_SEARCH_DOC_LIST, m_SearchDocViewList);
}

BEGIN_MESSAGE_MAP(Cdocs_api_sampleDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_UPLOAD_SEARCH_DOC_SELECT_BTN, &Cdocs_api_sampleDlg::OnBnClickedUploadSearchDocSelectBtn)
	ON_BN_CLICKED(IDC_UPLOAD_SEARCH_DOC_BTN, &Cdocs_api_sampleDlg::OnBnClickedUploadSearchDocBtn)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_UPLOAD_EMBED_DOC_BTN, &Cdocs_api_sampleDlg::OnBnClickedUploadEmbedDocBtn)
	ON_BN_CLICKED(IDC_UPLOAD_EMBED_DOC_SELECT_BTN, &Cdocs_api_sampleDlg::OnBnClickedUploadEmbedDocSelectBtn)
	ON_BN_CLICKED(IDC_EMBED_DOC_LIST_BTN, &Cdocs_api_sampleDlg::OnBnClickedEmbedDocListBtn)
	ON_NOTIFY(NM_RCLICK, IDC_DOC_LIST, &Cdocs_api_sampleDlg::OnNMRClickDocList)

	// �޴� ��Ʈ��
	ON_COMMAND(ID_RFILE_NAME_INFO, &Cdocs_api_sampleDlg::OnMenuRFileNameInfo)	// ���ϸ��>������������ �޴�
	ON_COMMAND(ID_SEARCH_RFILE_NAME_INFO, &Cdocs_api_sampleDlg::OnMenuSearchRFileNameInfo)	// �˻�>������������ �޴�

	ON_NOTIFY(NM_RCLICK, IDC_SEARCH_DOC_LIST, &Cdocs_api_sampleDlg::OnNMRClickSearchDocList)
END_MESSAGE_MAP()


// Cdocs_api_sampleDlg �޽��� ó����

BOOL Cdocs_api_sampleDlg::OnInitDialog()
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
	
	ZeroMemory(gHomeDirPath, sizeof(gHomeDirPath));
	GetCurrentDirectory(MAX_PATH, gHomeDirPath);

	InitializeCriticalSection(&WriteLogLock);

	// �˻���� ����Ʈ ��Ʈ�� �ʱ�ȭ
	ListView_SetExtendedListViewStyle(m_SearchDocViewList.m_hWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_SearchDocViewList.InsertColumn(0, _T("no"), LVCFMT_CENTER, 90);
	m_SearchDocViewList.InsertColumn(1, _T("���ϸ�"), LVCFMT_LEFT, 750);
	m_SearchDocViewList.InsertColumn(2, _T("���ھ�"), LVCFMT_LEFT, 150);

	// ������� ����Ʈ ��Ʈ�� �ʱ�ȭ
	ListView_SetExtendedListViewStyle(m_DocViewList.m_hWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_DocViewList.InsertColumn(0, _T("no"), LVCFMT_CENTER, 90);
	m_DocViewList.InsertColumn(1, _T("���ϸ�"), LVCFMT_LEFT, 750);
	m_DocViewList.InsertColumn(2, _T("��Ÿ"), LVCFMT_LEFT, 150);


	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void Cdocs_api_sampleDlg::InsertItemInList(CListCtrl *pListCtrl, 
											int nItem, 
											int nSubItem, 
											LPCTSTR Text)
{
	if(nSubItem == 0)
	{
		pListCtrl->InsertItem(nItem, Text);
	}
	else
	{
		pListCtrl->SetItemText(nItem, nSubItem, Text);
	}
}

void Cdocs_api_sampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void Cdocs_api_sampleDlg::OnPaint()
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
HCURSOR Cdocs_api_sampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// ���ε��� �˻� ���� ã�� ��ư Ŭ��
void Cdocs_api_sampleDlg::OnBnClickedUploadSearchDocSelectBtn()
{
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
		m_strUploadSearchDocEdit = ofn.lpstrFile;

		// [bong][2019-11-06] ���� \�� ������ ����
		if (m_strUploadSearchDocEdit[m_strUploadSearchDocEdit.GetLength() - 1] == '\\')
		{
			m_strUploadSearchDocEdit.SetAt(m_strUploadSearchDocEdit.GetLength() - 1, '\0');
		}

		UpdateData(FALSE);
	}
}

// ���ε� + �˻� ��ư Ŭ����...
void Cdocs_api_sampleDlg::OnBnClickedUploadSearchDocBtn()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	DWORD Status = ERROR_SUCCESS;
	CString strMsg = _T("");

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

	if(m_strUploadSearchDocEdit.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("�˻��� ���� ������ ������ �ֽʽÿ�."));
		GetDlgItem(IDC_UPLOAD_SEARCH_DOC_EDIT)->SetFocus();
		return;
	}

	if(m_strUploadSearchDoc_SaveServerFolderPath.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("�˻��� ���� ������ ����� ����������θ� �Է��� �ּ���."));
		GetDlgItem(IDC_UPLOAD_SEARCH_DOC_SAVE_SEARVER_FOLDER_PATH_EDIT)->SetFocus();
		return;
	}

	CString strUrlData = _T("");

	// ���� path�� URL���ڵ� ����, URLData ����
	CString strURLEncodeFilePath = _T("");
	strURLEncodeFilePath.Format(_T("%s"), m_strUploadSearchDoc_SaveServerFolderPath);

	// URLData ���� => /upload_search01?file_folder=
	strUrlData.Format(_T("/upload_search01?file_folder=%s"), URLEncode(strURLEncodeFilePath));
	LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"[OnBnClickedUploadSearchDocBtn] strUrlData: %s", strUrlData);

	//FileSend(_T("10.10.4.10"), 9002, _T("/upload_search01?file_folder={����Path}"), _T("E:\\docs\\text.txt"));
	CStringW strResponseW = _T("");
	Status = FileSend(m_strIP, _ttoi(m_strPort), strUrlData, m_strUploadSearchDocEdit, strResponseW);
	if(Status == ERROR_SUCCESS)
	{
		Status = Parse_Upload_Search01(strResponseW, &m_SimilarDocDataList);
		if(Status==ERROR_SUCCESS)
		{
			SIMILAR_DOC_DATA SimilarDocData;
			ULONG Count = (ULONG)m_SimilarDocDataList.GetCount();
			POSITION Pos = m_SimilarDocDataList.GetHeadPosition();

			m_strTotalSearchDocNum.Format(_T("*�� �˻���: %d"), Count);
	
			ULONG CurrentListIndex = 0;
			CString strIndex = _T("");
			CString strRfileName = _T("");
			CString strScore = _T("");

			for(ULONG i = 0;i < Count; i++)
			{
				ZeroMemory(&SimilarDocData, sizeof(SIMILAR_DOC_DATA));
				SimilarDocData = m_SimilarDocDataList.GetNext(Pos);

				strIndex.Format(_T("%d"), i);
				strRfileName.Format(_T("%s"), SimilarDocData.rfilename);
				strScore.Format(_T("%f"), SimilarDocData.score);

				// ����Ʈ�� �ѷ���
				InsertItemInList(&m_SearchDocViewList, CurrentListIndex, 0, strIndex.GetBuffer(0));		// no						// ���� ǥ��
				InsertItemInList(&m_SearchDocViewList, CurrentListIndex, 1, strRfileName.GetBuffer(0));	// ���ϸ�
				InsertItemInList(&m_SearchDocViewList, CurrentListIndex, 2, strScore.GetBuffer(0));		// ���ھ�

				CurrentListIndex++;

				//LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"rfilename: %s, score: %f", 
				//	SimilarDocData.rfilename, SimilarDocData.score);

			}

			AfxMessageBox(_T("�Ϸ�"));
		}
		else
		{
			LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"Parse_Upload_Search01 is Fail!!");
			strMsg.Format(_T("�Ľ� ����\r\nerror: %d"), Status);
			AfxMessageBox(strMsg);
		}
	}
	else
	{
		LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"FileSend is Fail!!");
		strMsg.Format(_T("API ȣ�� ����\r\nerror: %d"), Status);
		AfxMessageBox(strMsg);
	}

	UpdateData(FALSE);
}

// ���ε� + �Ӻ��� ��ư Ŭ����...
void Cdocs_api_sampleDlg::OnBnClickedUploadEmbedDocBtn()
{

	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	DWORD Status = ERROR_SUCCESS;
	CString strMsg = _T("");

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

	if(m_strUploadEmbedDocEdit.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("�Ӻ����� ���� ������ ������ �ֽʽÿ�."));
		GetDlgItem(IDC_UPLOAD_EMBED_DOC_EDIT)->SetFocus();
		return;
	}

	if(m_strUploadEmbedDoc_SaveServerFolderPath.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("�Ӻ����� ���� ������ ����� ����������θ� �Է��� �ּ���."));
		GetDlgItem(IDC_UPLOAD_EMBED_DOC_SAVE_EMBED_FOLDER_PATH_EDIT)->SetFocus();
		return;
	}

	CString strUrlData = _T("");

	// ���� path�� URL���ڵ� ����, URLData ����
	CString strURLEncodeFilePath = _T("");
	strURLEncodeFilePath.Format(_T("%s"), m_strUploadEmbedDoc_SaveServerFolderPath);

	// URLData ���� => /upload_search01?file_folder=
	strUrlData.Format(_T("/upload_embed01?file_folder=%s&bisEmbedding=%d"), URLEncode(strURLEncodeFilePath), m_bEmbedCheck);
	
	LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"[OnBnClickedUploadEmbedDocBtn] strUrlData: %s", strUrlData);

	CStringW strResponseW = _T("");
	Status = FileSend(m_strIP, _ttoi(m_strPort), strUrlData, m_strUploadEmbedDocEdit, strResponseW);
	if(Status == ERROR_SUCCESS)
	{
		Status = Parse_Upload_Embed01(strResponseW, UploadEmbedData);
		if(Status==ERROR_SUCCESS)
		{
			LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"extrafilePath: %s, mime_type: %s", 
				UploadEmbedData.strExtrafilePath, UploadEmbedData.strMineType);

			// res�� �ʹ� ��� buffer small ������ ���Ƿ� 1024��ŭ�� �α׿� �����.
			if(UploadEmbedData.strRes.GetLength() > 1024)
			{
				LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"res:\r\n%s", UploadEmbedData.strRes.Left(1024));
			}
			else
			{
				LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"res:\r\n%s", UploadEmbedData.strRes);
			}
			
			AfxMessageBox(_T("�Ϸ�"));
		}
		else
		{
			LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"Parse_Upload_Search01 is Fail!!");
			strMsg.Format(_T("�Ľ� ����\r\nerror: %d"), Status);
			AfxMessageBox(strMsg);
		}
	}
	else
	{
		LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"FileSend is Fail!!");
		strMsg.Format(_T("API ȣ�� ����\r\nerror: %d"), Status);
		AfxMessageBox(strMsg);
	}
}

void Cdocs_api_sampleDlg::OnClose()
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	DeleteCriticalSection(&WriteLogLock);

	CDialogEx::OnClose();
}

// ���ε�+�Ӻ��� ���� ���ù�ư(...) Ŭ�� 
void Cdocs_api_sampleDlg::OnBnClickedUploadEmbedDocSelectBtn()
{
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
		m_strUploadEmbedDocEdit = ofn.lpstrFile;

		// [bong][2019-11-06] ���� \�� ������ ����
		if (m_strUploadEmbedDocEdit[m_strUploadEmbedDocEdit.GetLength() - 1] == '\\')
		{
			m_strUploadEmbedDocEdit.SetAt(m_strUploadEmbedDocEdit.GetLength() - 1, '\0');
		}

		UpdateData(FALSE);
	}
}

// �Ӻ��� ���� ��� ���
void Cdocs_api_sampleDlg::OnBnClickedEmbedDocListBtn()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	DWORD Status = ERROR_SUCCESS;
	CString strMsg = _T("");

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

	BOOL bisPost = FALSE;
	CString strObject = _T("/list01");
	CStringW strResponseW = _T("");
	
	Status = Request(bisPost, m_strIP, _ttoi(m_strPort), strObject, _T(""), strResponseW);
	if(Status == ERROR_SUCCESS)
	{
		UINT nNum = 0;
		Status = Parse_List01(strResponseW, nNum, &m_SimilarDocDataList);
		if(Status==ERROR_SUCCESS)
		{
			SIMILAR_DOC_DATA SimilarDocData;
			ULONG Count = (ULONG)m_SimilarDocDataList.GetCount();
			POSITION Pos = m_SimilarDocDataList.GetHeadPosition();

			m_strTotalDocNum.Format(_T("*�� ������: %d"), nNum);
			//LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"*�Ӻ�����ϰ��: %d===================================", nNum);
			
			ULONG CurrentListIndex = 0;
			CString strIndex = _T("");
			CString strRfileName = _T("");

			for(ULONG i = 0;i < Count; i++)
			{
				ZeroMemory(&SimilarDocData, sizeof(SIMILAR_DOC_DATA));
				SimilarDocData = m_SimilarDocDataList.GetNext(Pos);
				
				strIndex.Format(_T("%d"), i);
				strRfileName.Format(_T("%s"), SimilarDocData.rfilename);

				// ����Ʈ�� �ѷ���
				InsertItemInList(&m_DocViewList, CurrentListIndex, 0, strIndex.GetBuffer(0));		// ����						// ���� ǥ��
				InsertItemInList(&m_DocViewList, CurrentListIndex, 1, strRfileName.GetBuffer(0));	// rfile_name
				CurrentListIndex++;

				//LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"rfilename: %s", SimilarDocData.rfilename);
			}

			AfxMessageBox(_T("�Ϸ�"));
		}
		else
		{
			LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"Parse_List01 is Fail!!");
			strMsg.Format(_T("�Ľ� ����\r\nerror: %d"), Status);
			AfxMessageBox(strMsg);
		}
	}
	else
	{
		LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"FileSend is Fail!!");
		strMsg.Format(_T("API ȣ�� ����\r\nerror: %d"), Status);
		AfxMessageBox(strMsg);
	}

	UpdateData(FALSE);
}

// ��������Ʈ�����ʸ��콺 ��ư Ŭ��
void Cdocs_api_sampleDlg::OnNMRClickDocList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CPoint Pos;
	CMenu DocMenu;
	CMenu* pDocSubMenu = NULL;

	if(DocMenu.LoadMenu(IDR_MENU) == FALSE)
	{
		return;
	}

	pDocSubMenu = DocMenu.GetSubMenu(0);

	if(pDocSubMenu == NULL)
	{
		DocMenu.DestroyMenu();
		return;
	}

	GetCursorPos(&Pos);

	pDocSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, Pos.x, Pos.y, this);
	DocMenu.DestroyMenu();

	*pResult = 0;
}

// RFileName�� �Է¹޾Ƽ� ��������(���ϸ�,����,����(vector0) ��� 
DWORD Cdocs_api_sampleDlg::GetRFileNameInfo(IN CString strRFileName)
{
	DWORD Status = ERROR_SUCCESS;

	if(strRFileName.IsEmpty())
	{
		AfxMessageBox(_T("���ϸ��� �Է��� �ּ���."));
		return ERROR_INVALID_PARAMETER;
	}

	CString strMsg = _T("");
	BOOL bisPost = FALSE;
	CString strObject = _T("");
	CStringW strResponseW = _T("");
	CString strUrlData = _T("");

	// ���� path�� URL���ڵ� ����, URLData ����
	CString strURLEncodeFilePath = _T("");
	strURLEncodeFilePath.Format(_T("%s"), strRFileName);

	// strObject ���� => /rfile_name01?rfile_name={���ϸ�}
	strObject.Format(_T("/rfile_name01?rfile_name=%s"), URLEncode(strURLEncodeFilePath));

	Status = Request(bisPost, m_strIP, _ttoi(m_strPort), strObject, _T(""), strResponseW);
	if(Status == ERROR_SUCCESS)
	{
		//AfxMessageBox(strResponseW);
		if(strResponseW.GetLength() > 1024)
		{
			strMsg.Format(_T("%s"), strResponseW.Left(1024));
			LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"%s", strMsg);
		}
		else
		{
			LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"%s", strResponseW);
		}

		Status = Parse_RFileName01(strResponseW, &m_RFileNameInfoList);
		if(Status == ERROR_SUCCESS)
		{
			RFILE_NAME_INFO RFileNameInfo;
			ULONG Count = (ULONG)m_RFileNameInfoList.GetCount();
			POSITION Pos = m_RFileNameInfoList.GetHeadPosition();

			CString strFileInfo = _T("");

			for(ULONG i = 0;i < Count; i++)
			{
				CString strTmpFileInfo = _T("");

				ZeroMemory(&RFileNameInfo, sizeof(RFILE_NAME_INFO));
				RFileNameInfo = m_RFileNameInfoList.GetNext(Pos);

				strTmpFileInfo.Format(_T("*����: %s\r\n\r\n%s\r\n\r\n*��պ���:\r\n%s\r\n\r\n"), RFileNameInfo.RFileName, RFileNameInfo.RFileText, RFileNameInfo.Vector0);
				strFileInfo += strTmpFileInfo;
			}

			AfxMessageBox(strFileInfo);
		}
		else
		{
			LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"Parse_RFileName01 is Fail!!");
			strMsg.Format(_T("�Ľ� ����\r\nerror: %d"), Status);
			AfxMessageBox(strMsg);
		}

	}
	else
	{
		LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"FileSend is Fail!!");
		strMsg.Format(_T("API ȣ�� ����\r\nerror: %d"), Status);
		AfxMessageBox(strMsg);
	}

	return Status;
}

// �˻�>������������޴� Ŭ��
void Cdocs_api_sampleDlg::OnMenuSearchRFileNameInfo()
{
	DWORD Status = ERROR_SUCCESS;

	int nItem = -1;
	POSITION Position;
	CString strRFileName = _T("");
	BOOLEAN bBackupLocalDisk = FALSE;

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

	Position = m_SearchDocViewList.GetFirstSelectedItemPosition();
	nItem = m_SearchDocViewList.GetNextSelectedItem(Position);

	if(nItem < 0)
	{
		return;
	}

	// rfile_name�� ����
	strRFileName = m_SearchDocViewList.GetItemText(nItem, 1);		
	if(strRFileName.IsEmpty())
	{
		return;
	}

	//AfxMessageBox(strRfileName);

	// ���ϸ����� ������ get ��û�ϰ� �������� �޽��� �ڽ��� �ѷ���
	GetRFileNameInfo(strRFileName);

	UpdateData(FALSE);

	return;
}

// ���ϸ��>������������޴� Ŭ��
void Cdocs_api_sampleDlg::OnMenuRFileNameInfo()
{
	DWORD Status = ERROR_SUCCESS;

	int nItem = -1;
	POSITION Position;
	CString strRFileName = _T("");
	BOOLEAN bBackupLocalDisk = FALSE;

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

	Position = m_DocViewList.GetFirstSelectedItemPosition();
	nItem = m_DocViewList.GetNextSelectedItem(Position);

	if(nItem < 0)
	{
		return;
	}

	// rfile_name�� ����
	strRFileName = m_DocViewList.GetItemText(nItem, 1);		
	if(strRFileName.IsEmpty())
	{
		return;
	}

	//AfxMessageBox(strRfileName);
	// ���ϸ����� ������ get ��û�ϰ� �������� �޽��� �ڽ��� �ѷ���
	GetRFileNameInfo(strRFileName);
	
	UpdateData(FALSE);

	return;
}

void Cdocs_api_sampleDlg::OnNMRClickSearchDocList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CPoint Pos;
	CMenu DocMenu;
	CMenu* pDocSubMenu = NULL;

	if(DocMenu.LoadMenu(IDR_MENU) == FALSE)
	{
		return;
	}

	pDocSubMenu = DocMenu.GetSubMenu(1);

	if(pDocSubMenu == NULL)
	{
		DocMenu.DestroyMenu();
		return;
	}

	GetCursorPos(&Pos);

	pDocSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, Pos.x, Pos.y, this);
	DocMenu.DestroyMenu();

	*pResult = 0;
}
