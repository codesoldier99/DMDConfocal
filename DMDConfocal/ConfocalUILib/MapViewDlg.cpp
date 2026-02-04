// MapViewDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "MapViewDlg.h"
#include "afxdialogex.h"
#include "HGloableFunction.h"

// CMapViewDlg 对话框

IMPLEMENT_DYNAMIC(CMapViewDlg, CDialogEx)

CMapViewDlg::CMapViewDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMapViewDlg::IDD, pParent)
{
	m_MapDate=0;
	m_ConfocalCore=0;
	m_AxisX=0;
	m_AxisY=0;

	bDrawRec=false;
	//bArrived=true;
	m_ROIStartPt.x=200;
	m_ROIStartPt.y=200;
	m_ROISize.x=50;
	m_ROISize.y=50;

	HConfigure m_ConfFile;
	float mVSize=m_ConfFile.GetDouble(L"Size",L"ImageROISize",L"Confocal_Map");
	CString strV=m_ConfFile.GetString(L"Object",L"curobj",L"Confocal_Microscope");
	strV+=L"_mm2pixel";
	float mVPixelSize=m_ConfFile.GetDouble(L"PixelSize",strV,L"Confocal_Microscope");
	ROITraveSize=mVSize*mVPixelSize;
	m_AxisX_Max=m_ConfFile.GetDouble(L"Paras",L"x_limit",L"Confocal_Stage");

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
	//以下GDI
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL);
	brush=new SolidBrush(Color(255, 0, 255, 10));   
}

CMapViewDlg::~CMapViewDlg()
{
	GdiplusShutdown(_gdiplusToken);
	if(m_MapDate)
		delete(m_MapDate);
	delete(m_bmi);
}

void CMapViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMapViewDlg, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()


BOOL CMapViewDlg::OnInitDialog()
{
	__super::OnInitDialog();
	// TODO:  在此添加额外的初始化
	this->SetWindowTextW(L"ConfocalMapView");
	int width,height,wbit;
	HLoadBmp(&m_MapDate,width,height,wbit, HGetStartPath() + L"\\EpiResult\\NaviMap.bmp");
	m_bmi->bmiHeader.biWidth=width;
	m_bmi->bmiHeader.biHeight=height;
	m_bmi->bmiHeader.biBitCount=wbit;
	wbit/=8;
	m_bmi->bmiHeader.biSizeImage=width*height*wbit;//图像大小 
	Invalidate();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CMapViewDlg::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);
	// TODO: 在此处添加消息处理程序代码
	Invalidate();
}

bool CMapViewDlg::InitPanel( LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	m_AxisX=0;
	m_AxisY=0;
	HGearBox* t_GearBox=m_ConfocalCore->GetGearBox();
	if(t_GearBox)
	{
		m_AxisX=t_GearBox->GetAxis(AXIS_X);
		m_AxisY=t_GearBox->GetAxis(AXIS_Y);
		if(!m_AxisX)
			return false;
		if(!m_AxisY)
			return false;
		mClocalPt.x=m_AxisX->GetPosition();
		mClocalPt.y=m_AxisY->GetPosition();
		mScanSPt.x=m_AxisX->GetAxialInfo()->sMinPos;
		mScanSPt.y=m_AxisY->GetAxialInfo()->sMinPos;
		mScanEPt.x=m_AxisX->GetAxialInfo()->sMaxPos;
		mScanEPt.y=m_AxisY->GetAxialInfo()->sMaxPos;
	}
  if (m_AxisX)
  {
	  m_AxisX_Max=m_AxisX->GetAxialInfo()->sMaxPos;
	  m_AxisX->GetSubject()->Attach(this);
  }
  if (m_AxisY)
  {
	  m_AxisY_Max=m_AxisY->GetAxialInfo()->sMaxPos;//载物台的最大位置
	  m_AxisY->GetSubject()->Attach(this);
  }
  if (m_ConfocalCore->GetCoreProcess(PROCESS_MapBuild))
  {
	  m_ConfocalCore->GetCoreProcess(PROCESS_MapBuild)->Attach(this);
  }
	m_ConfocalCore->GetDockablePanel(DOCKPANEL_STAGECTL)->Attach(this);

	Invalidate();
	return false;
}

