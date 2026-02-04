// VideoConfDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "VideoConfDlg.h"
#include "afxdialogex.h"


// CVideoConfDlg 对话框

IMPLEMENT_DYNAMIC(CVideoConfDlg, CDialogEx)

CVideoConfDlg::CVideoConfDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVideoConfDlg::IDD, pParent)
	, m_Vexposure(0)
{
	InitializeCriticalSection(&m_crtSec);
	bOnTimeMsg=false;
	iExScale=10;
  m_width = 0;
  m_height = 0;
  m_bExType = false;
}

CVideoConfDlg::~CVideoConfDlg()
{
}

void CVideoConfDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DEV, m_device);
	DDX_Control(pDX, IDC_COMBO_BIN, m_bining);
	DDX_Control(pDX, IDC_SLIDER_EXP, m_exposure);
	DDX_Text(pDX, IDC_EDIT_EXP, m_Vexposure);
	DDX_Control(pDX, IDC_CHECK1, m_CheckAuto);
	DDX_Control(pDX, IDC_CHECK_IN, m_Check_Int);
	DDX_Control(pDX, IDC_CHECK_OUT, m_Check_Ext);
	DDX_Control(pDX, IDC_CHECK_SOFT, m_Check_Soft);
	DDX_Control(pDX, IDC_CHECK_TIME, m_Check_Timer);
}


BEGIN_MESSAGE_MAP(CVideoConfDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_DEV, &CVideoConfDlg::OnCbnSelchangeComboDev)
	ON_CBN_SELCHANGE(IDC_COMBO_BIN, &CVideoConfDlg::OnCbnSelchangeComboBin)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CVideoConfDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CVideoConfDlg::OnBnClickedButtonClose)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_IN, &CVideoConfDlg::OnBnClickedCheckIn)
	ON_BN_CLICKED(IDC_CHECK_OUT, &CVideoConfDlg::OnBnClickedCheckOut)
	ON_BN_CLICKED(IDC_CHECK_SOFT, &CVideoConfDlg::OnBnClickedCheckSoft)
	ON_BN_CLICKED(IDC_CHECK_TIME, &CVideoConfDlg::OnBnClickedCheckTime)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ADVANCED, &CVideoConfDlg::OnBnClickedAdvanced)
	ON_BN_CLICKED(IDC_TrigerBtn, &CVideoConfDlg::OnBnClickedTrigerbtn)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECKMirror, &CVideoConfDlg::OnBnClickedCheckmirror)
	ON_BN_CLICKED(IDC_CHECKFlip, &CVideoConfDlg::OnBnClickedCheckflip)
	ON_BN_CLICKED(IDC_CHECK1, &CVideoConfDlg::OnBnClickedCheck1)
END_MESSAGE_MAP()

bool CVideoConfDlg::InitPanel( LPVOID p_Param,LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;

	if (p_Param2 != 0)
		m_bExType = *((bool*)p_Param2);

	if (m_bExType&&p_Param2)
		m_VideoDevice = m_ConfocalCore->GetCurVideoEx();
	else
		m_VideoDevice = m_ConfocalCore->GetCurVideo();

	//m_VideoDevice=m_ConfocalCore->GetCurVideo();
	if(!m_VideoDevice) 
		return false;
	m_VideoDevice->Attach(this);
	m_device.AddString(m_VideoDevice->GetCameraName());	
	m_device.SetCurSel(0);
	int count=m_VideoDevice->GetBiningCount();
	for (int i=0;i<count;i++)
	{
		m_bining.AddString(m_VideoDevice->GetBining(i));
	}
	m_bining.SetCurSel(m_VideoDevice->GetBiningIndex());
	m_exposure.SetRange(0,300*iExScale);//设置曝光时间
	bRecMsg=true;
	SetTimer(539,100,NULL);

	bOpenEnable=true;
	bPauseEnable=true;
	mTrigerV=TRIGER_INTERAL;
	//GetDlgItem(IDC_CHECK1)->EnableWindow(m_VideoDevice->GetEnable(VIDEO_PID_AutoExp));
	GetDlgItem(IDC_ADVANCED)->EnableWindow(m_VideoDevice->GetEnable(VIDEO_PID_WhitBalance));
	return true;
}
// CVideoConfDlg 消息处理程序


void CVideoConfDlg::OnCbnSelchangeComboDev()
{
	if (m_VideoDevice)
		m_VideoDevice->OpenCamera();
}

