#include "StdAfx.h"
#include "ColorView.h"


CColorView::CColorView(void)
{
	m_Buffer=0;
	m_nHOffset=0;
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
	font=new Gdiplus::Font(fontFamily, 15, FontStyleRegular, UnitPixel);
	//LOGFONT lf;  
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);  
	memcpy(lf.lfFaceName, "Arial", 5) ;  
	lf.lfHeight = 13 ;  
	brush=new SolidBrush(Color(255, 0, 255, 10));  
	stringformat.SetAlignment(StringAlignmentCenter);  
	stringformat.SetLineAlignment(StringAlignmentCenter);
}


CColorView::~CColorView(void)
{
}

void CColorView::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
{
	int width=pHeader->Vwidth;
	int heidth=pHeader->Vheight;
	int wbit=pHeader->VwBit;
	//Render(pBuffer,width,heidth,wbit);
}

int CColorView::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if(ID==NOTIFY_COLORBUILD_END)
	{
		HVideoHeader* pHeader=(HVideoHeader*)pParam;
		int width=pHeader->Vwidth;
		int heidth=pHeader->Vheight;
		int wbit=pHeader->VwBit;
		Render(pHeader->Vbuffer,width,heidth,wbit);
		Invalidate();
	}

	return 0;
}

void CColorView::Render(LPBYTE pBuffer, int width,int heidth,int wbit )
{
	DWORD newSize=width*heidth*wbit/8;

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
}

void CColorView::update_scrollbar( int width,int height )
{
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
	//if( width > 0 && height > 0 )
	//{
	//	if( hInfo.nMin < 0 )
	//		hInfo.nMin = 0;
	//	hInfo.nMax = width;
	//	hInfo.nPage = rc.Width();
	//	if( (int)( hInfo.nPos + hInfo.nPage - hInfo.nMax ) > 0 )
	//	{
	//		hInfo.nPos = hInfo.nMax -  hInfo.nPage;
	//		m_nHOffset = hInfo.nPos;//这里暂时不记录
	//	}
	//	if( width == rc.Width() )
	//		hInfo.nMax = 0;
	//	if( vInfo.nMin < 0 )
	//		vInfo.nMin = 0;
	//	vInfo.nMax = height;
	//	vInfo.nPage = rc.Height();

	//	if( (int)( vInfo.nPos + vInfo.nPage - vInfo.nMax ) > 0 )
	//	{
	//		vInfo.nPos = vInfo.nMax -  vInfo.nPage;
	//		m_nVOffset = vInfo.nPos;//这里暂时不记录
	//	}
	//	if( height == rc.Height() )
	//		vInfo.nMax = 0;	
	//}
	//else
	//{
		hInfo.nMin = 0;
		hInfo.nMax = 0;
		hInfo.nPage = rc.Width();

		vInfo.nMin = 0;
		vInfo.nMax = 0;
		vInfo.nPage = rc.Height();
	//}
	SetScrollInfo( SB_HORZ, &hInfo );
	SetScrollInfo( SB_VERT, &vInfo );
}

void CColorView::UpdataView(int pHpos,int pVpos )
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

BEGIN_MESSAGE_MAP(CColorView, CWnd)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void CColorView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CColorView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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

void CColorView::OnSize(UINT nType, int cx, int cy)
{
	//__super::OnSize(nType, cx, cy);
	CWnd::OnSize(nType, cx, cy);
	update_scrollbar( m_bmi->bmiHeader.biWidth,m_bmi->bmiHeader.biHeight );
	UpdataView(m_nHOffset,m_nVOffset);
	// TODO: 在此处添加消息处理程序代码
}

BOOL CColorView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect	rc;
	GetClientRect( &rc );
	DoubleDCFun(pDC,rc);//用双缓冲的方法
	return __super::OnEraseBkgnd(pDC);
}

void CColorView::DoubleDCFun( CDC* pDC,CRect pRc )
{
	//一大段的开始
	CDC MemDC;
	CBitmap MemBitmap;
	MemDC.CreateCompatibleDC(NULL);
	MemBitmap.CreateCompatibleBitmap(pDC,pRc.Width(),pRc.Height());
	CBitmap *pOldBit=MemDC.SelectObject(&MemBitmap);
	//一大段的准备结束
    DrawFunction(&MemDC,pRc);//调用绘图
	pDC->BitBlt(0,0,pRc.Width(),pRc.Height(),&MemDC,0,0,SRCCOPY);
	MemBitmap.DeleteObject();
	MemDC.DeleteDC();
}

void CColorView::DrawFunction(CDC* pDC,CRect pRc)
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