int CMapViewDlg::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if(pSubject==m_AxisX->GetSubject())
	{
		if(ID==NOTIFY_AXIS_MOVED||ID==NOTIFY_AXIS_POSITION)
		{
			mClocalPt.x=m_AxisX->GetPosition();
			mClocalPt.y=m_AxisY->GetPosition();
			//if (bArrived)
				Invalidate();
			//else
				//bArrived=true;
			TRACE(L"MoveRoi%.3f,%.3f\n",mClocalPt.x,mClocalPt.y);
		}
	}else if (pSubject==m_AxisY->GetSubject())
	{
		if(ID==NOTIFY_AXIS_MOVED||ID==NOTIFY_AXIS_POSITION)
		{
			mClocalPt.x=m_AxisX->GetPosition();
			mClocalPt.y=m_AxisY->GetPosition();
			//if (bArrived)
				Invalidate();
			//else
				//bArrived=true;
			TRACE(L"MoveRoi%.3f,%.3f\n",mClocalPt.x,mClocalPt.y);
		}
	}else if (ID==NOTIFY_MAPROISIZE_FLUSH)
	{
		if (pParam!=0)
		{
			HVideoHeader* pHeader=(HVideoHeader*)pParam;
			if(pHeader->Vsize!=m_bmi->bmiHeader.biSizeImage)
			{
				if(m_MapDate)
					delete(m_MapDate);
				m_bmi->bmiHeader.biWidth=pHeader->Vwidth;
				m_bmi->bmiHeader.biHeight=pHeader->Vheight;
				m_bmi->bmiHeader.biBitCount=pHeader->VwBit;
				m_bmi->bmiHeader.biSizeImage=pHeader->Vsize;
				m_MapDate=(LPBYTE)malloc(pHeader->Vsize);
			}
			memcpy(m_MapDate,pHeader->Vbuffer,pHeader->Vsize);
		}
		ROITraveSize=(wParam+0.0f)/1000;
		Invalidate();
	}
	return 0;
}

// CMapViewDlg 消息处理程序
BOOL CMapViewDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect	rc;
	GetClientRect( &rc );
	DrawDoubleDC(pDC,rc);//用双缓冲的方法
	return TRUE;//__super::OnEraseBkgnd(pDC);
}

void CMapViewDlg::DrawDoubleDC( CDC* pDC,CRect pRc )
{
	//一大段的开始
	CDC MemDC;
	CBitmap MemBitmap;
	MemDC.CreateCompatibleDC(NULL);
	MemBitmap.CreateCompatibleBitmap(pDC,pRc.Width(),pRc.Height());
	CBitmap *pOldBit=MemDC.SelectObject(&MemBitmap);
	//一大段的准备结束

	DrawMap(&MemDC,pRc);//调用绘图

	m_ROISize.x=m_ROISize.y=ROITraveSize/m_AxisX_Max*m_bmi->bmiHeader.biWidth/AllScale;
	DrawROIS(&MemDC);
	//DrawROI(&MemDC,m_ROIStartPt,m_ROISize);//绘制ROI和ScanROI

	pDC->BitBlt(0,0,pRc.Width(),pRc.Height(),&MemDC,0,0,SRCCOPY);
	MemBitmap.DeleteObject();
	MemDC.DeleteDC();
}

