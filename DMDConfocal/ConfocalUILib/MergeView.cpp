#include "StdAfx.h"
#include "MergeView.h"
#include "Resource.h"
#include <Mmsystem.h>            
#pragma comment(lib, "Winmm.lib")  

CMergeView::CMergeView(void)
{
	m_Buffer=0;
	ZoomHeight=1;
	ZoomWidth=1;
	m_hPos=0;
	m_vPos=0;
	ZoomAmp=1;
	m_bDragMouse=false;
	m_MouseX.SetPoint(0,0);
	m_MouxeY.SetPoint(0,0);
	m_FpsLastValue=0.0;
	m_bUpdata=false;
	//////////////初始化调色板//////////////
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
	//brush=new SolidBrush(Color(255, 199, 237, 204));  
	brush=new SolidBrush(Color(255,0x00,181,4));  
	stringformat.SetAlignment(StringAlignmentCenter);  
	stringformat.SetLineAlignment(StringAlignmentCenter);

	m_bFitWndows=true;
	m_bExType = false;
}

CMergeView::~CMergeView(void)
{
	if(m_Buffer)
	{
		delete(m_Buffer);
		m_Buffer=0;
	}
	delete(m_bmi);
	GdiplusShutdown(_gdiplusToken);
}

bool CMergeView::InitPanel( LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;

	if (p_Param2)
		m_bExType = *((bool*)p_Param2);
	if (m_bExType&&p_Param2)
	{
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_ConfocalEx)->push_back(this);
		HConfocalPlug* m = m_ConfocalCore->GetPlugin(PLUGIN_CONFOCALMODEEX);
		((IHsmSubject*)this)->Attach(m);
	}
	else
	{
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->push_back(this);
		HConfocalPlug* m = m_ConfocalCore->GetPlugin(PLUGIN_CONFOCALMODE);
		((IHsmSubject*)this)->Attach(m);
	}
	return true;
}



//confcalView中的显示相关操作
void CMergeView::UpdataView(int pHpos,int pVpos )
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

void CMergeView::Render(LPBYTE pBuffer, int width,int heidth,int wbit )
{
	DWORD newSize=width*heidth*wbit/8;
	if(!m_Buffer)//第一次开启
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
	//算fps
	//DWORD t_now=::GetTickCount();
	DWORD t_now=timeGetTime();
	DWORD dutime=t_now-m_FpsLastTime;
	if (dutime<=0||dutime<=16)
	{
		float mV=9.93f+(dutime+0.1f)/10;
		m_FpsLastValue=1000.0f/mV;
		m_FpsLastTime=t_now;
	}
	else
	{
		m_FpsLastValue=1000.0/dutime;
		m_FpsLastTime=t_now;
	}	

	//m_FpsLastValue=1000.0f/dutime;
	//m_FpsLastTime=t_now;
}

void CMergeView::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
{
	int width=pHeader->Vwidth;
	int heidth=pHeader->Vheight;
	int wbit=pHeader->VwBit;
	Render(pBuffer,width,heidth,wbit);
}

void CMergeView::update_scrollbar( int width,int height )
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

void CMergeView::DrawFunction(CDC* pDC,CRect pRc)
{
	pDC->FillSolidRect(0,0,pRc.Width(),pRc.Height(),afxGlobalData.clrBarFace);
	if(m_Buffer)// pvar_wndBitmap->dibits != NULL )
	{
		long destwidth2=pRc.Width();
		long destheight2=pRc.Height(); 
		long	destleft = 0;
		long	desttop = 0;
		long	destwidth=pRc.Width();
		long	destheight=pRc.Height();
		long	srcwidth = m_bmi->bmiHeader.biWidth;
		long	srcheight=m_bmi->bmiHeader.biHeight;
		if(srcwidth<destwidth)
		{
			destleft=(destwidth2-srcwidth)/2;
			destwidth=srcwidth;
		}
		else
			srcwidth=destwidth;

		if (srcheight<destheight)
		{
			desttop=(destheight2-srcheight)/2;
			destheight=srcheight;
		}
		else
			srcheight=destheight;
		::StretchDIBits(
			pDC->GetSafeHdc(),
			destleft, desttop,destwidth,destheight,
			m_hPos,m_vPos,srcwidth, srcheight,
			m_Buffer,
			m_bmi,
			DIB_RGB_COLORS,
			SRCCOPY
			);
	}
}

