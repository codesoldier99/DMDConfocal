#include "StdAfx.h"
#include "SplashWnd.h"


CSplashWnd::CSplashWnd(void)
{
	m_BkImg=0;
	m_width=0;
	m_height=0;
	m_bInited=false;

	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	//gdi+
	fontFamily=new FontFamily(L"楷体");//L"幼圆"); 
	font=new Gdiplus::Font(fontFamily, 15, FontStyleRegular, UnitPixel);
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);  
	memcpy(lf.lfFaceName, "楷体", 2) ;  
	lf.lfHeight = 13 ;  
	brush=new SolidBrush(Color(255, 255, 255, 255));  
	stringformat.SetAlignment(StringAlignmentNear);  
	stringformat.SetLineAlignment(StringAlignmentCenter);
}


CSplashWnd::~CSplashWnd(void)
{
	//删除背景
	if(m_BkImg)
		delete(m_BkImg);
	//删资源
	m_Message.clear();
	m_bInited=false;
	GdiplusShutdown(m_gdiplusToken);
}

BEGIN_MESSAGE_MAP(CSplashWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP() 


int CSplashWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO:  在此添加您专用的创建代码
	//gdi+
	m_Blend.BlendOp=0; 
	m_Blend.BlendFlags=0; 
	m_Blend.AlphaFormat=1; 
	m_Blend.SourceConstantAlpha=255;// 透明度
	//加载图像
	if(m_BkImg)
	{
		m_width=m_BkImg->GetWidth();
		m_height=m_BkImg->GetHeight();
		this->SetWindowPos(NULL,0,0,m_width,m_height,SWP_NOZORDER);	
	}
	// Center the window.
	CenterWindow();
	// Set a timer to destroy the splash screen.
	SetTimer(529, 100, NULL);
	return 0;
}


void CSplashWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CWnd::OnPaint()

	if(m_BkImg)
	{
		//Graphics graphics(dc.m_hDC); // Create a GDI+ graphics object
		//Image image(_T("1.png")); // Construct an image
		//graphics.DrawImage(m_BkImg, 0, 0, m_BkImg->GetWidth(), m_BkImg->GetHeight());
		//int width=m_BkImg->GetWidth();
		//int height= m_BkImg->GetHeight();
		HDC hdcTemp = GetDC()->m_hDC;
		HDC m_hdcMemory = CreateCompatibleDC(hdcTemp);
		HBITMAP hBitMap = CreateCompatibleBitmap(hdcTemp,m_width ,m_height);
		SelectObject(m_hdcMemory, hBitMap);
		// m_Blend.SourceConstantAlpha=100; // 窗口透明度最大为255，最小为0
		HDC hdcScreen = ::GetDC(m_hWnd);
		RECT rct;
		GetWindowRect(&rct);
		POINT ptWinPos = {rct.left, rct.top};
		Graphics graph(m_hdcMemory);	
		DrawObject(graph);//下面绘图
		SIZE sizeWindow={m_width,m_height};
		POINT ptSrc={0,0};
		DWORD dwExStyle=GetWindowLong(m_hWnd,GWL_EXSTYLE);
		if((dwExStyle&0x80000)!=0x80000)
			SetWindowLong(m_hWnd,GWL_EXSTYLE,dwExStyle^0x80000);
		::UpdateLayeredWindow(m_hWnd, hdcScreen, &ptWinPos, &sizeWindow, m_hdcMemory, &ptSrc, 0, &m_Blend, 2);
		graph.ReleaseHDC(m_hdcMemory);
		::ReleaseDC(m_hWnd,hdcScreen);
		hdcScreen=NULL;
		::ReleaseDC(m_hWnd,hdcTemp);
		hdcTemp=NULL;
		DeleteObject(hBitMap);
		DeleteDC(m_hdcMemory);
		m_hdcMemory=NULL;
	}	
}

void CSplashWnd::DrawObject( Graphics &p_graph )
{
	//Step1: 绘背景
	p_graph.DrawImage(m_BkImg,0,0,m_width,m_height);
	//Step2:绘文字
	//最大显示5条消息
	int size=m_Message.size();//
	int pos=m_height-20;//底扣掉20像素`
	for (int i=0;i<5;i++)
	{
		if(--size<0)
			break;
		CString text=m_Message[size];
		p_graph.DrawString(text.GetBuffer(),text.GetLength(),font,PointF(50,pos),&stringformat,brush);	
		pos-=20;//行距
	}
}

void CSplashWnd::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if(nIDEvent==529)
	{
		if(m_bInited)
			this->Invalidate();
	}
	CWnd::OnTimer(nIDEvent);
}


void CSplashWnd::PostNcDestroy()
{
	// TODO: 在此添加专用代码和/或调用基类

	//删资源
	m_Message.clear();
	m_bInited=false;

	CWnd::PostNcDestroy();
}

void CSplashWnd::Show(CString p_Res)
{
	if(!m_bInited)
	{
		//加载资源
		if(!m_BkImg)
			m_BkImg = Image::FromFile(p_Res);//L"F:\\abc.png");
		//if(m_BkImg->)
		//WS_EX_TOPMOST|WS_EX_TOOLWINDOW
		m_bInited=CreateEx(0,AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
			NULL, WS_POPUP | WS_VISIBLE, 0, 0, 100, 100, NULL, NULL);
		if(m_bInited)
			this->Invalidate();
		//this->ShowWindow(SW_SHOW);
	}
	else
		this->ShowWindow(SW_SHOW);

}
void CSplashWnd::Print( CString p_msg )
{
	m_Message.push_back(p_msg);
	if(m_Message.size()>6)//删除多余的
		m_Message.erase(m_Message.begin()+1);
	if(m_bInited)
		this->Invalidate();
}

void CSplashWnd::DestroyWnd( void )
{
	if(m_bInited)
		this->PostMessage(WM_CLOSE, 0L, 0L);
}

