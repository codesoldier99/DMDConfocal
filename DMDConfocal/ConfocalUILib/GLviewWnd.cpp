#include "StdAfx.h"
#include "GLviewWnd.h"
#include "HGloableFunction.h"

CGLviewWnd::CGLviewWnd(void)
{
	m_bRotate=false;
	m_RotateX=0;
	m_RotateY=0;
	m_RotateZ=0;
	m_Scale=0.1;
	m_RotateXR=0;
	m_RotateYR=0;
	m_RotateZR = 0;
}


CGLviewWnd::~CGLviewWnd(void)
{

	if(wglGetCurrentContext()!=NULL)
		wglMakeCurrent(NULL,NULL);
	if(this->m_hGLContext!=NULL)
	{
		wglDeleteContext(this->m_hGLContext);
		this->m_hGLContext=NULL;
	}
}
BEGIN_MESSAGE_MAP(CGLviewWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
//	ON_WM_MOUSEWHEEL()
//	ON_WM_MOUSEHWHEEL()
ON_WM_KEYDOWN()
END_MESSAGE_MAP()


int CGLviewWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	HWND hWnd=this->GetSafeHwnd();
	m_hDC=::GetDC(hWnd);
	//HDC hDC=GetDC()->GetSafeHdc();


	if(this->SetWindowPixelFormat(m_hDC)==FALSE)
		return 0;
	if(this->CreatViewGLContext(m_hDC)==FALSE)
		return 0;

	//测试用
	//m_GlHeight.LoadFromBmp(HGetStartPath() + L"//EpiResult//ResultHeight.bmp");
	//if(m_GlHeight.LoadGLTextures(HGetStartPath() + L"//EpiResult//ResultPicture.bmp"))//载入贴图--贴图要先上才能行。。
	//	glEnable(GL_TEXTURE_2D);//这里启用贴图

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	FlushView();
	return 0;
}

void CGLviewWnd::FlushView()
{
	m_GlHeight.LoadFromBmp(HGetStartPath() + L"//EpiResult//ResultHeight.bmp");//测试用
	glDisable(GL_TEXTURE_2D);
	Invalidate();
	if(m_GlHeight.LoadGLTextures(HGetStartPath() + L".//EpiResult//ResultPicture.bmp"))//载入贴图--贴图要先上才能行。。
		glEnable(GL_TEXTURE_2D);//这里启用贴图
	Invalidate();
}

bool CGLviewWnd::InitPanel(LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	//初始化载物台
	if(m_ConfocalCore)
	{
		m_ConfocalCore->GetPlugin(PLUGIN_3DScanner)->Attach(this);
	}
	return true;
}

int CGLviewWnd::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if(ID==PANEL_VIEW_3DCMOPLETE)
	{
		//FlushView();
		OnRButtonUp(0,NULL);
	}
	return 0;
}


BOOL CGLviewWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();									// Reset The Matrix

	glClearColor(0.74,0.85,1,1.0f);

	// 			 Position	      View		Up Vector
	//gluLookAt(212, 60, 194,  0, 0, 0,  0, 1, 0);	// This Determines Where The Camera's Position And View Is
	gluLookAt(150, 60, 120,  0, 0, 0,  0, 1, 0);
	//glScalef(0.15, 0.15 * 1.5, 0.15);
	glScalef(m_Scale, m_Scale, m_Scale);

	//glRotatef(m_Pos,0,0,1);//沿x,y,z轴转
	glRotatef(m_RotateX+m_RotateXR,1,0,0);//x轴
	glRotatef(m_RotateY+m_RotateYR,0,1,0);//y轴
	m_GlHeight.DrawObject();//试试这个

	SwapBuffers(m_hDC);//这个相当重要，少则不输出，多则没关系

	//GradientFill()
	return TRUE;
	//return __super::OnEraseBkgnd(pDC);
}