void CMergeView::DrawFunctionM2(CDC* pDC,CRect pRc)
{
	pDC->FillSolidRect(0,0,pRc.Width(),pRc.Height(),afxGlobalData.clrBarFace);
	if(m_Buffer)// pvar_wndBitmap->dibits != NULL )
	{
		long	destleft = 0;
		long	desttop = 0;
		long	destwidth=pRc.Width();
		long	destheight=pRc.Height();
		long	srcwidth = m_bmi->bmiHeader.biWidth;
		long	srcheight=m_bmi->bmiHeader.biHeight;
		//.算比例.....
		float scalex,scaley;
		scalex=(float)srcwidth/(float)destwidth;
		scaley=(float)srcheight/(float)destheight;
		float AllScale=scalex>scaley?scalex:scaley;//谁更小，用谁的
		//输出大小 
		destheight=srcheight/AllScale;
		destwidth=srcwidth/AllScale;
		//算顶点坐标
		destleft=(pRc.Width()-destwidth)/2;
		desttop=(pRc.Height()-destheight)/2;
		::SetStretchBltMode(pDC->GetSafeHdc(),HALFTONE);
		::StretchDIBits(
			pDC->GetSafeHdc(),
			destleft, desttop,destwidth,destheight,
			0,0,srcwidth, srcheight,//m_hPos,m_Vpos是滚动条的滚后位置
			m_Buffer,
			m_bmi,
			DIB_RGB_COLORS,
			SRCCOPY
			);
	}
}

void CMergeView::DoubleDCFun( CDC* pDC,CRect pRc )
{
	//一大段的开始
	CDC MemDC;
	CBitmap MemBitmap;
	MemDC.CreateCompatibleDC(NULL);
	MemBitmap.CreateCompatibleBitmap(pDC,pRc.Width(),pRc.Height());
	CBitmap *pOldBit=MemDC.SelectObject(&MemBitmap);
	//一大段的准备结束
	if(m_bFitWndows)//充满窗口用另一种方法 
		DrawFunctionM2(&MemDC,pRc);
	else
		DrawFunction(&MemDC,pRc);
	//DrawFunction(&MemDC,pRc);//调用绘图
	//DrawRuler(&MemDC,m_MouseX,m_MouxeY);//画标尺
	DrawFps(&MemDC,pRc,m_FpsLastValue);
	pDC->BitBlt(0,0,pRc.Width(),pRc.Height(),&MemDC,0,0,SRCCOPY);
	MemBitmap.DeleteObject();
	MemDC.DeleteDC();
}

void CMergeView::DrawRuler( CDC* pDC,CPoint p_x1,CPoint p_x2 )
{
	if(p_x1==p_x2)
		return;
	Graphics g(pDC->m_hDC);
	int x1=p_x1.x;
	int y1=p_x1.y;
	int x2=p_x2.x;
	int y2=p_x2.y;
	Pen roi_pen(Color::Red,2);
	g.DrawLine(&roi_pen,x1,y1,x2,y2);
	GetClientRect(rcClient); 
	CString text;
	text.Format(_T("%.2f"),sqrt((float)((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)))/(float)ZoomAmp);
	//g.DrawString(text.GetBuffer(), 4, &font,  
	//	RectF(x2,y2, 20, 20), &stringformat, &brush); 
	//g.DrawString(text.GetBuffer(),text.GetLength(),0,0,&roi_pen);
	//g.DrawString();
	g.DrawString(text.GetBuffer(),text.GetLength(),font,PointF(x2+15,y2-10),&stringformat,brush);
	//画标尺
#define  LineLong 5	//线长
	int xx=x2-x1;
	int yy=y2-y1;
	double xc,yc;
	if(xx==0)
	{
		xc=LineLong;
		yc=0;
	}
	else if(yy==0)
	{
		yc=LineLong;
		xc=0;
	}
	else
	{
		xc=LineLong/sqrt((double)(1.0+(double)xx*xx/(yy*yy))); //X^+X*(X1^*Y1^)=LineLong^;
		yc=xc*xx/(double)yy;
	}
	int xc1=x1-xc;
	int yc1=y1+yc;
	int xc2=x2-xc;
	int yc2=y2+yc;
	xc+=x1;
	yc=y1-yc;
	g.DrawLine(&roi_pen,xc1,yc1,xc,yc);
	g.DrawLine(&roi_pen,xc2,yc2,xc+xx,yc+yy);
}

