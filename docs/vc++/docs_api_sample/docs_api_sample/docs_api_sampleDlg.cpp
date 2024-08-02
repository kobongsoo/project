
// docs_api_sampleDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "docs_api_sample.h"
#include "docs_api_sampleDlg.h"
#include "afxdialogex.h"

#include "Global.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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


// Cdocs_api_sampleDlg 대화 상자




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

	// 메뉴 컨트롤
	ON_COMMAND(ID_RFILE_NAME_INFO, &Cdocs_api_sampleDlg::OnMenuRFileNameInfo)	// 파일목록>파일정보보기 메뉴
	ON_COMMAND(ID_SEARCH_RFILE_NAME_INFO, &Cdocs_api_sampleDlg::OnMenuSearchRFileNameInfo)	// 검색>파일정보보기 메뉴

	ON_NOTIFY(NM_RCLICK, IDC_SEARCH_DOC_LIST, &Cdocs_api_sampleDlg::OnNMRClickSearchDocList)
END_MESSAGE_MAP()


// Cdocs_api_sampleDlg 메시지 처리기

BOOL Cdocs_api_sampleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	
	ZeroMemory(gHomeDirPath, sizeof(gHomeDirPath));
	GetCurrentDirectory(MAX_PATH, gHomeDirPath);

	InitializeCriticalSection(&WriteLogLock);

	// 검색목록 리스트 컨트롤 초기화
	ListView_SetExtendedListViewStyle(m_SearchDocViewList.m_hWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_SearchDocViewList.InsertColumn(0, _T("no"), LVCFMT_CENTER, 90);
	m_SearchDocViewList.InsertColumn(1, _T("파일명"), LVCFMT_LEFT, 750);
	m_SearchDocViewList.InsertColumn(2, _T("스코어"), LVCFMT_LEFT, 150);

	// 문서목록 리스트 컨트롤 초기화
	ListView_SetExtendedListViewStyle(m_DocViewList.m_hWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_DocViewList.InsertColumn(0, _T("no"), LVCFMT_CENTER, 90);
	m_DocViewList.InsertColumn(1, _T("파일명"), LVCFMT_LEFT, 750);
	m_DocViewList.InsertColumn(2, _T("기타"), LVCFMT_LEFT, 150);


	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void Cdocs_api_sampleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR Cdocs_api_sampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// 업로드할 검색 문서 찾기 버튼 클릭
void Cdocs_api_sampleDlg::OnBnClickedUploadSearchDocSelectBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	WCHAR szFile[260]; // 선택된 파일 경로를 저장할 버퍼
	OPENFILENAME ofn;  // OPENFILENAME 구조체

	// 버퍼 초기화
	ZeroMemory(szFile, sizeof(szFile));
	ZeroMemory(&ofn, sizeof(ofn));

	// OPENFILENAME 구조체 초기화
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL; // 대화 상자의 소유자 윈도우 핸들
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
	ofn.lpstrFilter = _T("All Files\0*.*\0Text Files\0*.TXT\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	// 파일 선택 대화 상자 표시
	if (GetOpenFileName(&ofn) == TRUE) 
	{
		m_strUploadSearchDocEdit = ofn.lpstrFile;

		// [bong][2019-11-06] 끝에 \가 있으면 삭제
		if (m_strUploadSearchDocEdit[m_strUploadSearchDocEdit.GetLength() - 1] == '\\')
		{
			m_strUploadSearchDocEdit.SetAt(m_strUploadSearchDocEdit.GetLength() - 1, '\0');
		}

		UpdateData(FALSE);
	}
}

// 업로드 + 검색 버튼 클릭시...
void Cdocs_api_sampleDlg::OnBnClickedUploadSearchDocBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	DWORD Status = ERROR_SUCCESS;
	CString strMsg = _T("");

	UpdateData(TRUE);

	if(m_strIP.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("IP를 입력해 주십시오."));
		GetDlgItem(IDC_IP_EDIT)->SetFocus();
		return;
	}

	if(m_strPort.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Port를 입력해 주십시오."));
		GetDlgItem(IDC_PORT_EDIT)->SetFocus();
		return;
	}

	if(m_strUploadSearchDocEdit.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("검색할 문서 파일을 선택해 주십시오."));
		GetDlgItem(IDC_UPLOAD_SEARCH_DOC_EDIT)->SetFocus();
		return;
	}

	if(m_strUploadSearchDoc_SaveServerFolderPath.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("검색할 문서 파일이 저장될 서버폴더경로를 입력해 주세요."));
		GetDlgItem(IDC_UPLOAD_SEARCH_DOC_SAVE_SEARVER_FOLDER_PATH_EDIT)->SetFocus();
		return;
	}

	CString strUrlData = _T("");

	// 폴더 path만 URL인코딩 한후, URLData 만듬
	CString strURLEncodeFilePath = _T("");
	strURLEncodeFilePath.Format(_T("%s"), m_strUploadSearchDoc_SaveServerFolderPath);

	// URLData 만듬 => /upload_search01?file_folder=
	strUrlData.Format(_T("/upload_search01?file_folder=%s"), URLEncode(strURLEncodeFilePath));
	LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"[OnBnClickedUploadSearchDocBtn] strUrlData: %s", strUrlData);

	//FileSend(_T("10.10.4.10"), 9002, _T("/upload_search01?file_folder={폴더Path}"), _T("E:\\docs\\text.txt"));
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

			m_strTotalSearchDocNum.Format(_T("*총 검색수: %d"), Count);
	
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

				// 리스트에 뿌려줌
				InsertItemInList(&m_SearchDocViewList, CurrentListIndex, 0, strIndex.GetBuffer(0));		// no						// 순번 표기
				InsertItemInList(&m_SearchDocViewList, CurrentListIndex, 1, strRfileName.GetBuffer(0));	// 파일명
				InsertItemInList(&m_SearchDocViewList, CurrentListIndex, 2, strScore.GetBuffer(0));		// 스코어

				CurrentListIndex++;

				//LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"rfilename: %s, score: %f", 
				//	SimilarDocData.rfilename, SimilarDocData.score);

			}

			AfxMessageBox(_T("완료"));
		}
		else
		{
			LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"Parse_Upload_Search01 is Fail!!");
			strMsg.Format(_T("파싱 실패\r\nerror: %d"), Status);
			AfxMessageBox(strMsg);
		}
	}
	else
	{
		LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"FileSend is Fail!!");
		strMsg.Format(_T("API 호출 실패\r\nerror: %d"), Status);
		AfxMessageBox(strMsg);
	}

	UpdateData(FALSE);
}