void CVideoConfDlg::OnCbnSelchangeComboBin()
{
  if (m_VideoDevice)
  {
	  
    m_VideoDevice->SetBinning(m_bining.GetCurSel());//
    m_width = m_VideoDevice->GetImgHeader().Vwidth;
    m_height = m_VideoDevice->GetImgHeader().Vheight;
	
    Notify(this, NOTIFY_VIDEO_FORMAT_EXP, 0, &m_width, m_Vexposure, &m_height);
  }
}

void CVideoConfDlg::OnBnClickedButtonOpen()
{
	
	if(m_VideoDevice)
	{
		if (!m_VideoDevice->Run())
		{
			AfxMessageBox(L"启动失败！");
		}
    else
    {
      HVideoHeader video_hdr;
      video_hdr = m_VideoDevice->GetImgHeader();
      m_width = video_hdr.Vwidth;
      m_height = video_hdr.Vheight;
	  //video_hdr.VwBit *= 16;
	  
    }
	}
	else
		AfxMessageBox(L"没有相机！");

	UpdateData(FALSE);
}

void CVideoConfDlg::OnBnClickedButtonClose()
{
	if(m_VideoDevice)
		m_VideoDevice->Pause();
	//GetDlgItem(IDC_BUTTON_OPEN)->EnableWindow(true);
	//GetDlgItem(IDC_BUTTON_CLOSE)->EnableWindow(false);
}

void CVideoConfDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	EnterCriticalSection(&m_crtSec);
	bRecMsg=false;
	bOnTimeMsg=false;
	m_Vexposure=m_exposure.GetPos();
	m_Vexposure=m_Vexposure/iExScale;
	if(m_VideoDevice)
		m_VideoDevice->SetExposure((float)m_Vexposure);
	TRACE(L"OnHScroll Set%.1f\n",m_Vexposure);
	UpdateData(FALSE);
	LeaveCriticalSection(&m_crtSec);
  m_width = m_VideoDevice->GetImgHeader().Vwidth;
  m_height = m_VideoDevice->GetImgHeader().Vheight;
  Notify(this, NOTIFY_VIDEO_FORMAT_EXP, 0, &m_width, m_Vexposure, &m_height);
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CVideoConfDlg::OnBnClickedCheckIn()
{
	if(!m_VideoDevice)
		return;
	m_VideoDevice->SetTrigerMode(TRIGER_INTERAL);	
}

void CVideoConfDlg::OnBnClickedCheckOut()
{
	if(!m_VideoDevice)
		return;
	m_VideoDevice->SetTrigerMode(TRIGER_OUT);
}

void CVideoConfDlg::OnBnClickedCheckSoft()
{
	if(!m_VideoDevice)
		return;
	m_VideoDevice->SetTrigerMode(TRIGER_SOFT);
}


void CVideoConfDlg::OnBnClickedCheckTime()
{
}


HBRUSH CVideoConfDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	switch(pWnd->GetDlgCtrlID())
	{
	case IDC_STATIC:
		pDC->SetBkMode(TRANSPARENT);
		break;
	case IDC_SLIDER_EXP:
		pDC->SetBkMode(TRANSPARENT);
		break;
	}
	return afxGlobalData.brBarFace;
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


BOOL CVideoConfDlg::OnInitDialog()
{
	__super::OnInitDialog();
	// TODO:  在此添加额外的初始化
	this->SetWindowTextW(L"VideoControl");
	UpdateData(false);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}



void CVideoConfDlg::OnBnClickedAdvanced()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!m_VideoDevice)
		return;
	m_VideoDevice->SetValue(VIDEO_PID_GAIN,1);
	m_VideoDevice->AutoAWB(true);
}


void CVideoConfDlg::OnBnClickedTrigerbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!m_VideoDevice)
		return;
	m_VideoDevice->TrigerVideoData();
}

