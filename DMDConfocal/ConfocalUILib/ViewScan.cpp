#include "StdAfx.h"
#include "ViewScan.h"
#include "HGloableFunction.h"


ViewScan::ViewScan(void)
{
	bFlush=false;

	hFlushHeader.Vbuffer=0;
	hFlushHeader.Vsize=0;
	iCur=0;
	iTotal=0;

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
	}
	//以下gdi
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL);
	fontFamily=new FontFamily(L"Arial");//L"幼圆"); 
	font=new Gdiplus::Font(fontFamily, 20, FontStyleRegular, UnitPixel);
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);  
	memcpy(lf.lfFaceName, "Arial", 5) ;  
	lf.lfHeight = 13 ;  
	//brush=new SolidBrush(Color(255, 255, 255, 0));
	brush=new SolidBrush(Color(255,0x00,181,4)); 
	stringformat.SetAlignment(StringAlignmentCenter);  
	stringformat.SetLineAlignment(StringAlignmentCenter);
}

ViewScan::~ViewScan(void)
{
}

bool ViewScan::InitPanel (LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	m_ReBuildPro=m_ConfocalCore->GetCoreProcess(PROCESS_3DReBuild);
	if (m_ReBuildPro)
		m_ReBuildPro->Attach(this);
	return true;
}

int ViewScan::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	//得到消息，进行复制图像，然后刷新
	if (ID==NOTIFY_3DSCAN_FLUSH)
	{
		iCur=wParam;

		if (m_ReBuildPro)
		{
			if (m_ReBuildPro->ProcessImg(&iCur,&iTotal,&hFlushHeader))
			{
				m_bmi->bmiHeader.biWidth=hFlushHeader.Vwidth;
				m_bmi->bmiHeader.biHeight=hFlushHeader.Vheight;
				m_bmi->bmiHeader.biBitCount=hFlushHeader.VwBit;
				m_bmi->bmiHeader.biSizeImage=hFlushHeader.Vsize;
				Invalidate();
			}		
		}
	}
	return 0;
}

void ViewScan::DoubleDCFun( CDC* pDC,CRect pRc )
{
	//准备工作
	CDC MemDC;
	MemDC.CreateCompatibleDC(NULL);
	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(pDC,pRc.Width(),pRc.Height());
	MemDC.SelectObject(&MemBitmap);

	//准备结束，绘制图像数据...
	if(m_bFitWndows)//充满窗口用另一种方法 
		DrawFunctionM2(&MemDC,pRc);
	else
		DrawFunction(&MemDC,pRc);
	DrawCurImg(&MemDC,pRc);
	pDC->BitBlt(0,0,pRc.Width(),pRc.Height(),&MemDC,0,0,SRCCOPY);
	MemBitmap.DeleteObject();
	MemDC.DeleteDC();
}

void ViewScan::DrawFunctionM2(CDC* pDC,CRect pRc)
{
	pDC->FillSolidRect(0,0,pRc.Width(),pRc.Height(),afxGlobalData.clrBarFace);
	if(hFlushHeader.Vbuffer)
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
		ZoomAmp=AllScale;
		//输出大小 
		iDestH=srcheight/AllScale;
		iDestW=srcwidth/AllScale;		
		//算顶点坐标
		iDestLeft=(pRc.Width()-iDestW)/2;
		iDestTop=(pRc.Height()-iDestH)/2;
		::SetStretchBltMode(pDC->GetSafeHdc(),HALFTONE);
		::StretchDIBits(
			pDC->GetSafeHdc(),
			iDestLeft, iDestTop,iDestW,iDestH,0,0,
			srcwidth, srcheight,hFlushHeader.Vbuffer,m_bmi,DIB_RGB_COLORS,SRCCOPY);
	}
}

void ViewScan::DrawFunction(CDC* pDC,CRect pRc)
{
	ZoomAmp=1.0f;
	pDC->FillSolidRect(0,0,pRc.Width(),pRc.Height(),afxGlobalData.clrBarFace);
	if(hFlushHeader.Vbuffer)
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
		::StretchDIBits(
			pDC->GetSafeHdc(),
			iDestLeft, iDestTop,iDestW,iDestH,
			m_hPos,m_vPos,srcwidth, srcheight,hFlushHeader.Vbuffer,
			m_bmi,DIB_RGB_COLORS,SRCCOPY);
	}
}

void ViewScan::DrawCurImg(CDC* pDC,CRect pRc)
{
	Graphics g(pDC->m_hDC);
	CString text;
	float mV=1.0f/ZoomAmp * 100;
	text.Format(_T("%d / %d (Zoom:%.2f"),iCur+1,iTotal,mV);
	text+=L"%)";
	g.DrawString(text.GetBuffer(),text.GetLength(),font,PointF(140,50),&stringformat,brush);
}
BEGIN_MESSAGE_MAP(ViewScan, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_VIEWLAYER_FITWINDOW, &ViewScan::OnEpiFitwindows)
	ON_COMMAND(ID_VIEWLAYER_SAVEFRAME, &ViewScan::OnEpiSaveframe)
	ON_COMMAND(ID_VIEWLAYER_RELEASEALLIMG, &ViewScan::OnEpiReleaseImg)