void CMergeView::DrawFps(CDC* pDC,CRect pRc,float pFps)
{
	Graphics g(pDC->m_hDC);

	CString text;
	text.Format(_T("%.2f fps"),pFps);
	g.DrawString(text.GetBuffer(),text.GetLength(),font,PointF(60,50),&stringformat,brush);

}

BEGIN_MESSAGE_MAP(CMergeView, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_COMMAND(ID_MODE_STARTCONFOCAL, &CMergeView::OnModeStartconfocal)
	ON_COMMAND(ID_MODE_SAVEFRAME, &CMergeView::OnModeSaveframe)
	ON_COMMAND(ID_MODE_STOPCONFOCAL, &CMergeView::OnModeStopconfocal)
	ON_COMMAND(ID_MODE_SAVEPROCESSIMG, &CMergeView::OnModeSaveprocessimg)
	ON_COMMAND(ID_MODE_FITWINDOW, &CMergeView::OnFITWINDOW)
END_MESSAGE_MAP()


BOOL CMergeView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect	rc;
	GetClientRect( &rc );
	DoubleDCFun(pDC,rc);//用双缓冲的方法
	return __super::OnEraseBkgnd(pDC);
}

void CMergeView::OnSize(UINT nType, int cx, int cy)
{
	//__super::OnSize(nType, cx, cy);
	CWnd::OnSize(nType, cx, cy);
	update_scrollbar( m_bmi->bmiHeader.biWidth,m_bmi->bmiHeader.biHeight );
	UpdataView(m_nHOffset,m_nVOffset);
	// TODO: 在此处添加消息处理程序代码
}

void CMergeView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
		//update_bitmap();
		UpdataView(m_nHOffset,m_nVOffset);
	}

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
	//__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMergeView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
	//__super::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CMergeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_bDragMouse=true;
	m_MouseX=point;
	m_MouxeY=point;
	__super::OnLButtonDown(nFlags, point);
}

void CMergeView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//if(!m_bUpdata)
	//	SetTimer(529,30,NULL);// 刷新进程
	//m_bUpdata=true;
	__super::OnRButtonDown(nFlags, point);
}

void CMergeView::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	ClientToScreen(&point);
	CMenu tmenu;
	tmenu.LoadMenuW(IDR_MENUConfocal);//加入菜单
	CMenu* pPopMenu=tmenu.GetSubMenu(0);
	pPopMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,point.x,point.y,this);//显示菜单，左边，跟踪右键
	pPopMenu->Detach();
	tmenu.DestroyMenu();
	//__super::OnRButtonUp(nFlags, point);
}

void CMergeView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_bDragMouse=false;
	m_MouxeY=point;
	__super::OnLButtonUp(nFlags, point);
}

void CMergeView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_bDragMouse)
		m_MouxeY=point;
	__super::OnMouseMove(nFlags, point);
}

void CMergeView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(nIDEvent==529)//刷新进程
	{
		Invalidate();
		//KillTimer(529);
	}		
	CWnd::OnTimer(nIDEvent);
	//__super::OnTimer(nIDEvent);
}

void CMergeView::OnModeStartconfocal()
{
	// TODO: 在此添加命令处理程序代码
	this->Notify(this,NOTIFY_CONFOCAL_START);
}

void CMergeView::OnModeStopconfocal()
{
	// TODO: 在此添加命令处理程序代码
	this->Notify(this,NOTIFY_CONFOCAL_STOP);
}

void CMergeView::OnModeSaveframe()
{
	// TODO: 在此添加命令处理程序代码
	this->Notify(this,NOTIFY_CONFOCAL_SAVERESULT);
}

void CMergeView::OnModeSaveprocessimg()
{
	// TODO: 在此添加命令处理程序代码
	this->Notify(this,NOTIFY_CONFOCAL_SAVEPROCESS);
}

void CMergeView::OnFITWINDOW()
{
	m_bFitWndows=!m_bFitWndows;
	SendMessage(WM_SIZE);
	Invalidate();
}