// ShowCVDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "ShowCVDlg.h"
#include "afxdialogex.h"
#include "HGloableFunction.h"

// CShowCVDlg 对话框

IMPLEMENT_DYNAMIC(CShowCVDlg, CDialogEx)

CShowCVDlg::CShowCVDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CShowCVDlg::IDD, pParent)
	, m_iTh(150)
{
	m_ConfocalCore=0;
	m_CaptureVideo.Vsize=0;//表示没有东西
	m_CaptureVideo.Vbuffer=0;
	CalCVProcess=0;
	iCount=0;
	bCalNow=false;
	bTh=false;
	bSetCal=false;
	m_hCaptureEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_hMoveEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
  mCVParas =new CalCVParas;
  m_AxisX = 0;
  m_AxisY = 0;
}

CShowCVDlg::~CShowCVDlg()
{
  delete mCVParas;
}

void CShowCVDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CVPic, m_ChartCV);  
	DDX_Control(pDX, IDC_CVPic2,m_HistView);

	DDX_Text(pDX, IDC_EDIT1, m_iTh);
	DDV_MinMaxInt(pDX, m_iTh, 1, 255);
	DDX_Control(pDX, IDC_SLIDERTh, mThSlitder);
}


BEGIN_MESSAGE_MAP(CShowCVDlg, CDialogEx)
	ON_BN_CLICKED(IDC_StartBtn, &CShowCVDlg::OnBnClickedStartbtn)
	ON_WM_CTLCOLOR()
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK1, &CShowCVDlg::OnBnClickedCheck1)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_SentCalBtn, &CShowCVDlg::OnBnClickedSentcalbtn)
	ON_BN_CLICKED(IDC_CHECK3, &CShowCVDlg::OnBnClickedCheck3)
END_MESSAGE_MAP()


// CShowCVDlg 消息处理程序
bool CShowCVDlg::InitPanel(LPVOID p_Param, LPVOID p_Param2)
{
	if (!m_ConfocalCore)
		m_ConfocalCore=(HConfocalCore*)p_Param;
	if (!CalCVProcess)
	{
		CalCVProcess=m_ConfocalCore->GetCoreProcess(PROCESS_CalValue);
		ReadProcessParas();
	}	
	if (m_ConfocalCore)
	{
    if (m_ConfocalCore->GetGearBox())
    {
		  m_AxisX=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_X);
    }
		if (m_AxisX)
			m_AxisX->GetSubject()->Attach(this);
    if (m_ConfocalCore->GetGearBox())
    {
		  m_AxisY=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Y);
    }
		if (m_AxisY)
			m_AxisY->GetSubject()->Attach(this);
	}

	HCorePanel* m=m_ConfocalCore->GetViewPanel(VIEWPANEL_REALTIME);
	((IHsmSubject*)this)->Attach(m);
	m = m_ConfocalCore->GetViewPanel(VIEWPANEL_REALTIMEEX);
	((IHsmSubject*)this)->Attach(m);
	return true;
}

void CShowCVDlg::OnBnClickedStartbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (bTh)
	{
		bTh=false;
		m_ConfocalCore->GetCurVideo()->Pause();
		Sleep(500);	
		m_ConfocalCore->GetRenderChain()->remove(this);	
		SetDlgItemText(IDC_StartBtn,L"Start");
		m_renderwnd.SetDrawable(true);
		KillTimer(2);
		m_ConfocalCore->GetRenderChain()->push_back(&m_renderwnd);
		Sleep(500);
		m_ConfocalCore->GetCurVideo()->Run();
	}
	else
	{
		m_realArryCount=0;
		m_pLineCVSerie->ClearSerie();
		m_ConfocalCore->GetCurVideo()->Pause();
		Sleep(500);	
		m_ConfocalCore->GetRenderChain()->remove(&m_renderwnd);
		m_ConfocalCore->GetRenderChain()->push_back(this);
		Sleep(500);	
		m_ConfocalCore->GetCurVideo()->Run();
		bTh=true;
		CreateThread(NULL, 0, CalProc, this, 0, 0);//这个开线程
		SetDlgItemText(IDC_StartBtn,L"Stop");
		m_renderwnd.SetDrawable(false);
		SetTimer(2,0,NULL);		
	}
}

void CShowCVDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类
	//AfxMessageBox(L"关闭CV！");
	if (bTh)
	{
		bTh=false;	
		m_ConfocalCore->GetCurVideo()->Pause();
		Sleep(300);
		m_ConfocalCore->GetRenderChain()->remove(this);
		SetDlgItemText(IDC_StartBtn,L"Start");
		m_renderwnd.SetDrawable(true);
		KillTimer(2);
		if (m_ConfocalCore)
			m_ConfocalCore->GetRenderChain()->remove(&m_renderwnd);
		m_ConfocalCore->GetCurVideo()->Run();
	}
	__super::OnCancel();
}

void CShowCVDlg::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
{
	if (bCalNow)
		return;
	int width=pHeader->Vwidth;
	int heidth=pHeader->Vheight;
	int wbit=pHeader->VwBit;
	if(m_CaptureVideo.Vsize!=pHeader->Vsize)
	{
		if(m_CaptureVideo.Vbuffer)
			delete(m_CaptureVideo.Vbuffer);
		m_CaptureVideo.Vbuffer=(LPBYTE)malloc(pHeader->Vsize);
		m_CaptureVideo.Vwidth=pHeader->Vwidth;
		m_CaptureVideo.Vheight=pHeader->Vheight;
		m_CaptureVideo.Vsize=pHeader->Vsize;
		m_CaptureVideo.VwBit=pHeader->VwBit;
	}
	if (!pBuffer)
		return;
	memcpy(m_CaptureVideo.Vbuffer,pBuffer,m_CaptureVideo.Vsize);

	//if (bSetCal)
	//{
	//	bSetCal=false;
	//	Cal_TYPE mCul=Cal_LightAvg;
	//	bool bReturn=CalCVProcess->ProcessImg(&mCul,0,&m_CaptureVideo);
	//	//m_ConfocalCore->GetRenderChain()->remove(this);
	//}

	bCalNow=true;
	SetEvent(m_hCaptureEvent);
}

BOOL CShowCVDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: 在此添加专用代码和/或调用基类
	//if(message == WM_USER + 520)
	//{

	//}
	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

LRESULT CShowCVDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	return __super::WindowProc(message, wParam, lParam);
}

HBRUSH CShowCVDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_STATIC:
			pDC->SetBkMode(TRANSPARENT);
			break;
		case IDC_CVPic:
			pDC->SetBkMode(TRANSPARENT);
			break;
		case IDC_SLIDERTh:
			pDC->SetBkMode(TRANSPARENT);
			break;
	}
	// TODO:  在此更改 DC 的任何特性
	return afxGlobalData.brBarFace;
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

DWORD WINAPI CalProc( LPVOID lp )
{
	((CShowCVDlg*)lp)->CalImage();
	return 0;
}

