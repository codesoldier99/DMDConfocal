// PagerWnd.cpp : implementation file
//

#include "stdafx.h"
#include "PagerWnd.h"
#include ".\pagerwnd.h"

const int PAGER_ID = 1000;

// PagerWnd

IMPLEMENT_DYNAMIC(PagerWnd, CWnd)
PagerWnd::PagerWnd()
{
	m_pChild = 0;
	m_sz.SetSize(100,100);
}

PagerWnd::~PagerWnd()
{
	DestroyWindow();
}


BEGIN_MESSAGE_MAP(PagerWnd, CWnd)
	ON_NOTIFY(PGN_CALCSIZE,PAGER_ID,OnPagerCalcSize)
	ON_WM_SIZE()
END_MESSAGE_MAP()

// PagerWnd message handlers

BOOL PagerWnd::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	BOOL res = CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	m_pager.Create(WS_CHILD | WS_VISIBLE | PGS_VERT,rect,this,PAGER_ID);

	return res;
}

void PagerWnd::SetChild(CWnd *pChild)
{
	ASSERT(this);

	//不要重复设置
	if(m_pChild == pChild)return;

	m_pChild = pChild;

	if(!m_pChild)return;

	CRect r;
	m_pChild->GetWindowRect(&r);
	m_sz = r.Size();
	m_pager.SetChild(m_pChild);

	CWnd *pWnd = m_pChild->GetParent();
	if(pWnd)
	{
		::SetParent(GetSafeHwnd(),pWnd->GetSafeHwnd());
		::SetParent(m_pChild->GetSafeHwnd(),m_pager.GetSafeHwnd());
	}
	pWnd = m_pager.GetParent();
}

void PagerWnd::OnPagerCalcSize(NMHDR *pHdr,LRESULT *pResult)
{
	NMPGCALCSIZE *pNMSize = (NMPGCALCSIZE*)pHdr;
	switch(pNMSize->dwFlag)
	{
	case PGF_CALCWIDTH:
		pNMSize->iWidth = m_sz.cx;
		break;
	case PGF_CALCHEIGHT:
		pNMSize->iHeight = m_sz.cy;
		break;
	}
	*pResult = 0;
}
void PagerWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if(m_pager.GetSafeHwnd())
	{
		m_pager.SetHorz(cx * m_sz.cy < cy * m_sz.cx);
		m_pager.MoveWindow(0,0,cx,cy);
	}
}

void PagerWnd::SetScrollSize(int cx,int cy)
{
	m_sz.cx = cx;
	m_sz.cy = cy;
	//触发OnPagerCalcSize事件
	m_pager.SetChild(m_pChild);
}