int CVideoConfDlg::OnSubjectNotified(IPtnSubject * pSubject, int ID,long wParam,
	void* pParam,float fParam,void* mParam)
{
	if (!bRecMsg)
	{
		bRecMsg=true;
		return 0;
	}
	switch(ID)
	{
		case Notify_CarlVideo_OpenCamera:
			bOpenEnable=true;
			bPauseEnable=false;
			break;
		case Notify_CarlVideo_CloseCamera:
			bOpenEnable=true;
			bPauseEnable=false;
			break;
		case Notify_CarlVideo_RunCamera:
			bOpenEnable=false;
			bPauseEnable=true;
			break;
		case Notify_CarlVideo_PauseCamera:
			bOpenEnable=true;
			bPauseEnable=false;
			break;
		case Notify_CarlVideo_ExpTime:
			m_Vexposure=m_VideoDevice->GetExposure();
			break;
		case Notify_CarlVideo_Triger:
			mTrigerV=*(TrigerType*)pParam;
			break;
		case Notify_CarlVideo_Flip:
			bVideoFlip=*(bool*)pParam;
			//
			break;
		case Notify_CarlVideo_Mirror:
			bVideoMirror=*(bool*)pParam;
			//((CButton*)GetDlgItem(IDC_CHECKMirror))->SetCheck(bR);
			break;
		default:
				break;
	}
	bOnTimeMsg=true;
	return 0;
}

void CVideoConfDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(nIDEvent==539)//刷新
	{
		if (!bOnTimeMsg)
		{
			//TRACE(L"OnTimer No %.1f\n",m_Vexposure);
			return;
		}
		bOnTimeMsg=false;
		EnterCriticalSection(&m_crtSec);
		m_exposure.SetPos(m_Vexposure*iExScale);
		UpdateData(false);
		switch(mTrigerV)
		{
			case TRIGER_INTERAL:
				m_Check_Ext.SetCheck(FALSE);
				m_Check_Int.SetCheck(true);
				m_Check_Timer.SetCheck(FALSE);
				m_Check_Soft.SetCheck(FALSE);
				break;
			case TRIGER_SOFT:
				m_Check_Ext.SetCheck(FALSE);
				m_Check_Int.SetCheck(FALSE);
				m_Check_Timer.SetCheck(FALSE);
				m_Check_Soft.SetCheck(true);
				break;
			case TRIGER_OUT:
				m_Check_Ext.SetCheck(true);
				m_Check_Int.SetCheck(FALSE);
				m_Check_Timer.SetCheck(FALSE);
				m_Check_Soft.SetCheck(FALSE);
				break;
		}
		GetDlgItem(IDC_BUTTON_OPEN)->EnableWindow(bOpenEnable);
		GetDlgItem(IDC_BUTTON_CLOSE)->EnableWindow(bPauseEnable);
		((CButton*)GetDlgItem(IDC_CHECKFlip))->SetCheck(bVideoFlip);
		((CButton*)GetDlgItem(IDC_CHECKMirror))->SetCheck(bVideoMirror);
		LeaveCriticalSection(&m_crtSec);
	}
	__super::OnTimer(nIDEvent);
}


void CVideoConfDlg::OnBnClickedCheckmirror()
{
	// TODO: 在此添加控件通知处理程序代码
	bRecMsg=false;
	bOnTimeMsg=false;
	if(!m_VideoDevice)
		return;
	bVideoMirror=((CButton*)GetDlgItem(IDC_CHECKMirror))->GetCheck();
	if (!m_VideoDevice->SetMirror(bVideoMirror))
		bVideoMirror=!bVideoMirror;
		//((CButton*)GetDlgItem(IDC_CHECKMirror))->SetCheck(!bVideoMirror);
}


void CVideoConfDlg::OnBnClickedCheckflip()
{
	// TODO: 在此添加控件通知处理程序代码
	bRecMsg=false;
	bOnTimeMsg=false;
	if(!m_VideoDevice)
		return;
	bVideoFlip=((CButton*)GetDlgItem(IDC_CHECKFlip))->GetCheck();
	if (!m_VideoDevice->SetFlip(bVideoFlip))
		bVideoFlip=!bVideoFlip;
		//((CButton*)GetDlgItem(IDC_CHECKFlip))->SetCheck(!bVideoFlip);
}


void CVideoConfDlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	//int iEx=m_exposure.GetPos();
	if (iExScale==10)
	{
		iExScale=100;
		//m_exposure.SetRange(0,300*iExScale);
		//m_exposure.SetPos(m_Vexposure*iExScale);
	}	
	else
	{
		iExScale=10;
		//m_exposure.SetPos(m_Vexposure*iExScale);
		//m_exposure.SetRange(0,300*iExScale);
	}	
	m_exposure.SetPos(0);
	m_exposure.SetRange(0,300*iExScale);
	m_exposure.SetPos(m_Vexposure*iExScale);
}