void CShowCVDlg::CalImage()
{
	while(bTh)
	{
		while(WaitForSingleObject(m_hCaptureEvent,1300)==WAIT_TIMEOUT)
			continue;
		//UpdateData();
		Cal_TYPE mCul=Cal_CV;
		CalValueParas mV;
		mV.mROIRec=m_renderwnd.mROIRealRec;
		mV.iTh=0;
		if (((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck())
			mV.iTh=m_iTh;	

		if (((CButton*)GetDlgItem(IDC_CHECK3))->GetCheck())
		{
			mCul=Set_LightAvg;
			CalCVProcess->ProcessImg(&mCul,&mV,&m_CaptureVideo);//这里进行判断，要不要使用模板去除功能
		}
		//////////////////////////////传给OpenCV后图像会上下颠倒//////////////////////////////
		int iTop=mV.mROIRec.top;
		mV.mROIRec.top=m_CaptureVideo.Vheight-mV.mROIRec.bottom;
		mV.mROIRec.bottom=m_CaptureVideo.Vheight-iTop;
		//////////////////////////////传给OpenCV后图像会上下颠倒//////////////////////////////
		mCul=Cal_CV;
		bool bReturn=CalCVProcess->ProcessImg(&mCul,&mV,&m_CaptureVideo);
		CString strV=L"";
		strV.Format(L"Max:%.3f with %d\r\nMin:%.3f with %d\r\nMean:%.3f\r\nStd:%.3f\r\nCV:%.2f\r\n",
			mV.MaxValue,mV.GrayHist[(int)mV.MaxValue],mV.MinValue,mV.GrayHist[(int)mV.MinValue],
			mV.MeanValue,mV.SDValue,mV.CVValue*100);
		SetDlgItemText(IDC_EDITValueCV,strV);
		ResetEvent(m_hCaptureEvent);
		m_renderwnd.Renderer(&m_CaptureVideo,m_CaptureVideo.Vbuffer);//刷新图
		bCalNow=false;	

		if (m_realArryCount>=m_ArrayCount)
			LeftMoveArray(m_CVArray,m_ArrayCount,mV.CVValue*100);	
		else
		{
			*(m_CVArray+m_realArryCount)=mV.CVValue*100;
			if (m_realArryCount<m_ArrayCount)
				m_realArryCount++;
		}
		//记录直方图
		for (int i=0;i<256;i++)
			m_HistValue[i]=(double)mV.GrayHist[i];
		
	}
}

DWORD WINAPI SetCVProc(LPVOID lp)
{
	((CShowCVDlg*)lp)->SetCVImage();
	return 0;
}

void CShowCVDlg::SetCVImage()
{
	//移动方向过来进行均匀度计算
	float iDirect=1.0f;
	CString strProcess=L"Status: Calculate the average image……";
	for (int i=0;i<mCVParas->iUseImg;i++)
	{
		if (!bSetCVImage)
			break;	
		if (!Capture())//触发图像采集
		{
			bSetCVImage=false;
			break;
		}
		Cal_TYPE mCul=Cal_LightAvg;		
		Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strProcess);
		bool bReturn=CalCVProcess->ProcessImg(&mCul,&i,&m_CaptureVideo);//处理图像
		if (i==mCVParas->iUseImg-1)
			continue;
		if (i%3!=0)
		{
			if (!MoveAxisRef(m_AxisX,MoveStep*iDirect,L"Status:  X Moving"))
			{
				bSetCVImage=false;
				break;
			}
		}
		else
		{
			if (!MoveAxisRef(m_AxisY,MoveStep,L"Status:  Y Moving"))
			{
				bSetCVImage=false;
				break;
			}
			iDirect=-iDirect;
		}
		Sleep(100);
	}
	m_ConfocalCore->GetRenderChain()->remove(this);
	m_ConfocalCore->GetCurVideo()->SetTrigerMode(TRIGER_INTERAL);
	Notify(this,NOTIFY_MSGVIEW_END,0,0);
}

BOOL CShowCVDlg::OnInitDialog()
{
	__super::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CRect	rc;
	GetClientRect( &rc );
	m_renderwnd.Create(NULL,L"test", WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE, rc, this, 0 );
	m_renderwnd.SetWindowPos(NULL, 0, 0, 256, 256, SWP_NOZORDER );
	m_renderwnd.ShowWindow(SW_SHOW);
	m_renderwnd.SetDrawable(true);

	CChartAxis *pAxis= NULL;   
	pAxis = m_ChartCV.CreateStandardAxis(CChartCtrl::BottomAxis);  
	pAxis->SetAutomatic(true);  
	pAxis = m_ChartCV.CreateStandardAxis(CChartCtrl::LeftAxis);  
	pAxis->SetAutomatic(true);  
	m_pLineCVSerie = m_ChartCV.CreateLineSerie();
	m_ArrayCount=30;m_realArryCount=0;
	m_CVArray=new double[m_ArrayCount];
	m_X=new double[m_ArrayCount];
	m_Pt=new Point[m_ArrayCount];
	for (int i=0;i<m_ArrayCount;i++)
	{
		m_X[i]=i;
		m_CVArray[i]=0;
		m_Pt[i].X=0;m_Pt[i].Y=0;
	}
	m_pLineCVSerie->AddPoints(m_X,m_CVArray,m_ArrayCount);	
	TChartString str1;  
	str1 = _T("CV-Value View");  
	m_ChartCV.GetTitle()->AddString(str1); 

	m_HistX=new double[256];
	m_HistValue=new double[256];
	for (int i=0;i<256;i++)
	{
		m_HistX[i]=i;
		m_HistValue[i]=0;
	}
	pAxis = m_HistView.CreateStandardAxis(CChartCtrl::BottomAxis);  
	pAxis->SetAutomatic(true);  
	pAxis = m_HistView.CreateStandardAxis(CChartCtrl::LeftAxis);  
	pAxis->SetAutomatic(true);  
	m_BarSerie = m_HistView.CreateBarSerie();
	/*str1 = _T("Hist-gray View");  
	m_HistView.GetTitle()->AddString(str1); */
	m_BarSerie->SetBorderColor(RGB(0,0,0));
	m_BarSerie->SetBarWidth(3);
	m_HistView.SetBackGradient(RGB(205,205,205),RGB(175,175,275),gtVertical);

	m_ChartCV.EnableRefresh(false);// Disable the refresh of the control
	m_ChartCV.SetBackGradient(RGB(255,255,255),RGB(125,125,255),gtVertical);// Set the gradient for the background
	m_ChartCV.GetLegend()->DockLegend(CChartLegend::dsDockBottom);	// Dock the legend at the bottom
	m_ChartCV.GetLegend()->SetHorizontalMode(true);// Specifies that the legend entries are horizontally stacked
	m_ChartCV.EnableRefresh(true);// Re-enable the refresh of the control

	OnBnClickedCheck1();
	mThSlitder.SetRange(1,255);//设置曝光时间
	mThSlitder.SetPos(256-150);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CShowCVDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);
	if (bShow)
	{
		if (m_ConfocalCore)
			m_ConfocalCore->GetRenderChain()->push_back(&m_renderwnd);
	}
	else
	{

	}
	// TODO: 在此处添加消息处理程序代码
}