// 업로드 + 임베딩 버튼 클릭시...
void Cdocs_api_sampleDlg::OnBnClickedUploadEmbedDocBtn()
{

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	DWORD Status = ERROR_SUCCESS;
	CString strMsg = _T("");

	UpdateData(TRUE);

	if(m_strIP.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("IP를 입력해 주십시오."));
		GetDlgItem(IDC_IP_EDIT)->SetFocus();
		return;
	}

	if(m_strPort.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Port를 입력해 주십시오."));
		GetDlgItem(IDC_PORT_EDIT)->SetFocus();
		return;
	}

	if(m_strUploadEmbedDocEdit.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("임베딩할 문서 파일을 선택해 주십시오."));
		GetDlgItem(IDC_UPLOAD_EMBED_DOC_EDIT)->SetFocus();
		return;
	}

	if(m_strUploadEmbedDoc_SaveServerFolderPath.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("임베딩할 문서 파일이 저장될 서버폴더경로를 입력해 주세요."));
		GetDlgItem(IDC_UPLOAD_EMBED_DOC_SAVE_EMBED_FOLDER_PATH_EDIT)->SetFocus();
		return;
	}

	CString strUrlData = _T("");

	// 폴더 path만 URL인코딩 한후, URLData 만듬
	CString strURLEncodeFilePath = _T("");
	strURLEncodeFilePath.Format(_T("%s"), m_strUploadEmbedDoc_SaveServerFolderPath);

	// URLData 만듬 => /upload_search01?file_folder=
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

			// res가 너무 길면 buffer small 에러가 나므로 1024만큼만 로그에 기록함.
			if(UploadEmbedData.strRes.GetLength() > 1024)
			{
				LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"res:\r\n%s", UploadEmbedData.strRes.Left(1024));
			}
			else
			{
				LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"res:\r\n%s", UploadEmbedData.strRes);
			}
			
			AfxMessageBox(_T("완료"));
		}
		else
		{
			LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"Parse_Upload_Search01 is Fail!!");
			strMsg.Format(_T("파싱 실패\r\nerror: %d"), Status);
			AfxMessageBox(strMsg);
		}
	}
	else
	{
		LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"FileSend is Fail!!");
		strMsg.Format(_T("API 호출 실패\r\nerror: %d"), Status);
		AfxMessageBox(strMsg);
	}
}

