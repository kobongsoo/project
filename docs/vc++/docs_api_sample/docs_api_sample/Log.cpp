/*
* COPYRIGHT : 2010 MOCOMSYS
* FILEPATH  : /NEOPCCom/Log.cpp
* COMMENT   : 로그 출력 함수
*/

////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Global.h"

////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////


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

CRITICAL_SECTION WriteLogLock;
WCHAR gHomeDirPath[MAX_OBJ_PATH_W];

VOID
LogHandler(IN ULONG LogType,
		   IN ULONG StatusCode,
		   IN LPCWSTR pSourceFileName,
		   IN ULONG SourceLine,
		   IN LPCWSTR pLogMessage,
		   IN ...)
{
	static WCHAR PreLogFilePath[MAX_PATH] = {0,};
	WCHAR CurrentLogFilePath[MAX_PATH] = {0,};
	WCHAR LogFile1Path[MAX_PATH] = {0,};
	WCHAR LogFile2Path[MAX_PATH] = {0,};
	
	// Byte-order mark goes at the begining of the UNICODE file
	WCHAR BOM = (WCHAR)0xFEFF;	
	DWORD dwWritten;

	if(LogType == LOG_TYPE_USER_INFORMATION)
	{
		WCHAR LogMessage[2048] = {0,};
		va_list args;
		va_start(args, pLogMessage);
		_vsnwprintf_s(LogMessage, 2048, pLogMessage, args);
		va_end(args);
		OutputDebugStringW(LogMessage);
		return;	
	}
	
	StringCchPrintfW(LogFile1Path, MAX_PATH, L"%s\\%s", gHomeDirPath, _DOC_LOG_FILE_NAME_1);

	StringCchPrintfW(LogFile2Path, MAX_PATH, L"%s\\%s", gHomeDirPath, _DOC_LOG_FILE_NAME_2);
	
	StringCchCopyW(CurrentLogFilePath, MAX_PATH, LogFile1Path);
	
	HANDLE hFile;
	BOOLEAN bAllowWrite = FALSE;
	DWORD FileSizeLow; 

	__try
	{
		EnterCriticalSection(&WriteLogLock); 
	}	
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return;
	}

	for(int i = 0; i < 2; i++)
	{
		hFile = CreateFileW(CurrentLogFilePath,	
							GENERIC_READ | GENERIC_WRITE, 
							FILE_SHARE_READ | FILE_SHARE_WRITE, 
							0, 
							OPEN_EXISTING,	
							FILE_ATTRIBUTE_NORMAL, 
							0);
		if(hFile == INVALID_HANDLE_VALUE) // 파일이 없으면
		{
			//파일 생성
			hFile = CreateFileW(CurrentLogFilePath,	
								GENERIC_READ | GENERIC_WRITE, 
								FILE_SHARE_READ | FILE_SHARE_WRITE, 
								0, 
								CREATE_NEW,	
								FILE_ATTRIBUTE_NORMAL, 
								0);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				LeaveCriticalSection(&WriteLogLock);
				return;
			}

			bAllowWrite = TRUE; //새로 생성된 파일에 쓰기가 가능함
			//파일에 처음 쓰는 것이므로 UNICODE BOM을 써서 UNICODE 파일로 변경
			WriteFile(hFile, &BOM, sizeof(WCHAR), &dwWritten, NULL);
			break;
		}
		else //파일이 있으면
		{
			FileSizeLow = GetFileSize(hFile, NULL);
			if(FileSizeLow == INVALID_FILE_SIZE)
			{
				LeaveCriticalSection(&WriteLogLock);
				CloseHandle(hFile);
				return;
			}
			if(FileSizeLow < 1000000) //파일 크기가 1MB 보다 작으면
			{
				bAllowWrite = TRUE; //1MB 보다 작은 파일에 쓰기가 가능함
				break;
			}
			CloseHandle(hFile);
			//파일크기가 1MB 보다 크면 두번째 파일 확인
			StringCchCopyW(CurrentLogFilePath, MAX_PATH, LogFile2Path); //두번째 파일 생성(기존 내용 삭제됨)
		}
	}
	
	if(bAllowWrite == FALSE) //두 파일이 모든 1MB 보다 크면
	{
		//기존에 작성되던 파일이 첫번째 파일이라면
		if(_wcsicmp(PreLogFilePath, LogFile1Path) == 0)
		{
			StringCchCopyW(CurrentLogFilePath, MAX_PATH, LogFile2Path); //두번째 파일 생성(기존 내용 삭제됨)
		}
		else//기존에 작성되던 파일이 두번째 파일이라면
		{
			StringCchCopyW(CurrentLogFilePath, MAX_PATH, LogFile1Path); //첫번째 파일 생성(기존 내용 삭제됨)
		}
		
		StringCchCopyW(PreLogFilePath, MAX_PATH, CurrentLogFilePath); //현재 작성되는 파일명을 저장

		hFile = CreateFileW(CurrentLogFilePath,	
							GENERIC_READ | GENERIC_WRITE, 
							FILE_SHARE_READ | FILE_SHARE_WRITE, 
							0, 
							CREATE_ALWAYS,	
							FILE_ATTRIBUTE_NORMAL, 
							0);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			LeaveCriticalSection(&WriteLogLock);
			return;
		}
		//파일에 처음 쓰는 것이므로 UNICODE BOM을 써서 UNICODE 파일로 변경
		WriteFile(hFile, &BOM, sizeof(WCHAR), &dwWritten, NULL);
	}

	if(SetFilePointer(hFile, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
	{
		LeaveCriticalSection(&WriteLogLock);
		CloseHandle(hFile);
		return;
	}

	SYSTEMTIME CurrentTime;
	GetLocalTime(&CurrentTime);

	WCHAR LogBuffer[4096] = {0,};
	StringCchPrintfW(LogBuffer, 4096, L"[%04d:%02d:%02d:%02d:%02d:%02d][Type: %#x][Status: %#x][%s][Ln: %d] ", CurrentTime.wYear, CurrentTime.wMonth, CurrentTime.wDay, 
		CurrentTime.wHour, CurrentTime.wMinute, CurrentTime.wSecond, LogType, StatusCode, pSourceFileName, SourceLine);

	WCHAR LogMessage[2048] = {0,};
	va_list args;
	va_start(args, pLogMessage);
	_vsnwprintf_s(LogMessage, 2048, pLogMessage, args);
	va_end(args);
	OutputDebugStringW(LogMessage);

	StringCchCatW(LogBuffer, 4096, LogMessage);
	StringCchCatW(LogBuffer, 4096, L"\r\n");

	WriteFile(hFile, LogBuffer, (DWORD)(wcslen(LogBuffer)*sizeof(WCHAR)), &dwWritten, NULL);

	CloseHandle(hFile);
	LeaveCriticalSection(&WriteLogLock);

	return;
}