void CShowCVDlg::LeftMoveArray(double* ptr,int length,double data)
{
	for (int i=1;i<length;++i)
	{
		ptr[i-1] = ptr[i];
	}
	ptr[length-1] = data;
}

void CShowCVDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//if (bCalNow)
		//return;
	if (2 == nIDEvent)
	{		
		if (m_realArryCount>=m_ArrayCount)//如果超过要画的点数目，要清理后再加载绘制
		{
			m_pLineCVSerie->ClearSerie();
			m_pLineCVSerie->AddPoints(m_X,m_CVArray,m_ArrayCount);
		}			
		else
		{
			if (m_realArryCount==0)//更新了m_CVArray里面第m_realArryCount个值，然后m_realArryCount已经++，指向了下一个
				m_pLineCVSerie->AddPoint(*(m_X+m_realArryCount),*m_CVArray);
			else
				m_pLineCVSerie->AddPoint(*(m_X+m_realArryCount),*(m_CVArray+m_realArryCount-1));
			
		}
		m_BarSerie->ClearSerie();
		m_BarSerie->AddPoints(m_HistX,m_HistValue,256);
	}
	__super::OnTimer(nIDEvent);
}

void CShowCVDlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	bool bEnable=((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
	GetDlgItem(IDC_EDIT1)->EnableWindow(bEnable);
	GetDlgItem(IDC_SLIDERTh)->EnableWindow(bEnable);
}

void CShowCVDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_iTh=256-mThSlitder.GetPos();
	UpdateData(false);
	__super::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CShowCVDlg::OnBnClickedSentcalbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (bTh)
		OnBnClickedStartbtn();

	//读取参数
	ReadProcessParas();

	bSetCal=true;
	bCalNow=false;
	bSetCVImage=true;

	m_ConfocalCore->GetRenderChain()->push_back(this);//加入渲染
	ResetEvent(m_hMoveEvent);
	ResetEvent(m_hCaptureEvent);
	m_ConfocalCore->GetCurVideo()->SetTrigerMode(TRIGER_SOFT);

	m_CalCVThread=CreateThread(NULL, 0, SetCVProc, this, 0, 0);//这个开线程
	HCorePanel* mDlg=m_ConfocalCore->GetMsgPanel(MSG_TIME);
	mDlg->InitPanel(0);
	((IHsmSubject*)this)->Attach(mDlg);
	this->PostMessage(0x118);
	//m_ConfocalCore->GetDockablePanel(DOCKPANEL_STAGECTL)
	//	->GetCWnd()->PostMessage(0x118);
	((CDialogEx*)mDlg->GetCWnd())->DoModal();
	((IHsmSubject*)this)->Detach(mDlg);
	bSetCVImage=false;//退出线程

	WaitForSingleObject(m_CalCVThread,2000);
	

	//m_ConfocalCore->GetCurVideo()->Pause();
	//Sleep(500);	
	//m_ConfocalCore->GetRenderChain()->push_back(this);
	//Sleep(500);	
	//m_ConfocalCore->GetCurVideo()->Run();
}