BOOL CGLviewWnd::SetWindowPixelFormat( HDC hDC )
{
	PIXELFORMATDESCRIPTOR pixelDesc=
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|
		PFD_DOUBLEBUFFER,//|PFD_SUPPORT_GDI,//不注释这个后会变黑
		PFD_TYPE_RGBA,//rgba type
		24,//24-bit color depth
		0,0,0,0,0,0,//ignored color
		0,//no alpha
		0,//shift bit
		0,//no accumulation buffer
		0,0,0,0,//accum bits ignored
		32,//32-bit z-buffer
		0,//no stencil buffer
		0,//no auxiliary buffer
		PFD_MAIN_PLANE,//main layer
		0,//reserved
		0,0,0//layer masks ignored
	};

	this->m_GLPicexIndex=ChoosePixelFormat(hDC,&pixelDesc);
	if(this->m_GLPicexIndex==0)
	{
		this->m_GLPicexIndex=1;
		if(DescribePixelFormat(hDC,this->m_GLPicexIndex,sizeof(PIXELFORMATDESCRIPTOR),&pixelDesc)==0)
			return FALSE;
	}

	if(SetPixelFormat(hDC,this->m_GLPicexIndex,&pixelDesc)==FALSE)
		return FALSE;

	return TRUE;
}

BOOL CGLviewWnd::CreatViewGLContext( HDC hDC )
{
	this->m_hGLContext=wglCreateContext(hDC);//产生句柄
	if(this->m_hGLContext==NULL)
		return FALSE;//创建失败
	if(wglMakeCurrent(hDC,this->m_hGLContext)==FALSE)
		return FALSE;//rc选择失败
	return TRUE;
}

void CGLviewWnd::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	GLsizei width,height;
	GLdouble aspect;
	width=cx;
	height=cy;
	if(cy==0)
		aspect=(GLdouble)width;
	else
		aspect=(GLdouble)width/(GLdouble)height;

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window.  Farthest Distance Changed To 500.0f (NEW)
	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 500.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix

	// TODO: 在此处添加消息处理程序代码
}

void CGLviewWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	FlushView();
	//__super::OnRButtonUp(nFlags, point);
}


void CGLviewWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	m_bRotate=true;
	m_PointLast=point;
	m_PointEnd=point;
	__super::OnLButtonDown(nFlags, point);
}


void CGLviewWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	m_bRotate=false;

	m_RotateXR+=m_RotateX;
	m_RotateYR+=m_RotateY;
	m_RotateX=0;
	m_RotateY=0;
 	if(m_RotateXR>360.0)
 		m_RotateXR-=360.0;
 	if(m_RotateYR>360.0)
 		m_RotateYR-=360.0;
	if(m_RotateXR<360.0)
		m_RotateXR+=360.0;
	if(m_RotateYR<360.0)
		m_RotateYR+=360.0;
	Invalidate();
	__super::OnLButtonUp(nFlags, point);
}


void CGLviewWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if(m_bRotate)
	{
		m_PointEnd=point;
		CPoint t=m_PointEnd-m_PointLast;
		m_RotateX=t.y/10;

		m_RotateY=t.x/10;
	}
	Invalidate();

	__super::OnMouseMove(nFlags, point);
}


//BOOL CGLviewWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
//{
//	// TODO: 在此添加消息处理程序代码和/或调用默认值
//
//	m_Scale+=(float)zDelta/10.0;
//	Invalidate();
//	return __super::OnMouseWheel(nFlags, zDelta, pt);
//}


//void CGLviewWnd::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
//{
//	// 此功能要求 Windows Vista 或更高版本。
//	// _WIN32_WINNT 符号必须 >= 0x0600。
//	// TODO: 在此添加消息处理程序代码和/或调用默认值
//
//	__super::OnMouseHWheel(nFlags, zDelta, pt);
//}


void CGLviewWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	__super::OnKeyDown(nChar, nRepCnt, nFlags);
}


BOOL CGLviewWnd::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	if(WM_KEYFIRST<=pMsg->message&&pMsg->message<=WM_KEYLAST)
	{
		if(pMsg->wParam==VK_UP)
		{
			m_Scale+=0.01;
			Invalidate();
			return TRUE;
		}
		if(pMsg->wParam==VK_DOWN)
		{
			m_Scale-=0.01;
			Invalidate();
			return TRUE;
		}
	}

	return __super::PreTranslateMessage(pMsg);
}

