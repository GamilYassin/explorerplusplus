#pragma once

typedef struct
{
	TCHAR szPath[MAX_PATH];
	LPVOID pData;
	void (* pfnCallback)(int nFolders,int nFiles,
	PULARGE_INTEGER lTotalFolderSize,LPVOID pData);
} FolderSize_t;

DWORD WINAPI	Thread_CalculateFolderSize(LPVOID lpParameter);
HRESULT			CalculateFolderSize(const TCHAR *szPath, int *nFolders, int *nFiles, PULARGE_INTEGER lTotalFolderSize);