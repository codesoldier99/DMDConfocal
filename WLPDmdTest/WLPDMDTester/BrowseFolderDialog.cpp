
#include "stdafx.h"
#include "BrowseFolderDialog.h"

static int CALLBACK _BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		SendMessage(hwnd,BFFM_SETSELECTION,TRUE,lpData);
	}
	return 0;
} 


PCTSTR CBrowseFolderDialog::BrowseFolder(HWND hwndOwner, PCTSTR pTitle,int nCSIDL,LPARAM lParam,
									   UINT uFlags,BFFCALLBACK callbackProc)
{
	OleInitialize(NULL);
	m_bi.hwndOwner = hwndOwner;
	if(callbackProc)
		m_bi.lpfn = callbackProc;
	else
		m_bi.lpfn = _BrowseCallbackProc;
	if(!lParam)
	{
		TCHAR szPath[MAX_PATH];
		LPITEMIDLIST pidlRoot = NULL;
		LPMALLOC pMalloc = NULL;
        SHGetMalloc(&pMalloc);
		SHGetFolderLocation(hwndOwner, nCSIDL, NULL, NULL, &pidlRoot);
		if(pidlRoot)
		{
			SHGetPathFromIDList(pidlRoot, szPath);
			m_bi.lParam = (LPARAM)szPath;	
			pMalloc->Free(pidlRoot);
		}		
		pMalloc->Release();
	}
	else
		m_bi.lParam = lParam;	
	m_bi.lpszTitle = pTitle;
	m_bi.ulFlags = uFlags;
	//m_bi.ulFlags = BIF_BROWSEINCLUDEFILES ;
	LPITEMIDLIST lpIDList = ::SHBrowseForFolder(&m_bi);
	::SHGetPathFromIDList(lpIDList, m_szFolderPath);
	HGLOBAL hgMem(0);
  if (lpIDList)
  {
    hgMem = GlobalHandle(lpIDList);
    GlobalFree(hgMem);
  }
	OleUninitialize();
	return m_szFolderPath;
}



