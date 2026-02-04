#include "StdAfx.h"
#include "WndBmp.h"
#include "Resource.h"
#include "HGloableFunction.h"

CRenderWnd::CRenderWnd(void)
{
	m_Buffer=0;
	ZoomHeight=1;
	ZoomWidth=1;
	m_hPos=0;
	m_vPos=0;
	m_bFitWndows=true;
	//初始化调色板
	m_bmi=(BITMAPINFO*)malloc(sizeof(BITMAPINFO)+256*4);
	memset(m_bmi,0,sizeof(BITMAPINFO)+256*4);
	m_bmi->bmiHeader.biPlanes=1;
	m_bmi->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	for(int i=0;i<256;i++)//调色板
	{
		m_bmi->bmiColors[i].rgbBlue=i;
		m_bmi->bmiColors[i].rgbGreen=i;
		m_bmi->bmiColors[i].rgbRed=i;
		m_bmi->bmiColors[i].rgbReserved=0;
	}//下次试你
	bDrawRec=false;
	//以下GDI
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL);
	fontFamily=new FontFamily(L"Arial");//L"幼圆"); 
	font=new Gdiplus::Font(fontFamily, 13, FontStyleRegular, UnitPixel);
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);  
	memcpy(lf.lfFaceName, "Arial", 5) ;  
	lf.lfHeight = 13 ;  
	brush=new SolidBrush(Color(255, 199, 237, 204));  
	stringformat.SetAlignment(StringAlignmentCenter);  
	stringformat.SetLineAlignment(StringAlignmentCenter);

	bDraw=false;
	m_vPosReal = 0;
}

bool CRenderWnd::InitPanel(LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	//初始化载物台
	if(m_ConfocalCore)
	{
		m_ConfocalCore->GetPlugin(PLUGIN_3DScanner)->Attach(this);
	}
	return true;
}

int CRenderWnd::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if(ID==PANEL_VIEW_3DCMOPLETE)
	{
		FlushView();
	}
	return 0;
}

void CRenderWnd::FlushView()
{
	int width=0;
	int heidth=0;
	int wbit=0;
	LPBYTE mLoadBuf=0;
	HLoadBmp(&mLoadBuf,width,heidth,wbit, HGetStartPath() + L"//EpiResult//ResultPicture.bmp");
	Render(mLoadBuf,width,heidth,wbit);
}

CRenderWnd::~CRenderWnd(void)
{
	if(m_Buffer)
	{	
		delete(m_Buffer);
		m_Buffer=0;
	}
	delete(m_bmi);
}

BOOL CRenderWnd::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: 在此添加专用代码和/或调用基类 
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

BEGIN_MESSAGE_MAP(CRenderWnd, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_D_FITWINDOW, &CRenderWnd::OnEpiFitwindows)
	ON_COMMAND(ID_D_SAVEFRAME, &CRenderWnd::OnEpiSaveframe)
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CREATE()
END_MESSAGE_MAP()


BOOL CRenderWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect	rc;
	GetClientRect( &rc );
	if (m_Buffer)
	{
		CDC MemDC;
		CBitmap MemBitmap;
		MemDC.CreateCompatibleDC(NULL);
		MemBitmap.CreateCompatibleBitmap(pDC,rc.Width(),rc.Height());
		MemDC.SelectObject(&MemBitmap);
		//CBitmap *pOldBit=MemDC.SelectObject(&MemBitmap);
		if(m_bFitWndows)//充满窗口用另一种方法 
		{
			DrawFunctionM2(&MemDC,rc);
			DrawROIS(&MemDC);
		}			
		else
			DrawFunction(&MemDC,rc);
		
		pDC->BitBlt(0,0,rc.Width(),rc.Height(),&MemDC,0,0,SRCCOPY);
		MemBitmap.DeleteObject();
		MemDC.DeleteDC();
	}
	return true;
	//return CWnd::OnEraseBkgnd(pDC);
}

void CRenderWnd::SetDrawable(bool bDrawV)
{
	bDraw=bDrawV;
}

