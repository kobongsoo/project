
// testDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "test.h"
#include "testDlg.h"
#include "afxdialogex.h"

#include "Util.h"

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


// CtestDlg 대화 상자
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
	, m_strSearch01_FilePath(_T("../../data11/docs/extract/2022년_역량평가_RND(고봉수)_수정.xlsx"))
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


// CtestDlg 메시지 처리기

BOOL CtestDlg::OnInitDialog()
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

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CtestDlg::OnPaint()
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
HCURSOR CtestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CtestDlg::OnBnClickedRequestBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
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

	if(m_strGetData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("DATA를 입력해 주십시오."));
		GetDlgItem(IDC_GET_DATA_EDIT)->SetFocus();
		return;
	}

	if(m_bPostRadio == 1 && m_strPostData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Post 데이터를 입력해 주십시오."));
		GetDlgItem(IDC_POST_DATA_EDIT)->SetFocus();
		return;
	}

	if(m_bPostRadio == 1) //post 데이터인 경우
	{
		Request(m_bPostRadio, m_strIP, _ttoi(m_strPort),
			m_strGetData,
			m_strPostData);
	}
	else				//get 데이터인 경우
	{
		Request(m_bPostRadio, m_strIP, _ttoi(m_strPort),
			m_strGetData,
			_T(""));
	}
	

	return;
}

void CtestDlg::OnBnClickedRadio1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	GetDlgItem(IDC_POST_DATA_EDIT)->EnableWindow(FALSE);
}


void CtestDlg::OnBnClickedRadio2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	GetDlgItem(IDC_POST_DATA_EDIT)->EnableWindow(TRUE);
}

// 업로드 테스트 버튼
void CtestDlg::OnBnClickedRequestTestBtn()
{
	//FileSend(_T("10.10.4.10"), 9002, _T("/upload01"), _T("E:\\docs\\(2일과정)JADECROSS_istio_교재(k8s-v1.28+istio-v1.20.3)_20240313"));

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

	if(m_strFileUploadGetData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("DATA를 입력해 주십시오."));
		GetDlgItem(IDC_FILE_UPLOAD_GET_DATA_EDIT)->SetFocus();
		return;
	}

	if(m_FilePath.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("파일를 입력해 주십시오."));
		GetDlgItem(IDC_FILE_EDIT)->SetFocus();
		return;
	}

	// 파일 업로드 (m_strGetData = _T("/upload01")
	FileSend(m_strIP, _ttoi(m_strPort), m_strFileUploadGetData, m_FilePath);

	return;
}

// 업로드할 파일 선택
void CtestDlg::OnBnClickedFileSelectBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
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
		m_FilePath = ofn.lpstrFile;

		// [bong][2019-11-06] 끝에 \가 있으면 삭제
		if (m_FilePath[m_FilePath.GetLength() - 1] == '\\')
		{
			m_FilePath.SetAt(m_FilePath.GetLength() - 1, '\0');
		}

		UpdateData(FALSE);
	}
}

// 인덱싱 버튼
void CtestDlg::OnBnClickedEmbeddingBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	CString strMsg;
	//strMsg.Format(_T("선택: %d"), m_bNewIndexCheck);
	//AfxMessageBox(strMsg);

	if(m_strFileIndexingGetData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("DATA를 입력해 주십시오."));
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

// 검색
void CtestDlg::OnBnClickedSearch01Btn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	
	CString strUrlData = _T("");

	// 파일path만 URL인코딩 해줌
	CString strURLEncodeFilePath = _T("");
	strURLEncodeFilePath.Format(_T("%s"), m_strSearch01_FilePath);

	strUrlData.Format(_T("/search01?file_path=%s"), URLEncode(strURLEncodeFilePath));
	//AfxMessageBox(strUrlData);

	Request(FALSE, m_strIP, _ttoi(m_strPort), strUrlData,_T(""));
}
