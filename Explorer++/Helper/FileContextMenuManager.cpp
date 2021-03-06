/******************************************************************
 *
 * Project: Helper
 * File: FileContextMenuManager.cpp
 * License: GPL - See LICENSE in the top level directory
 *
 * Manages the file context menu.
 *
 * Written by David Erceg
 * www.explorerplusplus.com
 *
 *****************************************************************/

#include "stdafx.h"
#include <vector>
#include "ShellHelper.h"
#include "FileContextMenuManager.h"
#include "StatusBar.h"
#include "Macros.h"


LRESULT CALLBACK ShellMenuHookProcStub(HWND hwnd,UINT Msg,WPARAM wParam,
	LPARAM lParam,UINT_PTR uIdSubclass,DWORD_PTR dwRefData);

CFileContextMenuManager::CFileContextMenuManager(HWND hwnd,
	LPCITEMIDLIST pidlParent, const std::list<LPITEMIDLIST> &pidlItemList) :
m_hwnd(hwnd),
m_pidlParent(ILClone(pidlParent)),
m_pShellContext3(NULL),
m_pShellContext2(NULL),
m_pShellContext(NULL)
{
	IContextMenu *pContextMenu = NULL;
	HRESULT hr;

	for(auto pidl : pidlItemList)
	{
		m_pidlItemList.push_back(ILClone(pidl));
	}

	m_pActualContext = NULL;

	if(pidlItemList.size() == 0)
	{
		IShellFolder *pShellParentFolder = NULL;
		LPCITEMIDLIST pidlRelative = NULL;

		hr = SHBindToParent(pidlParent, IID_PPV_ARGS(&pShellParentFolder),
			&pidlRelative);

		if(SUCCEEDED(hr))
		{
			hr = GetUIObjectOf(pShellParentFolder, hwnd, 1,
				&pidlRelative, IID_PPV_ARGS(&pContextMenu));

			pShellParentFolder->Release();
		}
	}
	else
	{
		IShellFolder *pShellFolder = NULL;
		hr = BindToIdl(pidlParent, IID_PPV_ARGS(&pShellFolder));

		if(SUCCEEDED(hr))
		{
			std::vector<LPITEMIDLIST> pidlItemVector(pidlItemList.begin(),pidlItemList.end());

			hr = GetUIObjectOf(pShellFolder, hwnd, static_cast<UINT>(pidlItemList.size()),
				const_cast<LPCITEMIDLIST *>(&pidlItemVector[0]), IID_PPV_ARGS(&pContextMenu));

			pShellFolder->Release();
		}
	}

	if(SUCCEEDED(hr))
	{
		/* First, try to get IContextMenu3, then IContextMenu2, and if neither of these
		are available, IContextMenu. */
		hr = pContextMenu->QueryInterface(IID_PPV_ARGS(&m_pShellContext3));
		m_pActualContext = m_pShellContext3;

		if(FAILED(hr))
		{
			hr = pContextMenu->QueryInterface(IID_PPV_ARGS(&m_pShellContext2));
			m_pActualContext = m_pShellContext2;

			if(FAILED(hr))
			{
				hr = pContextMenu->QueryInterface(IID_PPV_ARGS(&m_pShellContext));
				m_pActualContext = m_pShellContext;
			}
		}
	}

	if(pContextMenu != NULL)
	{
		pContextMenu->Release();
	}
}

CFileContextMenuManager::~CFileContextMenuManager()
{
	for(auto pidl : m_pidlItemList)
	{
		CoTaskMemFree(pidl);
	}

	CoTaskMemFree(m_pidlParent);

	if(m_pShellContext3 != NULL)
	{
		m_pShellContext3->Release();
	}
	else if(m_pShellContext2 != NULL)
	{
		m_pShellContext2->Release();
	}
	else if(m_pShellContext != NULL)
	{
		m_pShellContext->Release();
	}
}

