/*
* COPYRIGHT : 2010 MOCOMSYS
* FILEPATH  : /NEOPCCom/Log.cpp
* COMMENT   : �α� ��� �Լ�
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
		if(hFile == INVALID_HANDLE_VALUE) // ������ ������
		{
			//���� ����
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

			bAllowWrite = TRUE; //���� ������ ���Ͽ� ���Ⱑ ������
			//���Ͽ� ó�� ���� ���̹Ƿ� UNICODE BOM�� �Ἥ UNICODE ���Ϸ� ����
			WriteFile(hFile, &BOM, sizeof(WCHAR), &dwWritten, NULL);
			break;
		}
		else //������ ������
		{
			FileSizeLow = GetFileSize(hFile, NULL);
			if(FileSizeLow == INVALID_FILE_SIZE)
			{
				LeaveCriticalSection(&WriteLogLock);
				CloseHandle(hFile);
				return;
			}
			if(FileSizeLow < 1000000) //���� ũ�Ⱑ 1MB ���� ������
			{
				bAllowWrite = TRUE; //1MB ���� ���� ���Ͽ� ���Ⱑ ������
				break;
			}
			CloseHandle(hFile);
			//����ũ�Ⱑ 1MB ���� ũ�� �ι�° ���� Ȯ��
			StringCchCopyW(CurrentLogFilePath, MAX_PATH, LogFile2Path); //�ι�° ���� ����(���� ���� ������)
		}
	}
	
	if(bAllowWrite == FALSE) //�� ������ ��� 1MB ���� ũ��
	{
		//������ �ۼ��Ǵ� ������ ù��° �����̶��
		if(_wcsicmp(PreLogFilePath, LogFile1Path) == 0)
		{
			StringCchCopyW(CurrentLogFilePath, MAX_PATH, LogFile2Path); //�ι�° ���� ����(���� ���� ������)
		}
		else//������ �ۼ��Ǵ� ������ �ι�° �����̶��
		{
			StringCchCopyW(CurrentLogFilePath, MAX_PATH, LogFile1Path); //ù��° ���� ����(���� ���� ������)
		}
		
		StringCchCopyW(PreLogFilePath, MAX_PATH, CurrentLogFilePath); //���� �ۼ��Ǵ� ���ϸ��� ����

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
		//���Ͽ� ó�� ���� ���̹Ƿ� UNICODE BOM�� �Ἥ UNICODE ���Ϸ� ����
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