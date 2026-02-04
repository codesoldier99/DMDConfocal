// DeviceConfigureDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DeviceConfigureDlg.h"
#include "afxdialogex.h"



// CDeviceConfigureDlg 对话框
//Openwrt---

IMPLEMENT_DYNAMIC(CDeviceConfigureDlg, CDialogEx)

CDeviceConfigureDlg::CDeviceConfigureDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDeviceConfigureDlg::IDD, pParent)
	, m_Vexposure(0)
{
	m_VideoDevice=0;
	m_VideoRender=0;
	//m_bFitWndows=false;
  m_timer = 0;
}

CDeviceConfigureDlg::~CDeviceConfigureDlg()
{
	if (m_VideoDevice)
		m_VideoDevice->CloseCamera();
}

void CDeviceConfigureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DEV, m_device);
	DDX_Control(pDX, IDC_COMBO_BIN, m_bining);
	DDX_Control(pDX, IDC_SLIDER_EXP, m_exposure);
	DDX_Text(pDX, IDC_EDIT_EXP, m_Vexposure);
	DDX_Control(pDX, IDC_CHECK1, m_CheckAuto);
}


BEGIN_MESSAGE_MAP(CDeviceConfigureDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_BIN, &CDeviceConfigureDlg::OnCbnSelchangeComboBin)
	ON_CBN_SELCHANGE(IDC_COMBO_DEV, &CDeviceConfigureDlg::OnCbnSelchangeComboDev)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CDeviceConfigureDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CDeviceConfigureDlg::OnBnClickedButtonClose)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_ADVANCED, &CDeviceConfigureDlg::OnBnClickedAdvanced)
	ON_BN_CLICKED(IDC_CHECK1, &CDeviceConfigureDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK_IN, &CDeviceConfigureDlg::OnBnClickedCheckIn)
	ON_BN_CLICKED(IDC_CHECK_SOFT, &CDeviceConfigureDlg::OnBnClickedCheckSoft)
	ON_BN_CLICKED(IDC_CHECK_TIME, &CDeviceConfigureDlg::OnBnClickedCheckTime)
	ON_BN_CLICKED(IDC_CHECK_OUT, &CDeviceConfigureDlg::OnBnClickedCheckOut)
	ON_BN_CLICKED(IDC_ResetGBtn, &CDeviceConfigureDlg::OnBnClickedResetgbtn)
	ON_BN_CLICKED(IDC_CloseBtn, &CDeviceConfigureDlg::OnBnClickedClosebtn)
	ON_BN_CLICKED(IDC_OpenBtn, &CDeviceConfigureDlg::OnBnClickedOpenbtn)
	ON_BN_CLICKED(IDC_Mirror, &CDeviceConfigureDlg::OnBnClickedMirror)
	ON_BN_CLICKED(IDC_Flip, &CDeviceConfigureDlg::OnBnClickedFlip)
  ON_BN_CLICKED(IDC_TRIGGER, &CDeviceConfigureDlg::OnBnClickedTrigger)
  ON_WM_TIMER()
END_MESSAGE_MAP()


void CDeviceConfigureDlg::SetVideoDevice(HVideoManager* pVideoDevice,HVideoRender* pRender)
{
	m_device.ResetContent();
	m_bining.ResetContent();
	m_VideoManager=pVideoDevice;
	m_VideoRender=pRender;
	if(!m_VideoManager)
		return;
	m_VideoManager->SearchCameras();
	if(m_VideoManager->GetCameraCount()<1)
		return;
	m_VideoDevice=m_VideoManager->OpenCamera(0);
	m_device.AddString(m_VideoDevice->GetCameraName());	
	m_device.SetCurSel(0);
	int count=m_VideoDevice->GetBiningCount();
	for (int i=0;i<count;i++)
	{
		m_bining.AddString(m_VideoDevice->GetBining(i));
	}
	m_bining.SetCurSel(m_VideoDevice->GetBiningIndex());
	m_exposure.SetRange(0,3000);//设置曝光时间
	m_Vexposure=m_VideoDevice->GetExposure();
	m_exposure.SetPos(m_Vexposure*10); 
	UpdateData(FALSE);
	m_VideoDevice->SetRender(m_VideoRender);
	GetDlgItem(IDC_CHECK1)->EnableWindow(m_VideoDevice->GetEnable(VIDEO_PID_AutoExp));
	GetDlgItem(IDC_ADVANCED)->EnableWindow(m_VideoDevice->GetEnable(VIDEO_PID_WhitBalance));
}