END_MESSAGE_MAP()


BOOL ViewScan::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	bFlush=true;
	CRect rc;
	GetClientRect(&rc);
	DoubleDCFun(pDC,rc);//用双缓冲的方法
	bFlush=false;
	return __super::OnEraseBkgnd(pDC);
}

BOOL ViewScan::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	TRACE(L"MouseWheel%d\n",zDelta);
	if (bFlush)
		return false;

	if (zDelta>=0)
	{
		iCur++;
		if (m_ReBuildPro->ProcessImg(&iCur,&iTotal,&hFlushHeader))
		{
			m_bmi->bmiHeader.biWidth=hFlushHeader.Vwidth;
			m_bmi->bmiHeader.biHeight=hFlushHeader.Vheight;
			m_bmi->bmiHeader.biBitCount=hFlushHeader.VwBit;
			m_bmi->bmiHeader.biSizeImage=hFlushHeader.Vsize;
			PostMessage(529+1);					
		}
	}
	else if (zDelta<0)
	{
		iCur--;
		if (m_ReBuildPro->ProcessImg(&iCur,&iTotal,&hFlushHeader))
		{
			m_bmi->bmiHeader.biWidth=hFlushHeader.Vwidth;
			m_bmi->bmiHeader.biHeight=hFlushHeader.Vheight;
			m_bmi->bmiHeader.biBitCount=hFlushHeader.VwBit;
			m_bmi->bmiHeader.biSizeImage=hFlushHeader.Vsize;
			PostMessage(529+1);
		}
	}
	return TRUE;
	//return __super::OnMouseWheel(nFlags, zDelta, pt);
}

LRESULT ViewScan::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	if(message == 529+1)
	{
		Invalidate();
	}
	return __super::WindowProc(message, wParam, lParam);
}

void ViewScan::UpdataView(int pHpos,int pVpos )
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
		m_vPos=m_bmi->bmiHeader.biHeight-theight-pVpos;//要不要反向
	Invalidate();
}

void ViewScan::update_scrollbar( int width,int height )
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
	SetScrollInfo( SB_HORZ, &hInfo );
	SetScrollInfo( SB_VERT, &vInfo );
}

void ViewScan::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);
	// TODO: 在此处添加消息处理程序代码
	update_scrollbar( m_bmi->bmiHeader.biWidth,m_bmi->bmiHeader.biHeight );
	UpdataView(m_nHOffset,m_nVOffset);
}

void ViewScan::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	SCROLLINFO info;
	memset( &info, 0, sizeof( info ) );
	info.cbSize = sizeof( info );
	info.fMask	= SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
	GetScrollInfo( SB_HORZ, &info );
	long dx = 0;
	long nStep=1;
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
	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void ViewScan::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	SCROLLINFO info;
	memset( &info, 0, sizeof( info ) );
	info.cbSize = sizeof( info );
	info.fMask	= SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
	GetScrollInfo( SB_VERT, &info );
	long dy = 0;
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
		UpdataView(m_nHOffset,m_nVOffset);
	}
	__super::OnVScroll(nSBCode, nPos, pScrollBar);
}

void ViewScan::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//下面是菜单 
	ClientToScreen(&point);
	CMenu tmenu;
	tmenu.LoadMenuW(IDR_ViewMutiLayer);//加入菜单
	CMenu* pPopMenu=tmenu.GetSubMenu(0);
	pPopMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,point.x,point.y,this);//显示菜单，左边，跟踪右键
	pPopMenu->Detach();
	tmenu.DestroyMenu();
	//__super::OnRButtonUp(nFlags, point);
}

void ViewScan::OnEpiFitwindows()
{
	// TODO: 在此添加命令处理程序代码
	m_bFitWndows=!m_bFitWndows;
	SendMessage(WM_SIZE);
	//update_scrollbar(0,0);
	Invalidate();
}

void ViewScan::OnEpiSaveframe()
{
	// TODO: 在此添加命令处理程序代码
	if (hFlushHeader.Vsize<=0)
		return;

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
		HSaveBmp((LPBYTE)hFlushHeader.Vbuffer,m_bmi->bmiHeader.biWidth,
			m_bmi->bmiHeader.biHeight,m_bmi->bmiHeader.biBitCount,fileDlg.GetPathName().GetBuffer());
	}
}

void ViewScan::OnEpiReleaseImg()
{
	if (m_ReBuildPro)
		m_ReBuildPro->UnInitProcess();
}