void CRenderWnd::DrawFunction(CDC* pDC,CRect pRc)
{
	pDC->FillSolidRect(0,0,pRc.Width(),pRc.Height(),afxGlobalData.clrBarFace);
	if(m_Buffer)
	{
		iDestLeft = 0;
		iDestTop = 0;
		iDestW=pRc.Width();
		iDestH=pRc.Height();
		long	srcwidth = m_bmi->bmiHeader.biWidth;
		long	srcheight=m_bmi->bmiHeader.biHeight;
		if(srcwidth<iDestW)
		{
			iDestLeft=(iDestW-srcwidth)/2;
			iDestW=srcwidth;
		}
		else
			srcwidth=iDestW;
		if (srcheight<iDestH)
		{
			iDestTop=(iDestH-srcheight)/2;
			iDestH=srcheight;
		}
		else
			srcheight=iDestH;
		::StretchDIBits(pDC->GetSafeHdc(),iDestLeft, iDestTop,iDestW,iDestH,
			m_hPos,m_vPos,srcwidth, srcheight,m_Buffer,m_bmi,DIB_RGB_COLORS,SRCCOPY);
	}
}

void CRenderWnd::DrawFunctionM2(CDC* pDC,CRect pRc)
{
	pDC->FillSolidRect(0,0,pRc.Width(),pRc.Height(),afxGlobalData.clrBarFace);
	if(m_Buffer)
	{
		long	srcwidth = m_bmi->bmiHeader.biWidth;
		long	srcheight= m_bmi->bmiHeader.biHeight;
		iDestLeft = 0;
		iDestTop = 0;
		iDestW=pRc.Width();
		iDestH=pRc.Height();
		//算比例.....
		float scalex,scaley;
		scalex=(float)srcwidth/(float)iDestW;
		scaley=(float)srcheight/(float)iDestH;
		float AllScale=scalex>scaley?scalex:scaley;//谁更小，用谁的
		//输出大小 
		iDestH=srcheight/AllScale;
		iDestW=srcwidth/AllScale;		
		//算顶点坐标
		iDestLeft=(pRc.Width()-iDestW)/2;
		iDestTop=(pRc.Height()-iDestH)/2;
		::SetStretchBltMode(pDC->GetSafeHdc(),HALFTONE);
		::StretchDIBits(pDC->GetSafeHdc(),iDestLeft, iDestTop,iDestW,iDestH,0,0,
			srcwidth, srcheight,m_Buffer,m_bmi,DIB_RGB_COLORS,SRCCOPY);	
		m_ShowSize.x=iDestW;
		m_ShowSize.y=iDestH;
		m_RealSIze.x=srcwidth;
		m_RealSIze.y=srcheight;
		m_ShowPt.x=iDestLeft;
		m_ShowPt.y=iDestTop;
		ScaleValue=AllScale;
	}
}

void CRenderWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	update_scrollbar( m_bmi->bmiHeader.biWidth,m_bmi->bmiHeader.biHeight );
	UpdataView(m_nHOffset,m_nVOffset);
	// TODO: 在此处添加消息处理程序代码
}

void CRenderWnd::Render(LPBYTE pBuffer, int width,int heidth,int wbit )
{
	DWORD newSize=width*heidth*wbit/8;
	if(!m_Buffer)
		SetTimer(529,30,NULL);// 刷新进程
	if(newSize!=m_bmi->bmiHeader.biSizeImage)
	{
		m_bmi->bmiHeader.biWidth=width;
		m_bmi->bmiHeader.biHeight=heidth;
		m_bmi->bmiHeader.biBitCount=wbit;
		m_bmi->bmiHeader.biSizeImage=width*heidth*wbit/8;
		if(m_Buffer)
		{
			delete(m_Buffer);
			m_Buffer=0;
		}
		m_Buffer=(LPBYTE)malloc(m_bmi->bmiHeader.biSizeImage);
		update_scrollbar( m_bmi->bmiHeader.biWidth,m_bmi->bmiHeader.biHeight );
	}
	if(m_Buffer)
		memcpy(m_Buffer,pBuffer,m_bmi->bmiHeader.biSizeImage);
	m_bUpdata=true;
}

