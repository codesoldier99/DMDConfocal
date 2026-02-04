#include "StdAfx.h"
#include "DockDlg.h"


CDockDlg::CDockDlg(void)
{
	m_Wnd=0;
	m_WndNext=0;
}


CDockDlg::~CDockDlg(void)
{


}

void CDockDlg::LockPane( bool p_lock )
{
	if(IsTabbed())
	{
		GetParentTabbedPane()->EnableGripper(FALSE);                     
		GetParentTabbedPane()->EnableDocking(0);
	}
	//GetParentTabbedPane()->EnableGripper(p_lock);                     
	//GetParentTabbedPane()->EnableDocking(p_lock);
	EnableDocking(p_lock);//是否可以停靠
	EnableGripper(p_lock);//显示标题栏与否
}
BEGIN_MESSAGE_MAP(CDockDlg, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


int CDockDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码

	return 0;
}

void CDockDlg::SetWnd( CWnd* pWnd )
{
	m_Wnd=pWnd;
	if(!m_Wnd) return;
	m_Wnd->SetParent(this);
	m_Wnd->ShowWindow(SW_SHOW);
	CString texts;
	m_Wnd->GetWindowTextW(texts);
	SetWindowTextW(texts);
}


void CDockDlg::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	
	// TODO: 在此处添加消息处理程序代码
	if(m_Wnd)
		m_Wnd->SetWindowPos(this,0,0,cx,cy,SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
	iW=cx;
	iH=cy;
}

void CDockDlg::NewWnd( void )
{
	if(!m_WndNext)
		m_WndNext=new CDockDlg();
}


LRESULT CDockDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (message==1993)
	{
		int iX=wParam,iY=lParam;
		m_Wnd->MoveWindow(iX,iY,iW,iH);
		//if(m_Wnd)
		//	m_Wnd->SetWindowPos(this,iX,iY,iW,iH,SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
		Invalidate();
	}
	return CDockablePane::WindowProc(message, wParam, lParam);
}
