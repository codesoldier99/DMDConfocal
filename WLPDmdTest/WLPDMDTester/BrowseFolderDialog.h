/*-----------------------------------------------------------------
  * CBrowseFolderDialog - incapsulates work with "BrowseForFolder" dialog 
   ----------------------------------------------------------------*/
#pragma once

#include "shlobj.h"
class CBrowseFolderDialog
{
	BROWSEINFO m_bi;
	TCHAR m_szFolderPath[MAX_PATH];
public:
	CBrowseFolderDialog() : m_bi(), m_szFolderPath() {}
	PCTSTR BrowseFolder(HWND hwndOwner = NULL, 
						PCTSTR pTitle = NULL, 
						int nCSIDL = 0,
						LPARAM lParam = NULL,
						UINT uFlags = BIF_NEWDIALOGSTYLE, 
						BFFCALLBACK callbackProc = NULL
						);
};



