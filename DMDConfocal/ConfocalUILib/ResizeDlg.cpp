// ResizeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "ResizeDlg.h"
#include "afxdialogex.h"


// CResizeDlg 对话框

IMPLEMENT_DYNAMIC(CResizeDlg, CDialogEx)

CResizeDlg::CResizeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CResizeDlg::IDD, pParent)
{
	m_pChild = NULL;

	m_bHorz = false;
	m_bVert = false;
	m_bScroll = true;
	m_pChildPanel=0;
}

CResizeDlg::~CResizeDlg()
{
}

void CResizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CResizeDlg, CDialogEx)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


void CResizeDlg::SetChildWnd(HCorePanel* p_ChildPanel )
{
	m_pChildPanel=p_ChildPanel;	

	if(!m_pChildPanel)
		return;

	m_pChild=m_pChildPanel->GetCWnd();

	CRect p_childRect;
	m_pChild->GetWindowRect(&p_childRect);
	int cx=p_childRect.Width();
	int cy=p_childRect.Height();

	m_pChild->SetParent(this);

	m_pChild->SetWindowPos(this,0,0,cx,cy,
		SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);

	m_pChild->ShowWindow(SW_SHOW);
	CString name;
	m_pChild->GetWindowTextW(name);
	this->SetWindowTextW(name);//设置名字
	p_ChildPanel->Attach(this);//加上消息传递

}

int	CResizeDlg::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,
	void* mParam)
{
	Notify(this,ID,wParam,pParam,fParam,mParam);
	return 0;
}

void CResizeDlg::UpdateScrollbar()
{
	if(m_pChild)
	{
		if(m_bScroll)
		{
			RECT rect0;
			m_pChild->GetWindowRect(&rect0);
			RECT rect1;
			this->GetClientRect(&rect1);
			int dx0,dy0,dx1,dy1;
			dx0=rect0.right-rect0.left+1;
			dy0=rect0.bottom-rect0.top+1;
			dx1=rect1.right-rect1.left+1;
			dy1=rect1.bottom-rect1.top+1;	
			SCROLLINFO si;		
			si.fMask = SIF_ALL;				
			si.cbSize = sizeof(SCROLLINFO);		
			if(dx0>dx1)
			{				
				si.nMax=1000;
				si.nMin=1;
				si.nPos=GetScrollPos(SB_HORZ); 
				si.nPage = 1000*dx1/dx0;		
				this->SetScrollInfo(SB_HORZ,&si);
				this->ShowScrollBar(SB_HORZ,1);
				m_bHorz = true;
			}
			else
			{			
				this->ShowScrollBar(SB_HORZ,0);
				m_bHorz = false;
			}
			if(dy0>dy1)
			{	
				si.nMax=1000;
				si.nMin=1;
				si.nPos=GetScrollPos(SB_HORZ); 			
				si.nPage = 1000*dy1/dy0;
				this->SetScrollInfo(SB_VERT,&si);
				this->ShowScrollBar(SB_VERT,1);
				m_bVert = true;
			}
			else
			{
				this->ShowScrollBar(SB_VERT,0);
				m_bVert = false;
			}
		}
		else
		{
			m_bHorz = false;
			m_bVert = false;
			ShowScrollBar(SB_BOTH,0);
		}		
	}
}

void CResizeDlg::UpdateChildPos()
{
	if(m_pChild)
	{		
		if(m_bScroll)
		{
			int x=0,y=0;
			RECT rect0;
			m_pChild->GetWindowRect(&rect0);
			RECT rect1;
			this->GetClientRect(&rect1);
			int dx0,dy0,dx1,dy1;
			dx0=rect0.right-rect0.left+1;
			dy0=rect0.bottom-rect0.top+1;
			dx1=rect1.right-rect1.left+1;		
			dy1=rect1.bottom-rect1.top+1;	
			SCROLLINFO si;		

			if(dx1<dx0)
			{			
				GetScrollInfo(SB_HORZ,&si);
				x=-(dx0-dx1)*(int)si.nPos/(1000-(int)si.nPage);
			}
			else
			{
				x=(dx1-dx0)/2;            
			}		
			if(dy1<dy0)
			{
				GetScrollInfo(SB_VERT,&si);
				y=-(dy0-dy1)*(int)si.nPos/(1000-(int)si.nPage);
			}
			else
			{
				y=(dy1-dy0)/2;
			}

			m_pChild->SetWindowPos(0,x,y,0,0,SWP_NOSIZE);

			//Notify(this,SCROLLPANEL_POSCHANGED);
		}
		else
		{
			//没有滚动条
			int x=0,y=0;
			RECT rect0;
			m_pChild->GetWindowRect(&rect0);
			RECT rect1;
			this->GetClientRect(&rect1);
			int dx0,dy0,dx1,dy1;
			dx0=rect0.right-rect0.left+1;
			dy0=rect0.bottom-rect0.top+1;
			dx1=rect1.right-rect1.left+1;		
			dy1=rect1.bottom-rect1.top+1;	
			if(dx1<dx0)
			{			
				x = 0;
			}
			else
			{
				x = (dx1-dx0)/2;            
			}		
			if(dy1<dy0)
			{
				y = 0;
			}
			else
			{
				y = (dy1-dy0)/2;
			}
			m_pChild->SetWindowPos(0,x,y,0,0,SWP_NOSIZE);
			//Notify(this,SCROLLPANEL_POSCHANGED);
		}
	}
}



// CResizeDlg 消息处理程序


void CResizeDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(nSBCode==SB_THUMBPOSITION || nSBCode == SB_THUMBTRACK)
	{
		this->SetScrollPos(SB_VERT,nPos);        		
	}
	else if(nSBCode==SB_PAGELEFT)
	{
		this->SetScrollPos(SB_VERT,this->GetScrollPos(SB_VERT)-10);			       		
	}
	else if(nSBCode==SB_PAGERIGHT)
	{
		this->SetScrollPos(SB_VERT,this->GetScrollPos(SB_VERT)+10);			       		
	}
	else if(nSBCode==SB_LINELEFT)
	{
		this->SetScrollPos(SB_VERT,this->GetScrollPos(SB_VERT)-5);			       		
	}
	else if(nSBCode==SB_LINERIGHT)
	{
		this->SetScrollPos(SB_VERT,this->GetScrollPos(SB_VERT)+5);			       		
	}	
	UpdateChildPos();
	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CResizeDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(nSBCode==SB_THUMBPOSITION || nSBCode == SB_THUMBTRACK)
	{
		this->SetScrollPos(SB_HORZ,nPos);			       		
	}
	else if(nSBCode==SB_PAGELEFT)
	{
		this->SetScrollPos(SB_HORZ,this->GetScrollPos(SB_HORZ)-10);			       		
	}
	else if(nSBCode==SB_PAGERIGHT)
	{
		this->SetScrollPos(SB_HORZ,this->GetScrollPos(SB_HORZ)+10);			       		
	}
	else if(nSBCode==SB_LINELEFT)
	{
		this->SetScrollPos(SB_HORZ,this->GetScrollPos(SB_HORZ)-5);			       		
	}
	else if(nSBCode==SB_LINERIGHT)
	{
		this->SetScrollPos(SB_HORZ,this->GetScrollPos(SB_HORZ)+5);			       		
	}
	UpdateChildPos();
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CResizeDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	UpdateScrollbar();	
	UpdateChildPos();
}


HBRUSH CResizeDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	return afxGlobalData.brBarFace;
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}
