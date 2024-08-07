#include <strsafe.h>
#include <CommCtrl.h>

#include "Util.h"
#include "Log.h"
////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////
extern CRITICAL_SECTION WriteLogLock;
extern WCHAR gHomeDirPath[MAX_OBJ_PATH_W];

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

// 로그 파일 이름 : Log.cpp
#define _DOC_LOG_FILE_NAME_1				L"doc_api_1.log"
#define _DOC_LOG_FILE_NAME_2				L"doc_api_2.log"

////////////////////////////////////////////////////////////////
// TYPEDEFES
////////////////////////////////////////////////////////////////
#pragma pack(1)

#pragma pack()

////////////////////////////////////////////////////////////////
// VARIABLES
////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////