void CRenderWnd::UpdataView(int pHpos,int pVpos )
{
	if(pHpos<0)
		pHpos=0;
	if (pVpos<0)
		pVpos=0;
	CRect	rc;
	GetClientRect(&rc);
	long twidth=rc.right-rc.left;
	long theight=rc.bottom-rc.top;
	if(twidth>=m_bmi->bmiHeader.biWidth)
		m_hPos=0;
	else
		m_hPos=pHpos;

	if(theight>=m_bmi->bmiHeader.biHeight)
		m_vPos=0;
	else
	{
		m_vPosReal = pVpos;
		m_vPos = m_bmi->bmiHeader.biHeight - theight - pVpos;//要不要反向
	}
	Invalidate();
}

void CRenderWnd::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(nIDEvent==529)//刷新进程
	{
		//UpdataView(0,0);
		if(m_bUpdata)
		{
			Invalidate();
			m_bUpdata=false;
		}
	}
	//HRenderWnd::OnTimer(nIDEvent);
}

void CRenderWnd::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
{
	int width=pHeader->Vwidth;
	int heidth=pHeader->Vheight;
	int wbit=pHeader->VwBit;
	Render(pBuffer,width,heidth,wbit);
}

void CRenderWnd::update_scrollbar( int width,int height )
{
	if(m_bFitWndows)
	{
		width=0;
		height=0;
	}
	SCROLLINFO hInfo, vInfo;
	memset( &hInfo , 0, sizeof( hInfo ) );
	hInfo.cbSize = sizeof( hInfo );
	hInfo.fMask	= SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
	GetScrollInfo( SB_HORZ, &hInfo );
	memset( &vInfo, 0, sizeof( vInfo ) );
	vInfo.cbSize = sizeof( vInfo );
	vInfo.fMask	= SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
	GetScrollInfo( SB_VERT, &vInfo );
	CRect	rc;
	GetClientRect( &rc );
	if( width > 0 && height > 0 )
	{
		if( hInfo.nMin < 0 )
			hInfo.nMin = 0;
		hInfo.nMax = width;
		hInfo.nPage = rc.Width();
		if( (int)( hInfo.nPos + hInfo.nPage - hInfo.nMax ) > 0 )
		{
			hInfo.nPos = hInfo.nMax -  hInfo.nPage;
			m_nHOffset = hInfo.nPos;//这里暂时不记录
		}
		if( width == rc.Width() )
			hInfo.nMax = 0;
		if( vInfo.nMin < 0 )
			vInfo.nMin = 0;
		vInfo.nMax = height;
		vInfo.nPage = rc.Height();
		if( (int)( vInfo.nPos + vInfo.nPage - vInfo.nMax ) > 0 )
		{
			vInfo.nPos = vInfo.nMax -  vInfo.nPage;
			m_nVOffset = vInfo.nPos;//这里暂时不记录
		}
		if( height == rc.Height() )
			vInfo.nMax = 0;	
	}
	else
	{
		hInfo.nMin = 0;
		hInfo.nMax = 0;
		hInfo.nPage = rc.Width();
		vInfo.nMin = 0;
		vInfo.nMax = 0;
		vInfo.nPage = rc.Height();
	}
	hInfo.nPos = m_hPos;
	vInfo.nPos = m_vPosReal;
	SetScrollInfo( SB_HORZ, &hInfo );
	SetScrollInfo( SB_VERT, &vInfo );
}

void CRenderWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	SCROLLINFO info;
	memset( &info, 0, sizeof( info ) );
	info.cbSize = sizeof( info );
	info.fMask	= SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
	GetScrollInfo( SB_HORZ, &info );
	long dx = 0;
	long	nStep=1;
	switch( nSBCode )
	{
		case SB_LEFT:			dx = -1;			break;
		case SB_LINELEFT:		dx = -10 * nStep;		break;
		case SB_PAGELEFT:		dx = -(int)info.nPage;	break;
		case SB_PAGERIGHT:		dx = (int)info.nPage;	break;
		case SB_LINERIGHT:		dx = 10 * nStep;		break;
		case SB_RIGHT:			dx = nStep;				break;
		case SB_ENDSCROLL:		dx = 0;					break;
		case SB_THUMBTRACK:		dx = nPos - info.nPos;	break;
		case SB_THUMBPOSITION:		
			if( long( nPos - info.nPos ) > 10 * nStep )
				dx = 10 * nStep;
			else
				if( long( nPos - info.nPos ) < -10 * nStep )
					dx = -10 * nStep;
				else
					dx = nPos - info.nPos;
		break;
	}
	info.nPos += dx;
	if( info.nPos > (int)(info.nMax - info.nPage) )
		info.nPos = info.nMax - info.nPage;

	if( info.nPos < info.nMin )
		info.nPos = info.nMin;

	if( m_nHOffset != info.nPos )
	{
		m_nHOffset = info.nPos;
		SetScrollPos( SB_HORZ, info.nPos );
		UpdataView(m_nHOffset,m_nVOffset);
	}
	//HRenderWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CRenderWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	SCROLLINFO info;
	memset( &info, 0, sizeof( info ) );
	info.cbSize = sizeof( info );
	info.fMask	= SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
	GetScrollInfo( SB_VERT, &info );
	long dy = 0;
	double	fZoomH, fZoomV;
	//m_wndBitmap->get_zoom( fZoomH, fZoomV );
	long	nStep =1;// (long)fZoomV;
	if( nStep < 1 )	nStep = 1;
	switch( nSBCode )
	{
		case SB_BOTTOM:			dy = nStep;				break;
		case SB_LINEDOWN:		dy = 10 * nStep;		break;
		case SB_PAGEDOWN:		dy = (int)info.nPage;	break;
		case SB_PAGEUP:			dy = -(int)info.nPage;	break;
		case SB_LINEUP:			dy = -10 * nStep;		break;
		case SB_TOP:			dy = -nStep;			break;
		case SB_ENDSCROLL:		dy = 0;break;
		case SB_THUMBTRACK	:	dy = nPos - info.nPos;	break;
		case SB_THUMBPOSITION:		
			if( long( nPos - info.nPos ) > 10 * nStep )
				dy = 10 * nStep;
			if( long( nPos - info.nPos ) < -10 * nStep )
				dy = -10 * nStep;
			else
				dy = nPos - info.nPos;
		break;
	}
	info.nPos += dy;
	if( info.nPos > (int)(info.nMax - info.nPage) )
		info.nPos = info.nMax - info.nPage;
	if( info.nPos < info.nMin )
		info.nPos = info.nMin;
	if( m_nVOffset != info.nPos )
	{
		m_nVOffset = info.nPos;
		SetScrollPos( SB_VERT, info.nPos );
		//update_bitmap();
		//update_view();
		UpdataView(m_nHOffset,m_nVOffset);
	}
	//HRenderWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CRenderWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//下面是菜单 
	ClientToScreen(&point);
	CMenu tmenu;
	tmenu.LoadMenuW(IDR_MENU1);//加入菜单
	CMenu* pPopMenu=tmenu.GetSubMenu(0);
	pPopMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,point.x,point.y,this);//显示菜单，左边，跟踪右键
	pPopMenu->Detach();
	tmenu.DestroyMenu();
	__super::OnRButtonUp(nFlags, point);
}

void CRenderWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	__super::OnRButtonDown(nFlags, point);
}

void CRenderWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_bFitWndows&&bDraw)
	{
		m_DrawROIPt=point;
		CorrectPt(&m_DrawROIPt);
		bDrawRec=true;
	}
	__super::OnLButtonDown(nFlags, point);
}


void CRenderWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	bDrawRec=false;
	__super::OnLButtonUp(nFlags, point);
}


void CRenderWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (bDrawRec)
	{
		m_DrawROIEndPt=point;
		CorrectPt(&m_DrawROIEndPt);
		m_DrawROISize.x=abs(m_DrawROIPt.x-m_DrawROIEndPt.x);
		m_DrawROISize.y=abs(m_DrawROIPt.y-m_DrawROIEndPt.y);
		Invalidate();
	}
	__super::OnMouseMove(nFlags, point);
}

void CRenderWnd::OnEpiFitwindows()
{
	// TODO: 在此添加命令处理程序代码
	m_bFitWndows=!m_bFitWndows;
	SendMessage(WM_SIZE);
	//update_scrollbar(0,0);
	Invalidate();
}