HRESULT CFileContextMenuManager::ShowMenu(IFileContextMenuExternal *pfcme,
	int iMinID,int iMaxID,const POINT *ppt,CStatusBar *pStatusBar,
	DWORD_PTR dwData,BOOL bRename,BOOL bExtended)
{
	if(m_pActualContext == NULL)
	{
		return E_FAIL;
	}

	if(pfcme == NULL ||
		iMaxID <= iMinID ||
		ppt == NULL)
	{
		return E_FAIL;
	}

	m_pStatusBar = pStatusBar;

	m_iMinID = iMinID;
	m_iMaxID = iMaxID;

	HMENU hMenu = CreatePopupMenu();

	if(hMenu == NULL)
	{
		return E_FAIL;
	}

	UINT uFlags = CMF_NORMAL;

	if(bExtended)
	{
		uFlags |= CMF_EXTENDEDVERBS;
	}

	if(bRename)
	{
		uFlags |= CMF_CANRENAME;
	}

	HRESULT hr = m_pActualContext->QueryContextMenu(hMenu,0,iMinID,
		iMaxID,uFlags);

	if(FAILED(hr))
	{
		return hr;
	}

	/* Allow the caller to add custom entries to the menu. */
	pfcme->AddMenuEntries(m_pidlParent,m_pidlItemList,dwData,hMenu);

	BOOL bWindowSubclassed = FALSE;

	if(m_pShellContext3 != NULL || m_pShellContext2 != NULL)
	{
		/* Subclass the owner window, so that the shell can handle menu messages. */
		bWindowSubclassed = SetWindowSubclass(m_hwnd,ShellMenuHookProcStub,CONTEXT_MENU_SUBCLASS_ID,
			reinterpret_cast<DWORD_PTR>(this));
	}

	int iCmd = TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_RETURNCMD,ppt->x,ppt->y,
		0,m_hwnd,NULL);

	if(bWindowSubclassed)
	{
		/* Restore previous window procedure. */
		RemoveWindowSubclass(m_hwnd,ShellMenuHookProcStub,CONTEXT_MENU_SUBCLASS_ID);
	}

	/* Was a shell menu item selected, or one of the
	custom entries? */
	if(iCmd >= iMinID && iCmd <= iMaxID)
	{
		TCHAR szCmd[64];

		hr = m_pActualContext->GetCommandString(iCmd - iMinID,GCS_VERB,
			NULL,reinterpret_cast<LPSTR>(szCmd),SIZEOF_ARRAY(szCmd));

		BOOL bHandled = FALSE;

		/* Pass the menu back to the caller to give
		it the chance to handle it. */
		if(SUCCEEDED(hr))
		{
			bHandled = pfcme->HandleShellMenuItem(m_pidlParent,m_pidlItemList,dwData,szCmd);
		}

		if(!bHandled)
		{
			CMINVOKECOMMANDINFO	cmici;

			cmici.cbSize		= sizeof(CMINVOKECOMMANDINFO);
			cmici.fMask			= 0;
			cmici.hwnd			= m_hwnd;
			cmici.lpVerb		= (LPCSTR)MAKEWORD(iCmd - iMinID,0);
			cmici.lpParameters	= NULL;
			cmici.lpDirectory	= NULL;
			cmici.nShow			= SW_SHOW;

			m_pActualContext->InvokeCommand(&cmici);
		}
	}
	else
	{
		/* Custom menu entry, so pass back
		to caller. */
		pfcme->HandleCustomMenuItem(m_pidlParent,m_pidlItemList,iCmd);
	}

	/* Do NOT destroy the menu until AFTER
	the command has been executed. Items
	on the "Send to" submenu may not work,
	for example, if this item is destroyed
	earlier. */
	DestroyMenu(hMenu);

	return S_OK;
}

LRESULT CALLBACK ShellMenuHookProcStub(HWND hwnd,UINT Msg,WPARAM wParam,
	LPARAM lParam,UINT_PTR uIdSubclass,DWORD_PTR dwRefData)
{
	UNREFERENCED_PARAMETER(uIdSubclass);

	CFileContextMenuManager *pfcmm = reinterpret_cast<CFileContextMenuManager *>(dwRefData);

	return pfcmm->ShellMenuHookProc(hwnd,Msg,wParam,lParam);
}

LRESULT CALLBACK CFileContextMenuManager::ShellMenuHookProc(HWND hwnd,UINT uMsg,WPARAM wParam,
	LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_MEASUREITEM:
			/* wParam is 0 if this item was sent by a menu. */
			if(wParam == 0)
			{
				if(m_pShellContext3 != NULL)
					m_pShellContext3->HandleMenuMsg2(uMsg,wParam,lParam,NULL);
				else if(m_pShellContext2 != NULL)
					m_pShellContext2->HandleMenuMsg(uMsg,wParam,lParam);

				return TRUE;
			}
			break;

		case WM_DRAWITEM:
			if(wParam == 0)
			{
				if(m_pShellContext3 != NULL)
					m_pShellContext3->HandleMenuMsg2(uMsg,wParam,lParam,NULL);
				else if(m_pShellContext2 != NULL)
					m_pShellContext2->HandleMenuMsg(uMsg,wParam,lParam);
			}
			return TRUE;
			break;

		case WM_INITMENUPOPUP:
			{
				if(m_pShellContext3 != NULL)
					m_pShellContext3->HandleMenuMsg2(uMsg,wParam,lParam,NULL);
				else if(m_pShellContext2 != NULL)
					m_pShellContext2->HandleMenuMsg(uMsg,wParam,lParam);
			}
			break;

		case WM_MENUSELECT:
			{
				if(m_pStatusBar != NULL)
				{
					if(HIWORD(wParam) == 0xFFFF && lParam == 0)
					{
						m_pStatusBar->HandleStatusBarMenuClose();
					}
					else
					{
						m_pStatusBar->HandleStatusBarMenuOpen();

						int iCmd = static_cast<int>(LOWORD(wParam));

						if(!((HIWORD(wParam) & MF_POPUP) == MF_POPUP) &&
							(iCmd >= m_iMinID && iCmd <= m_iMaxID))
						{
							TCHAR szHelpString[512];

							/* Ask for the help string for the currently selected menu item. */
							HRESULT hr = m_pActualContext->GetCommandString(iCmd - m_iMinID,GCS_HELPTEXT,
								NULL,reinterpret_cast<LPSTR>(szHelpString),SIZEOF_ARRAY(szHelpString));

							/* If the help string was found, send it to the status bar. */
							if(hr == NOERROR)
							{
								m_pStatusBar->SetPartText(0,szHelpString);
							}
						}
					}

					/* Prevent the message from been passed onto the original window. */
					return 0;
				}
			}
			break;
	}

	return DefSubclassProc(hwnd,uMsg,wParam,lParam);
}