void Cdocs_api_sampleDlg::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	DeleteCriticalSection(&WriteLogLock);

	CDialogEx::OnClose();
}

// 업로드+임베딩 파일 선택버튼(...) 클릭 
void Cdocs_api_sampleDlg::OnBnClickedUploadEmbedDocSelectBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	WCHAR szFile[260]; // 선택된 파일 경로를 저장할 버퍼
	OPENFILENAME ofn;  // OPENFILENAME 구조체

	// 버퍼 초기화
	ZeroMemory(szFile, sizeof(szFile));
	ZeroMemory(&ofn, sizeof(ofn));

	// OPENFILENAME 구조체 초기화
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL; // 대화 상자의 소유자 윈도우 핸들
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
	ofn.lpstrFilter = _T("All Files\0*.*\0Text Files\0*.TXT\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	// 파일 선택 대화 상자 표시
	if (GetOpenFileName(&ofn) == TRUE) 
	{
		m_strUploadEmbedDocEdit = ofn.lpstrFile;

		// [bong][2019-11-06] 끝에 \가 있으면 삭제
		if (m_strUploadEmbedDocEdit[m_strUploadEmbedDocEdit.GetLength() - 1] == '\\')
		{
			m_strUploadEmbedDocEdit.SetAt(m_strUploadEmbedDocEdit.GetLength() - 1, '\0');
		}

		UpdateData(FALSE);
	}
}

// 임베딩 문서 목록 얻기
void Cdocs_api_sampleDlg::OnBnClickedEmbedDocListBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	DWORD Status = ERROR_SUCCESS;
	CString strMsg = _T("");

	UpdateData(TRUE);

	if(m_strIP.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("IP를 입력해 주십시오."));
		GetDlgItem(IDC_IP_EDIT)->SetFocus();
		return;
	}

	if(m_strPort.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Port를 입력해 주십시오."));
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

			m_strTotalDocNum.Format(_T("*총 문서수: %d"), nNum);
			//LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"*임베딩목록계수: %d===================================", nNum);
			
			ULONG CurrentListIndex = 0;
			CString strIndex = _T("");
			CString strRfileName = _T("");

			for(ULONG i = 0;i < Count; i++)
			{
				ZeroMemory(&SimilarDocData, sizeof(SIMILAR_DOC_DATA));
				SimilarDocData = m_SimilarDocDataList.GetNext(Pos);
				
				strIndex.Format(_T("%d"), i);
				strRfileName.Format(_T("%s"), SimilarDocData.rfilename);

				// 리스트에 뿌려줌
				InsertItemInList(&m_DocViewList, CurrentListIndex, 0, strIndex.GetBuffer(0));		// 순번						// 순번 표기
				InsertItemInList(&m_DocViewList, CurrentListIndex, 1, strRfileName.GetBuffer(0));	// rfile_name
				CurrentListIndex++;

				//LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"rfilename: %s", SimilarDocData.rfilename);
			}

			AfxMessageBox(_T("완료"));
		}
		else
		{
			LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"Parse_List01 is Fail!!");
			strMsg.Format(_T("파싱 실패\r\nerror: %d"), Status);
			AfxMessageBox(strMsg);
		}
	}
	else
	{
		LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"FileSend is Fail!!");
		strMsg.Format(_T("API 호출 실패\r\nerror: %d"), Status);
		AfxMessageBox(strMsg);
	}

	UpdateData(FALSE);
}

