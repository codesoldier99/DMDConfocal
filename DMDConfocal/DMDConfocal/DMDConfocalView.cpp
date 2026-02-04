
// DMDConfocalView.cpp : CDMDConfocalView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "DMDConfocal.h"
#endif

#include "DMDConfocalDoc.h"
#include "DMDConfocalView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDMDConfocalView

IMPLEMENT_DYNCREATE(CDMDConfocalView, CView)

BEGIN_MESSAGE_MAP(CDMDConfocalView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CDMDConfocalView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// CDMDConfocalView 构造/析构

CDMDConfocalView::CDMDConfocalView()
{
	// TODO: 在此处添加构造代码
	m_Wnd=0;
	m_bools=false;
}

CDMDConfocalView::~CDMDConfocalView()
{
}

BOOL CDMDConfocalView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CDMDConfocalView 绘制

void CDMDConfocalView::OnDraw(CDC* /*pDC*/)
{
	CDMDConfocalDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// CDMDConfocalView 打印


void CDMDConfocalView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CDMDConfocalView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CDMDConfocalView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CDMDConfocalView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CDMDConfocalView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CDMDConfocalView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CDMDConfocalView 诊断

#ifdef _DEBUG
void CDMDConfocalView::AssertValid() const
{
	CView::AssertValid();
}

void CDMDConfocalView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDMDConfocalDoc* CDMDConfocalView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDMDConfocalDoc)));
	return (CDMDConfocalDoc*)m_pDocument;
}
#endif //_DEBUG

void CDMDConfocalView::SetShowWnd( CWnd* p_Wnd )
{
	//m_bools=true;

	if(!m_Wnd&&p_Wnd!=0)
	{
		m_Wnd=p_Wnd;//GetDocument()->GetShowWnd();
		CRect	rc;
		GetClientRect( &rc );
		//Sleep(10);
		//m_Wnd->Create(NULL,L"test", WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE, rc, this, 0 ) ;
		m_Wnd->SetParent(this);
		m_Wnd->ShowWindow(SW_SHOW);
	}
}


// CDMDConfocalView 消息处理程序


void CDMDConfocalView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if(IsWindow( m_Wnd->GetSafeHwnd() ) )//获取窗口句柄？
	{
		RECT rect;
		GetClientRect(&rect);
		m_Wnd->SetWindowPos(0,0,0,rect.right-rect.left+1,rect.bottom - rect.top +1,0);
	}
}


BOOL CDMDConfocalView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//鼠标滚动键控制载物台Z轴
	//if(m_Wnd->GetSafeHwnd())
	//	m_Wnd->SendMessage(WM_MOUSEWHEEL,zDelta);//发送指令，控制载物台Z轴数值
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


BOOL CDMDConfocalView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if(!m_Wnd) return TRUE;
	if(m_Wnd->GetSafeHwnd())
		m_Wnd->PreTranslateMessage(pMsg);
	return CView::PreTranslateMessage(pMsg);
}