void CDeviceConfigureDlg::SetVideoDevice(HVideoManager* pVideoDevice,HVideoRender* pRender,CString strCamraName)
{
	m_device.ResetContent();
	m_bining.ResetContent();

	m_VideoManager=pVideoDevice;
	m_VideoRender=&m_RenderWnd;//pRender;
	if(!m_VideoManager)
		return;
	//m_VideoManager->SearchCameras();
	if(m_VideoManager->GetCameraCount()<1)
		return;
	int mIdx=0;
	for (int idx=0;idx<m_VideoManager->GetCameraCount();idx++)
	{
		if (m_VideoManager->GetCameraName(idx)==strCamraName)
		{
			mIdx=idx;
			break;
		}
	}
	m_VideoDevice=m_VideoManager->OpenCamera(mIdx);
	m_device.AddString(m_VideoDevice->GetCameraName());	
	m_device.SetCurSel(0);
	int count=m_VideoDevice->GetBiningCount();
	for (int i=0;i<count;i++)
	{
		m_bining.AddString(m_VideoDevice->GetBining(i));
	}
	m_bining.SetCurSel(m_VideoDevice->GetBiningIndex());
	m_exposure.SetRange(0,3000);//设置曝光时间
	m_Vexposure =  m_VideoDevice->GetExposure();
	m_exposure.SetPos(m_Vexposure*10);
	UpdateData(FALSE);
	m_VideoDevice->SetRender(m_VideoRender);
	GetDlgItem(IDC_CHECK1)->EnableWindow(m_VideoDevice->GetEnable(VIDEO_PID_AutoExp));
	GetDlgItem(IDC_ADVANCED)->EnableWindow(m_VideoDevice->GetEnable(VIDEO_PID_WhitBalance));

	SetWindowText(strCamraName);
}
// CDeviceConfigureDlg 消息处理程序

void CDeviceConfigureDlg::SetVideoDevice(HVideoManager* pVideoDevice,CString strCamraName)
{
	m_device.ResetContent();
	m_bining.ResetContent();

	m_VideoManager=pVideoDevice;
	m_VideoRender=&m_RenderWnd;
	if(!m_VideoManager)
		return;
	if(m_VideoManager->GetCameraCount()<1)
		return;
	int mIdx=0;
	for (int idx=0;idx<m_VideoManager->GetCameraCount();idx++)
	{
		if (m_VideoManager->GetCameraName(idx)==strCamraName)
		{
			mIdx=idx;
			break;
		}
	}
	m_VideoDevice=m_VideoManager->OpenCamera(mIdx);
	m_device.AddString(m_VideoDevice->GetCameraName());	
	m_device.SetCurSel(0);
	int count=m_VideoDevice->GetBiningCount();
	for (int i=0;i<count;i++)
	{
		m_bining.AddString(m_VideoDevice->GetBining(i));
	}
	m_bining.SetCurSel(m_VideoDevice->GetBiningIndex());
	m_exposure.SetRange(0,3000);//设置曝光时间
	m_Vexposure=m_VideoDevice->GetExposure();
	m_exposure.SetPos(m_Vexposure*10);
	UpdateData(FALSE);
	m_VideoDevice->SetRender(m_VideoRender);
	GetDlgItem(IDC_CHECK1)->EnableWindow(m_VideoDevice->GetEnable(VIDEO_PID_AutoExp));
	GetDlgItem(IDC_ADVANCED)->EnableWindow(m_VideoDevice->GetEnable(VIDEO_PID_WhitBalance));
	SetWindowText(strCamraName);
	strDeviceName=strCamraName;
}


void CDeviceConfigureDlg::OnCbnSelchangeComboBin()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_VideoDevice)
	{
		m_VideoDevice->SetBinning(m_bining.GetCurSel());
		//CString str;
		//m_bining.GetLBText(m_bining.GetCurSel(), str);
	}
}


void CDeviceConfigureDlg::OnCbnSelchangeComboDev()//这个换相机----
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_VideoManager)
	{
		if(m_VideoDevice)
			m_VideoDevice->CloseCamera();
		m_VideoDevice=m_VideoManager->OpenCamera(m_device.GetCurSel());
		m_VideoDevice->SetRender(m_VideoRender);
	}
}


void CDeviceConfigureDlg::OnBnClickedButtonOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	m_Vexposure = 10;
	m_Vexposure = m_Vexposure / 10;
	if(m_VideoDevice)
	{
		//设定初始曝光时间
		m_VideoDevice->SetExposure((float)m_Vexposure);
		m_exposure.SetPos(m_Vexposure);
		UpdateData(FALSE);

		m_VideoDevice->OpenCamera();
		m_VideoDevice->Run();

		//HVideoHeader video_hdr;
		//video_hdr = m_VideoDevice->GetImgHeader();
		//video_hdr.VwBit = 16;
		//UpdateData(FALSE);


	}		
}

void CDeviceConfigureDlg::OnBnClickedButtonClose()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_VideoDevice)
		m_VideoDevice->Pause();
}


void CDeviceConfigureDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_Vexposure=m_exposure.GetPos();
	m_Vexposure=m_Vexposure/10;

	HVideoHeader video_hdr;
	video_hdr = m_VideoDevice->GetImgHeader();


	if(m_VideoDevice)
		m_VideoDevice->SetExposure((float)m_Vexposure);
	UpdateData(FALSE);
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CDeviceConfigureDlg::OnBnClickedAdvanced()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!m_VideoDevice)
		return;
	m_VideoDevice->AutoAWB(true);
}


void CDeviceConfigureDlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_CheckAuto.GetCheck())
	{
		m_exposure.EnableWindow(FALSE);
		if(m_VideoDevice)
			m_VideoDevice->AutoAWB(true);
	}
	else
	{
		m_exposure.EnableWindow(TRUE);
		if(m_VideoDevice)
			m_VideoDevice->AutoAWB(false);
	}
}


void CDeviceConfigureDlg::OnBnClickedCheckIn()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!m_VideoDevice)
		return;
	m_VideoDevice->SetTrigerMode(TRIGER_INTERAL);
	((CButton*)GetDlgItem(IDC_CHECK_SOFT))->SetCheck(false);
	((CButton*)GetDlgItem(IDC_CHECK_OUT))->SetCheck(false);
	((CButton*)GetDlgItem(IDC_CHECK_TIME))->SetCheck(false);

}


void CDeviceConfigureDlg::OnBnClickedCheckSoft()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!m_VideoDevice)
		return;
	m_VideoDevice->SetTrigerMode(TRIGER_SOFT);
	((CButton*)GetDlgItem(IDC_CHECK_IN))->SetCheck(false);
	((CButton*)GetDlgItem(IDC_CHECK_OUT))->SetCheck(false);
	((CButton*)GetDlgItem(IDC_CHECK_TIME))->SetCheck(false);
}

void CDeviceConfigureDlg::OnBnClickedCheckOut()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!m_VideoDevice)
		return;
	m_VideoDevice->SetTrigerMode(TRIGER_OUT);
	((CButton*)GetDlgItem(IDC_CHECK_SOFT))->SetCheck(false);
	((CButton*)GetDlgItem(IDC_CHECK_IN))->SetCheck(false);
	((CButton*)GetDlgItem(IDC_CHECK_TIME))->SetCheck(false);
}

void CDeviceConfigureDlg::OnBnClickedCheckTime()
{
  if (m_timer > 0)
  {
    KillTimer(m_timer);
    m_timer = 0;
  }
  else
  {
    m_timer = SetTimer(1, 50, 0);
  }
  ((CButton*)GetDlgItem(IDC_CHECK_TIME))->SetCheck(m_timer);
}


void CDeviceConfigureDlg::OnBnClickedResetgbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!m_VideoDevice)
		return;
	m_VideoDevice->SetValue(VIDEO_PID_GAIN,1);
}


void CDeviceConfigureDlg::OnBnClickedClosebtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!m_VideoDevice)
		return;
	m_VideoDevice->CloseCamera();
}


void CDeviceConfigureDlg::OnBnClickedOpenbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!m_VideoDevice)
		return;
	m_VideoDevice->OpenCamera();
}


void CDeviceConfigureDlg::OnBnClickedMirror()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!m_VideoDevice)
		return;
	bool bMirror=((CButton*)GetDlgItem(IDC_Mirror))->GetCheck();
	if (!m_VideoDevice->SetMirror(bMirror))
		((CButton*)GetDlgItem(IDC_Mirror))->SetCheck(!bMirror);
}


void CDeviceConfigureDlg::OnBnClickedFlip()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!m_VideoDevice)
		return;
	bool bFlip=((CButton*)GetDlgItem(IDC_Flip))->GetCheck();
	if (!m_VideoDevice->SetFlip(bFlip))
		((CButton*)GetDlgItem(IDC_Flip))->SetCheck(!bFlip);
}


BOOL CDeviceConfigureDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CRect	rc;
	GetClientRect( &rc );
	m_RenderWnd.Create(NULL,L"test", WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE, rc, this, 0 );
	m_RenderWnd.SetWindowPos(NULL, 25, 5, 512, 512, SWP_NOZORDER );
	m_RenderWnd.ShowWindow(SW_SHOW);
	strDeviceName=L"";



	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CDeviceConfigureDlg::OnBnClickedTrigger()
{
  if (!m_VideoDevice)
    return;

  m_VideoDevice->TrigerVideoData();
}


void CDeviceConfigureDlg::OnTimer(UINT_PTR nIDEvent)
{
  if (nIDEvent == m_timer)
  {
    if (m_VideoDevice)
    {
      m_VideoDevice->TrigerVideoData();
    }
  }
  CDialogEx::OnTimer(nIDEvent);
}