void CMapViewDlg::DrawMap( CDC* pDC,CRect pRc )
{
	//画背景
	pDC->FillSolidRect(0,0,pRc.Width(),pRc.Height(),afxGlobalData.clrBarFace);

	//画地图
	if(m_MapDate)
	{
		int destwidth=(pRc.Width()-20);
		int destheight=(pRc.Height()-20);//留两边了//destwidth*m_MapScale; 

		long	srcwidth = m_bmi->bmiHeader.biWidth;
		long	srcheight= m_bmi->bmiHeader.biHeight;//可变大小

		//.算比例.....
		float scalex,scaley;

		scalex=(float)srcwidth/(float)destwidth;
		scaley=(float)srcheight/(float)destheight;
		AllScale=scalex>scaley?scalex:scaley;//谁更小，用谁的

		//输出大小 
		destheight=srcheight/AllScale;
		destwidth=srcwidth/AllScale;

		//算顶点坐标
		int destleft=(pRc.Width()-destwidth)/2;
		int desttop=(pRc.Height()-destheight)/2;

		::SetStretchBltMode(pDC->GetSafeHdc(),HALFTONE);
		::StretchDIBits(
			pDC->GetSafeHdc(),
			destleft, desttop,destwidth,destheight,
			0,0,srcwidth, srcheight,
			m_MapDate,
			m_bmi,
			DIB_RGB_COLORS,
			SRCCOPY
			);
		m_MapStart.x=destleft;//存入地图的大小 参数
		m_MapStart.y=desttop;
		m_MapSize.x=destwidth;
		m_MapSize.y=destheight;
	}
}

void CMapViewDlg::DrawROIS( CDC* pDC)
{
	Graphics g(pDC->m_hDC);
	//计算m_ROIStartPt,m_ROISize
	float scalex=mClocalPt.x/m_AxisX_Max;
	float scaley=mClocalPt.y/m_AxisY_Max;
	CPoint tpos;
	tpos.x=scalex*m_MapSize.x;
	tpos.y=scaley*m_MapSize.y;
	m_ROIStartPt=m_MapStart+tpos;

	int x1=m_ROIStartPt.x;
	int y1=m_ROIStartPt.y;
	int x2=m_ROISize.x;
	int y2=m_ROISize.y;
	Pen ROIPen(Color::Red,1);
	//Pen ROIPen(Color::White,1);
	ROIPen.SetDashStyle(DashStyleDash);
	g.DrawRectangle(&ROIPen,x1,y1,x2,y2);

	////////////左右、上下的直线连接////////////
	Point pt1=Point(m_MapStart.x,y1+y2/2);
	Point pt2=Point(x1,pt1.Y);
	Point pt3=Point(x1+x2/2,m_MapStart.y);
	Point pt4=Point(pt3.X,y1);
	g.DrawLine(&ROIPen,pt1,pt2);
	g.DrawLine(&ROIPen,pt3,pt4);
	pt1=Point(m_MapStart.x+m_MapSize.x,y1+y2/2);
	pt2=Point(x1+x2,pt1.Y);
	pt3=Point(x1+x2/2,m_MapStart.y+m_MapSize.y);
	pt4=Point(pt3.X,y1+y2);
	g.DrawLine(&ROIPen,pt1,pt2);
	g.DrawLine(&ROIPen,pt3,pt4);
	////////////左右、上下的直线连接////////////

	if (bDrawRec)
	{
		if (m_DrawROISize.x>5&&m_DrawROISize.y>5)
		{
			int mX=m_DrawROIEndPt.x>m_DrawROIPt.x?m_DrawROIPt.x:m_DrawROIEndPt.x;
			int mY=m_DrawROIEndPt.y>m_DrawROIPt.y?m_DrawROIPt.y:m_DrawROIEndPt.y;
			g.DrawRectangle(&ROIPen,mX,mY,m_DrawROISize.x,m_DrawROISize.y);
		}
	}
	else
	{
		//计算m_DrawROIPt,m_DrawROISize
		m_DrawROIPt.x=mScanSPt.x/m_AxisX_Max*m_MapSize.x+m_MapStart.x;
		m_DrawROIPt.y=mScanSPt.y/m_AxisY_Max*m_MapSize.y+m_MapStart.y;
		m_DrawROISize.x=(mScanEPt.x-mScanSPt.x)/m_AxisX_Max*m_MapSize.x;
		m_DrawROISize.y=(mScanEPt.y-mScanSPt.y)/m_AxisY_Max*m_MapSize.y;
		g.DrawRectangle(&ROIPen,m_DrawROIPt.x,m_DrawROIPt.y,m_DrawROISize.x,m_DrawROISize.y);
	}

}