void CShowCVDlg::ReadProcessParas()
{
	//读取参数
	mCVParas->iBKErode=m_ConfocalCore->GetConfigure()->GetInt(L"CVParas",L"iBKErode",L"Confocal_Camera");
	mCVParas->iUseImg=m_ConfocalCore->GetConfigure()->GetInt(L"CVParas",L"iUseImg",L"Confocal_Camera");
	mCVParas->strBKImg = HGetStartPath() + L".//EpiResult//BGImg.bmp";
		//m_ConfocalCore->GetConfigure()->GetString(L"CVParas",L"strBKImg",L"Confocal_Camera");
	mCVParas->strBKImg=m_ConfocalCore->GetConfigure()->GetModulePath()+mCVParas->strBKImg;
	float ImageROISize=m_ConfocalCore->GetConfigure()->GetDouble(L"Size",L"ImageROISize",L"Confocal_Map");
	CString strObject=m_ConfocalCore->GetConfigure()->GetString(L"Object",L"curobj",L"Confocal_Microscope");
	CString strV=strObject+L"_mm2pixel";
	float PixelSize=m_ConfocalCore->GetConfigure()->GetDouble(L"PixelSize",strV,L"Confocal_Microscope");
	MoveStep=ImageROISize*PixelSize;
	CalCVProcess->InitProcess(mCVParas);
}

void CShowCVDlg::OnBnClickedCheck3()
{
	// TODO: 在此添加控件通知处理程序代码
	int iK=((CButton*)GetDlgItem(IDC_CHECK3))->GetCheck();
	//m_ChartCV.SaveAsImage(L"D:\\1.bmp",CRect(0,0,0,0),24);
	//m_HistView.SaveAsImage(L"D:\\2.bmp",CRect(0,0,0,0),24);
	//m_HistView.Print(L"Test");
	this->Notify(this,NOTIFY_CVCal_FLUSH,iK);

}

int CShowCVDlg::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if(pSubject==m_AxisX->GetSubject())
	{
		if(ID==NOTIFY_AXIS_MOVED)//NOTIFY_AXIS_POSITION)//
		{
			SetEvent(m_hMoveEvent);
		}

	}else if (pSubject==m_AxisY->GetSubject())
	{
		if(ID==NOTIFY_AXIS_MOVED)//NOTIFY_AXIS_POSITION)//
		{
			SetEvent(m_hMoveEvent);
		}
	}

	return 1;
}

bool CShowCVDlg::MoveAxisRef(HAxis *m_Axis,float mPos,CString strOut)
{
	int iTime=7000;
	Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strOut);
	ResetEvent(m_hMoveEvent);
	m_Axis->MoveRef(mPos);
	if (WaitForSingleObject(m_hMoveEvent,iTime)==WAIT_TIMEOUT)
	{
		CString strMsg=strOut+L"  TIMEOUT!";
		Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
		Sleep(3000);
		return false;
	}
	return true;
}

bool CShowCVDlg::Capture()
{	
	ResetEvent(m_hCaptureEvent);
	bCalNow=false;	
	CString strMsg=L"Status:   Capturing!";
	Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
	m_ConfocalCore->GetCurVideo()->TrigerVideoData();
	int iCount=0;
	while(WaitForSingleObject(m_hCaptureEvent,1300)==WAIT_TIMEOUT)
	{
		iCount++;
		if (iCount<=3)
		{
			ResetEvent(m_hCaptureEvent);
			bCalNow=false;	
			m_ConfocalCore->GetCurVideo()->TrigerVideoData();
		}	
		else
		{
			strMsg=L"Status:   Capturing Time Out!";
			Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
			Sleep(3000);
			return false;
		}		
	}
	return true;
}


