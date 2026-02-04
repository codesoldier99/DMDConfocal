// Pager.cpp : implementation file
//

#include "stdafx.h"
#include "Pager.h"


// CPager

IMPLEMENT_DYNAMIC(CPager, CWnd)
CPager::CPager()
{
}

CPager::~CPager()
{
	DestroyWindow();
}


BEGIN_MESSAGE_MAP(CPager, CWnd)
END_MESSAGE_MAP()



// CPager message handlers


BOOL CPager::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

BOOL CPager::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return Create(WC_PAGESCROLLER,NULL,dwStyle,rect,pParentWnd,nID,0);
}

void CPager::SetChild(CWnd *pWnd)
{
	Pager_SetChild(GetSafeHwnd(),pWnd->GetSafeHwnd());
}

void CPager::SetBorder(int iBorder)
{
	Pager_SetBorder(GetSafeHwnd(),iBorder);
}

void CPager::SetHorz(bool bHorz)
{
	DWORD style = GetStyle();
	style &= 1;
	if(bHorz && (style == PGS_VERT))
	{
		ModifyStyle(PGS_VERT,PGS_HORZ);
	}

	if((!bHorz) && (style == PGS_HORZ))
	{
		ModifyStyle(PGS_HORZ,PGS_VERT);
	}
}