// 문서리스트오른쪽마우스 버튼 클릭
void Cdocs_api_sampleDlg::OnNMRClickDocList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
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

// RFileName을 입력받아서 파일정보(파일명,내용,벡터(vector0) 출력 
DWORD Cdocs_api_sampleDlg::GetRFileNameInfo(IN CString strRFileName)
{
	DWORD Status = ERROR_SUCCESS;

	if(strRFileName.IsEmpty())
	{
		AfxMessageBox(_T("파일명을 입력해 주세요."));
		return ERROR_INVALID_PARAMETER;
	}

	CString strMsg = _T("");
	BOOL bisPost = FALSE;
	CString strObject = _T("");
	CStringW strResponseW = _T("");
	CString strUrlData = _T("");

	// 폴더 path만 URL인코딩 한후, URLData 만듬
	CString strURLEncodeFilePath = _T("");
	strURLEncodeFilePath.Format(_T("%s"), strRFileName);

	// strObject 만듬 => /rfile_name01?rfile_name={파일명}
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

				strTmpFileInfo.Format(_T("*파일: %s\r\n\r\n%s\r\n\r\n*평균벡터:\r\n%s\r\n\r\n"), RFileNameInfo.RFileName, RFileNameInfo.RFileText, RFileNameInfo.Vector0);
				strFileInfo += strTmpFileInfo;
			}

			AfxMessageBox(strFileInfo);
		}
		else
		{
			LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"Parse_RFileName01 is Fail!!");
			strMsg.Format(_T("파싱 실패\r\nerror: %d"), Status);
			AfxMessageBox(strMsg);
		}

	}
	else
	{
		LogHandler(LOG_TYPE_USER_ERROR, Status, __WFILE__, __LINE__, L"FileSend is Fail!!");
		strMsg.Format(_T("API 호출 실패\r\nerror: %d"), Status);
		AfxMessageBox(strMsg);
	}

	return Status;
}

// 검색>파일정보보기메뉴 클릭
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
		AfxMessageBox(_T("IP를 입력해 주십시오."));
		GetDlgItem(IDC_IP_EDIT)->SetFocus();
		return;
	}

	if(m_strPort.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Port를 입력해 주십시오."));
		GetDlgItem(IDC_PORT_EDIT)->SetFocus();
		return;
	}

	Position = m_SearchDocViewList.GetFirstSelectedItemPosition();
	nItem = m_SearchDocViewList.GetNextSelectedItem(Position);

	if(nItem < 0)
	{
		return;
	}

	// rfile_name을 얻어옴
	strRFileName = m_SearchDocViewList.GetItemText(nItem, 1);		
	if(strRFileName.IsEmpty())
	{
		return;
	}

	//AfxMessageBox(strRfileName);

	// 파일명으로 서버에 get 요청하고 응답결과를 메시지 박스로 뿌려줌
	GetRFileNameInfo(strRFileName);

	UpdateData(FALSE);

	return;
}

// 파일목록>파일정보보기메뉴 클릭
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
		AfxMessageBox(_T("IP를 입력해 주십시오."));
		GetDlgItem(IDC_IP_EDIT)->SetFocus();
		return;
	}

	if(m_strPort.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Port를 입력해 주십시오."));
		GetDlgItem(IDC_PORT_EDIT)->SetFocus();
		return;
	}

	Position = m_DocViewList.GetFirstSelectedItemPosition();
	nItem = m_DocViewList.GetNextSelectedItem(Position);

	if(nItem < 0)
	{
		return;
	}

	// rfile_name을 얻어옴
	strRFileName = m_DocViewList.GetItemText(nItem, 1);		
	if(strRFileName.IsEmpty())
	{
		return;
	}

	//AfxMessageBox(strRfileName);
	// 파일명으로 서버에 get 요청하고 응답결과를 메시지 박스로 뿌려줌
	GetRFileNameInfo(strRFileName);
	
	UpdateData(FALSE);

	return;
}

void Cdocs_api_sampleDlg::OnNMRClickSearchDocList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
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