void CMapViewDlg::CorrectPt(CPoint* mPT)
{
	CPoint tRect=m_MapStart+m_MapSize;//下边界
	if(mPT->x<=0)
		mPT->x=0;
	if (mPT->y<=0)
		mPT->y=0;
	if(mPT->x<m_MapStart.x)
		mPT->x=m_MapStart.x;//不超出
	if(mPT->y<m_MapStart.y)
		mPT->y=m_MapStart.y;
	if (mPT->x>tRect.x)
		mPT->x=tRect.x;
	if (mPT->y>tRect.y)
		mPT->y=tRect.y;	
}

void CMapViewDlg::MoveStage( CPoint p_Pos )//这个是根据"鼠标"的位置来移动载物台
{
	CPoint tpos=p_Pos-m_MapStart;//去掉地图的头位置
	float scalex=(float)tpos.x/(float)m_MapSize.x;//在地图中的比例
	float scaley=(float)tpos.y/(float)m_MapSize.y;
	scalex*=m_AxisX_Max;//变成实际的坐标
	scaley*=m_AxisY_Max;
	//if (mClocalPt.x!=scalex&&mClocalPt.y!=scaley)
	//	bArrived=false;

	if(m_AxisX)
		m_AxisX->MoveTo(scalex);
	//Sleep(500);
	if(m_AxisY)
		m_AxisY->MoveTo(scaley);
}

void CMapViewDlg::OnMouseMove(UINT nFlags, CPoint point)
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

void CMapViewDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_DrawROIPt=point;
	CorrectPt(&m_DrawROIPt);
	bDrawRec=true;
	//__super::OnRButtonDown(nFlags, point);
}

void CMapViewDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_DrawROISize.x>5&&m_DrawROISize.y>5)
	{
		int mX=m_DrawROIEndPt.x>m_DrawROIPt.x?m_DrawROIPt.x:m_DrawROIEndPt.x;
		int mY=m_DrawROIEndPt.y>m_DrawROIPt.y?m_DrawROIPt.y:m_DrawROIEndPt.y;
		mX-=m_MapStart.x;
		mY-=m_MapStart.y;
		int mEndX=mX+m_DrawROISize.x;
		int mEndY=mY+m_DrawROISize.y;

		float scalex=(float)mX/(float)m_MapSize.x;//在地图中的比例
		float scaley=(float)mY/(float)m_MapSize.y;
		CPointF mStarPt;
		mStarPt.x=scalex*m_AxisX_Max;mStarPt.x=((int)(mStarPt.x*10)/10.0);//取小数点后1位
		mStarPt.y=scaley*m_AxisY_Max;mStarPt.y=((int)(mStarPt.y*10)/10.0);

		scalex=(float)mEndX/(float)m_MapSize.x;//在地图中的比例
		scaley=(float)mEndY/(float)m_MapSize.y;
		CPointF mEndPt;
		mEndPt.x=scalex*m_AxisX_Max;mEndPt.x=((int)(mEndPt.x*10)/10.0);
		mEndPt.y=scaley*m_AxisY_Max;mEndPt.y=((int)(mEndPt.y*10)/10.0);

		mScanSPt=mStarPt;
		mScanEPt=mEndPt;
		Notify(this,NOTIFY_MAPROISTART_FLUSH,0,&mStarPt);
		Notify(this,NOTIFY_MAPROIEND_FLUSH,0,&mEndPt);
	}
	bDrawRec=false;
	Invalidate();
}

void CMapViewDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPoint mPt=point;
	mPt.x-=m_ROISize.x/2;
	mPt.y-=m_ROISize.y/2;
	CorrectPt(&mPt);
	MoveStage(mPt);
	__super::OnLButtonUp(nFlags, point);
}

