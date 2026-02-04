#include "stdafx.h"
#include "CPButton.h"

// CCPButton

IMPLEMENT_DYNAMIC(CCPButton, CButton)
CCPButton::CCPButton()
{
	m_bTracking = FALSE;
	m_bOver = false;
	w = GetSystemMetrics(SM_CXICON);
	h = GetSystemMetrics(SM_CYICON);
	m_bDown = false;
	m_bChecked = false;
	m_hGrayIcon = 0;
}

CCPButton::~CCPButton()
{
	if(m_hGrayIcon)
	{
		DestroyIcon(m_hGrayIcon);
	}			
}

BEGIN_MESSAGE_MAP(CCPButton, CButton)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// CCPButton 消息处理程序

//int CCPButton::Attach(IPtnObserver * pObserver)
//{
//	return m_sbj.Attach(pObserver);
//}
//
//int CCPButton::Detach(IPtnObserver * pObserver)
//{
//	return m_sbj.Detach(pObserver);
//}
//
//int CCPButton::Notify(IPtnSubject * pSubject,int ID,long wParam  ,void* pParam  )
//{
//	return 0;
//}
		
LRESULT CCPButton::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	if(message == WM_LBUTTONDOWN)
	{
		Notify(this,EVENT_BUTTONDOWN,0,0);
	}
	else if(message == WM_LBUTTONUP)
	{
		Notify(this,EVENT_BUTTONUP,0,0);
	}
	return __super::WindowProc(message, wParam, lParam);
}

void CCPButton::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	ModifyStyle(0,BS_OWNERDRAW);
	if(!m_tip.m_hWnd)
	{
		m_tip.Create(this,TTS_ALWAYSTIP);
		m_tip.Activate(TRUE);
		CString str;
		GetWindowText(str);
		m_tip.AddTool(this,str);
	}
	__super::PreSubclassWindow();
}

HICON CreateGrayIcon(HICON hIcon)
{
	HICON hGrayIcon = ::CopyIcon(hIcon);
	ICONINFO info;
	if(!::GetIconInfo(hGrayIcon,&info))return hGrayIcon;
	CDC dcMem,dcMemMask;
	dcMem.CreateCompatibleDC(NULL);
	CBitmap *pOldBitmap = dcMem.SelectObject(CBitmap::FromHandle(info.hbmColor));
	dcMemMask.CreateCompatibleDC(NULL);
	CBitmap *pOldMaskBitmap = (CBitmap*)dcMemMask.SelectObject(CBitmap::FromHandle(info.hbmMask));

	BITMAP bmp;
	GetObject(info.hbmColor,sizeof(BITMAP),&bmp);

	COLORREF clr,clrMask;
	BYTE gray;
	for(int i = 0; i < bmp.bmHeight; i++)
	{
		for(int j = 0; j < bmp.bmWidth; j++)
		{
			clr = dcMem.GetPixel(j,i);
			clrMask = dcMemMask.GetPixel(j,i);
			if(clrMask == 0)
			{
				gray = (GetRValue(clr) * 30 + GetGValue(clr) * 59 + GetBValue(clr) * 11) / 100;
				dcMem.SetPixel(j,i,RGB(gray,gray,gray));
			}
		}
	}
	dcMem.SelectObject(pOldBitmap);
	dcMemMask.SelectObject(pOldMaskBitmap);
	dcMem.DeleteDC();
	dcMemMask.DeleteDC();
	hGrayIcon = ::CreateIconIndirect(&info);
	return hGrayIcon;
}

void CCPButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ASSERT(lpDrawItemStruct);
	//AFX_MANAGE_STATE(::GetModuleHandle(L"ConfocalUILib.dll"));

	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rc;
	rc = lpDrawItemStruct->rcItem;

	UINT action = lpDrawItemStruct->itemAction;
			
	pDC->FillSolidRect(&rc,GetSysColor(COLOR_BTNFACE));

	int offset = 1;

	if(lpDrawItemStruct->itemState & ODS_DISABLED)
	{
		if(!m_hGrayIcon)
		{
			DestroyIcon(m_hGrayIcon);
			m_hGrayIcon = CreateGrayIcon(GetIcon());
		}
		pDC->DrawIcon((abs(rc.Width() - w) >> 1),(abs(rc.Height() - h) >> 1),m_hGrayIcon);
		return;
	}

	if(m_bChecked)
	{
		pDC->Draw3dRect(&rc,RGB(128,128,128),RGB(128,128,128));
	}
	else if(m_bOver)
	{
		pDC->Draw3dRect(&rc,RGB(0,0,0),RGB(0,0,0));
	}

	if(action & ODA_DRAWENTIRE)
	{
		pDC->DrawIcon((abs(rc.Width() - w) >> 1),(abs(rc.Height() - h) >> 1),GetIcon());
		//pDC->DrawIcon((abs(rc.Width() - w) >> 1),(abs(rc.Height() - h) >> 1),
		//	::LoadIcon(::GetModuleHandle(L"ConfocalUILib.dll"),MAKEINTRESOURCE(IDI_ICON_TOP)));
	}
	else if(action & ODA_SELECT)
	{
		if(m_bDown)
		{
			pDC->DrawIcon(offset + (abs(rc.Width() - w) >> 1),offset + (abs(rc.Height() - h) >> 1),GetIcon());
			//pDC->DrawIcon(offset + (abs(rc.Width() - w) >> 1),offset + (abs(rc.Height() - h) >> 1),
			//	::LoadIcon(::GetModuleHandle(L"ConfocalUILib.dll"),MAKEINTRESOURCE(IDI_ICON_TOP)));
		}
		else
		{
			pDC->DrawIcon((abs(rc.Width() - w) >> 1),(abs(rc.Height() - h) >> 1),GetIcon());
			//pDC->DrawIcon((abs(rc.Width() - w) >> 1),(abs(rc.Height() - h) >> 1),
			//	::LoadIcon(::GetModuleHandle(L"ConfocalUILib.dll"),MAKEINTRESOURCE(IDI_ICON_TOP)));
		}
	}
	else if(action & ODA_FOCUS)
	{
		pDC->DrawIcon((abs(rc.Width() - w) >> 1),(abs(rc.Height() - h) >> 1),GetIcon());
		//pDC->DrawIcon((abs(rc.Width() - w) >> 1),(abs(rc.Height() - h) >> 1),
		//	::LoadIcon(::GetModuleHandle(L"ConfocalUILib.dll"),MAKEINTRESOURCE(IDI_ICON_TOP)));
	}
}

void CCPButton::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	WPARAM lParam;
	lParam = MAKELONG(point.x,point.y);
	SendMessage(WM_LBUTTONDOWN,nFlags,lParam);

	__super::OnLButtonDblClk(nFlags, point);
}

void CCPButton::OnMouseMove(UINT nFlags, CPoint point)
{
	if(!m_bTracking)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = 1;
		m_bTracking = _TrackMouseEvent(&tme);
		m_bOver = true;
		Invalidate(FALSE);
	}
	__super::OnMouseMove(nFlags, point);
}

LRESULT CCPButton::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_bTracking = FALSE;
	m_bOver = false;
	m_bDown = false;
	Invalidate(FALSE);
	return 0;
}

void CCPButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(!m_bChecked)
		m_bDown = true;

	__super::OnLButtonDown(nFlags, point);
}

void CCPButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(!m_bChecked)
	{
		if(m_bDown)
		{
			Notify(this,EVENT_BUTTONCLICKED,0,0);
		}
		m_bDown = false;
	}

	__super::OnLButtonUp(nFlags, point);
}

void CCPButton::Check(bool bChecked)
{
	m_bChecked = bChecked;
}

bool CCPButton::IsChecked()
{
	return m_bChecked;
}
		
BOOL CCPButton::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message== WM_LBUTTONDOWN ||
		pMsg->message== WM_LBUTTONUP ||
		pMsg->message== WM_MOUSEMOVE)
		m_tip.RelayEvent(pMsg);

	return __super::PreTranslateMessage(pMsg);
}