void CRenderWnd::OnEpiSaveframe()
{
	// TODO: 在此添加命令处理程序代码
	CString ImgName;
	SYSTEMTIME st;
	GetLocalTime(&st);
	CFileDialog fileDlg(false);
	fileDlg.m_ofn.lpstrTitle=_T("SaveImg");
	fileDlg.m_ofn.lpstrFilter=_T("Bmp Files(*.bmp)\0*.bmp\0All Files(*.*)\0*.*\0\0");
	fileDlg.m_ofn.lpstrDefExt=_T("bmp");
	ImgName.Format(_T("%02d-%02d_%02d%02d%02d.bmp"),st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	fileDlg.m_ofn.lpstrFile = ImgName.GetBuffer(200);  
	fileDlg.m_ofn.nMaxFile = 200;
	if (IDOK==fileDlg.DoModal())
	{
		HSaveBmp((LPBYTE)m_Buffer,m_bmi->bmiHeader.biWidth,
			m_bmi->bmiHeader.biHeight,m_bmi->bmiHeader.biBitCount,fileDlg.GetPathName().GetBuffer());
	}
}

void CRenderWnd::DrawROIS( CDC* pDC)
{
	if(m_Buffer)
	{
		Graphics g(pDC->m_hDC);
		if (m_DrawROISize.x<=10||m_DrawROISize.y<=10)//如果太小就选全局
		{
			mROIRealRec.left=mROIRealRec.right=mROIRealRec.top
				=mROIRealRec.bottom=0;
			CString text;
			text.Format(_T("All Size"));
			g.DrawString(text.GetBuffer(),text.GetLength(),font,PointF(70,60),&stringformat,brush);
			return;
		}	
		int mX=m_DrawROIEndPt.x>m_DrawROIPt.x?m_DrawROIPt.x:m_DrawROIEndPt.x;
		int mY=m_DrawROIEndPt.y>m_DrawROIPt.y?m_DrawROIPt.y:m_DrawROIEndPt.y;
		//if (mX+m_DrawROISize.x>m_ShowSize.x)
		//	m_DrawROISize.x=m_ShowSize.x-mX;
		//if (mY+m_DrawROISize.y>m_ShowSize.y)
		//	m_DrawROISize.y=m_ShowSize.y-mY;
		if (bDraw)
		{
			Pen ROIPen(Color::Red,1);
			ROIPen.SetDashStyle(DashStyleDash);
			g.DrawRectangle(&ROIPen,mX,mY,m_DrawROISize.x,m_DrawROISize.y);
		}
		else
		{
			Pen ROIPen(Color::White,1);
			ROIPen.SetDashStyle(DashStyleDash);
			g.DrawRectangle(&ROIPen,mX,mY,m_DrawROISize.x,m_DrawROISize.y);
		}
		mX-=m_ShowPt.x;
		mY-=m_ShowPt.y;
		mROIRealRec.left=mX*ScaleValue;
		mROIRealRec.top=mY*ScaleValue;
		mROIRealRec.right=(mX+m_DrawROISize.x)*ScaleValue;
		mROIRealRec.bottom=(mY+m_DrawROISize.y)*ScaleValue;
		if (mROIRealRec.right>m_RealSIze.x)
			mROIRealRec.right=m_RealSIze.x;
		if (mROIRealRec.bottom>m_RealSIze.y)
			mROIRealRec.bottom=m_RealSIze.y;
		CString text;
		text.Format(_T("(%d,%d)、(%d,%d)\r\nSize(%d,%d)"),mROIRealRec.left,mROIRealRec.top,
			mROIRealRec.right,mROIRealRec.bottom,mROIRealRec.Width(),mROIRealRec.Height());
		g.DrawString(text.GetBuffer(),text.GetLength(),font,PointF(70,60),&stringformat,brush);
	}
}



int CRenderWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO:  在此添加您专用的创建代码
	FlushView();
	return 0;
}

void CRenderWnd::CorrectPt(CPoint* mPT)
{
	CPoint tRect=m_ShowPt+m_ShowSize;//下边界
	if(mPT->x<=0)
		mPT->x=0;
	if (mPT->y<=0)
		mPT->y=0;
	if(mPT->x<m_ShowPt.x)
		mPT->x=m_ShowPt.x;//不超出
	if(mPT->y<m_ShowPt.y)
		mPT->y=m_ShowPt.y;
	if (mPT->x>tRect.x)
		mPT->x=tRect.x;
	if (mPT->y>tRect.y)
		mPT->y=tRect.y;	
}
