
// docs_api_sample.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// Cdocs_api_sampleApp:
// �� Ŭ������ ������ ���ؼ��� docs_api_sample.cpp�� �����Ͻʽÿ�.
//

class Cdocs_api_sampleApp : public CWinApp
{
public:
	Cdocs_api_sampleApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern Cdocs_api_sampleApp theApp;