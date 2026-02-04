
// VideoTesterDlg.cpp : 实现文件
//
#include "stdafx.h"
#include "VideoTester.h"
#include "VideoTesterDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVideoTesterDlg 对话框
CVideoTesterDlg::CVideoTesterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVideoTesterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_VideoDevice=0;
	m_VideoManager=0;
}

void CVideoTesterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBOType, mCoCameraTypes);
	DDX_Control(pDX, IDC_COMBOCamera, mCoCameraName);
}

BEGIN_MESSAGE_MAP(CVideoTesterDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CAM, &CVideoTesterDlg::OnBnClickedBtnCam)
	ON_BN_CLICKED(IDC_BTN_GRAB, &CVideoTesterDlg::OnBnClickedBtnGrab)
	ON_BN_CLICKED(IDC_BTN_TRIGER, &CVideoTesterDlg::OnBnClickedBtnTriger)
	ON_CBN_SELCHANGE(IDC_COMBOType, &CVideoTesterDlg::OnCbnSelchangeCombotype)
	ON_BN_CLICKED(IDC_BTNStart, &CVideoTesterDlg::OnBnClickedBtnstart)
	ON_CBN_SELCHANGE(IDC_COMBOCamera, &CVideoTesterDlg::OnCbnSelchangeCombocamera)
	ON_BN_CLICKED(IDC_CloseCameraBtn, &CVideoTesterDlg::OnBnClickedClosecamerabtn)
	ON_BN_CLICKED(IDC_BTNCapture, &CVideoTesterDlg::OnBnClickedBtncapture)
	ON_BN_CLICKED(IDC_CreateBtn, &CVideoTesterDlg::OnBnClickedCreatebtn)
  ON_WM_DESTROY()
END_MESSAGE_MAP()

// CVideoTesterDlg 消息处理程序

BOOL CVideoTesterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_DeviceConf.Create(CDeviceConfigureDlg::IDD);

	//CRect	rc;
	//GetClientRect( &rc );
	//m_renderwnd.Create(NULL,L"test", WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE, rc, this, 0 );
	//m_renderwnd.SetWindowPos(NULL, 0, 0, 512, 512, SWP_NOZORDER );
	//m_renderwnd.ShowWindow(SW_SHOW);
	
	mVideoType=LoadDlls(L"VideoPlugin.dll");//加入库
	if(mVideoType==0)
	{
		AfxMessageBox(L"没有发现相机插件！");
		return TRUE;//返回
	}
	int iCount=mVideoType->FindCameraType();//查找了所有相机类型
	for (int i=0;i<iCount;i++)
	{
		mCoCameraTypes.AddString(mVideoType->GetCameraType(i)->strCameraType);
		mVideoType->GetCameraType(i)->mVideoManager->Attach(this);
	}
	if (mCoCameraTypes.GetCount()>0)
		mCoCameraTypes.SetCurSel(0);

	/*	for (int i=0;i<10;i++)
	{
	mDeviceNewList[i]=NULL;
	}*/	
	miCameraCreateCount=0;


	//-----自动识别当前连接的设备名称
	int iSelected = mCoCameraTypes.GetCurSel();
	if (iSelected >= 0 && mVideoType->GetTypeCount()>iSelected)
		m_VideoManager = mVideoType->GetCameraType(iSelected)->mVideoManager;
	if (m_VideoManager)
	{
		mCoCameraName.ResetContent();
		for (int i = 0; i<m_VideoManager->GetCameraCount(); i++)
		{
			mCoCameraName.AddString(m_VideoManager->GetCameraName(i));
		}
	}
	if (mCoCameraName.GetCount()>0)
		mCoCameraName.SetCurSel(0);


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
HVideoType* CVideoTesterDlg::LoadDlls(CString t_name)
{
	HMODULE t_mudule=GetModuleHandle(0);
	CString pfileName;
	GetModuleFileName(t_mudule,pfileName.GetBufferSetLength(MAX_PATH),MAX_PATH);
	pfileName.ReleaseBuffer();
	int nPos=pfileName.ReverseFind('\\');
	pfileName=pfileName.Left(nPos);
	pfileName+=L"\\";
	pfileName+=t_name;
	HMODULE t_module;
	t_module=::LoadLibrary(pfileName);
	if(t_module==NULL)
		return 0;
	PluginLib_GetInterface t_GetInterface;
	PluginLib_GetName		t_GetName;
	t_GetInterface=(PluginLib_GetInterface)::GetProcAddress(t_module,"HGetPluginInterface");
	t_GetName=(PluginLib_GetName)::GetProcAddress(t_module,"HGetPluginName");
	CString tname=t_GetName();
	TRACE(tname);
	HVideoType* t;
	t=(HVideoType*)t_GetInterface();
	return t;
}

void CVideoTesterDlg::OnCbnSelchangeCombotype()
{
	// TODO: 在此添加控件通知处理程序代码
	//-----识别选择连接的设备名称
	int iSelected=mCoCameraTypes.GetCurSel();
	if (iSelected>=0&&mVideoType->GetTypeCount()>iSelected)
		m_VideoManager=mVideoType->GetCameraType(iSelected)->mVideoManager;
	if (m_VideoManager)
	{
		mCoCameraName.ResetContent();
		for (int i=0;i<m_VideoManager->GetCameraCount();i++)
		{
			mCoCameraName.AddString(m_VideoManager->GetCameraName(i));
		}
	}
	if (mCoCameraName.GetCount()>0)
		mCoCameraName.SetCurSel(0);
}

void CVideoTesterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CVideoTesterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVideoTesterDlg::OnBnClickedBtnCam()
{
	// TODO: 在此添加控件通知处理程序代码
	m_DeviceConf.ShowWindow(SW_SHOW);
}


void CVideoTesterDlg::OnBnClickedBtnGrab()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CVideoTesterDlg::OnBnClickedBtnTriger()
{
	// TODO: 在此添加控件通知处理程序代码
	HVideoDevice* tdevice;
	tdevice=m_DeviceConf.GetDevice();
	//tdevice->SetTrigerMode(TRIGER_SOFT);
	if(tdevice)
		tdevice->TrigerVideoData();
}



void CVideoTesterDlg::OnBnClickedBtnstart()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_VideoManager)
	{
		//m_DeviceConf.SetVideoDevice(m_VideoManager,&m_renderwnd);//设置设备
		CString str;
		mCoCameraName.GetLBText(mCoCameraName.GetCurSel(),str);
		//m_DeviceConf.SetVideoDevice(m_VideoManager,&m_renderwnd,str);
		//for (int i=0;i<m_VideoManager->GetCameraCount();i++)
		//	m_VideoManager->GetCamerDevice(i)->Attach(this);
	}		
}

int CVideoTesterDlg::OnSubjectNotified(IPtnSubject * pSubject, int ID,long wParam,
	void* pParam,float fParam,void* mParam)
{
	CString str=L"Status: ";
	TrigerType* mV=0;
	switch(ID)
	{
		case Notify_CarlVideo_OpenCamera:
			str+=*(CString*)pParam+L" Open";
			break;
		case Notify_CarlVideo_CloseCamera:
			str+=*(CString*)pParam+L" Close";
			break;
		case Notify_CarlVideo_RunCamera:
			str+=*(CString*)pParam+L" Run";
			break;
		case Notify_CarlVideo_PauseCamera:
			str+=*(CString*)pParam+L" Pause";
			break;
		case Notify_CarlVideo_ExpTime:
			//str+=*(CString*)pParam+L" Pause";
			str.Format(L"Set ExportTime:%.1f",fParam);
			break;
		case Notify_CarlVideo_Triger:
			 mV=(TrigerType*)pParam;
			 if (!mV)
				break;
			switch(*mV)
			{
				case TRIGER_INTERAL:
					str=L"TRIGER_INTERAL";
					break;
				case TRIGER_SOFT:
					str=L"TRIGER_SOFT";
					break;
				case TRIGER_OUT:
					str=L"TRIGER_OUT";
					break;
			}
			break;
		default:
			break;
	}
	SetDlgItemText(IDC_EDITStatus,str);
	return 0;
}


void CVideoTesterDlg::OnCbnSelchangeCombocamera()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CVideoTesterDlg::OnBnClickedClosecamerabtn()
{
	// TODO: 在此添加控件通知处理程序代码
	int iAll=mVideoType->GetTypeCount();
	for (int i=0;i<iAll;i++)
	{
		int iManager=mVideoType->GetCameraType(i)->mVideoManager->GetCameraCount();
		for (int j=0;j<iManager;j++)
		{
			mVideoType->GetCameraType(i)->mVideoManager->CloseCamera(j);
		}
	}
}

void CVideoTesterDlg::OnBnClickedBtncapture()
{
	// TODO: 在此添加控件通知处理程序代码
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
		//m_renderwnd.Savebmp(fileDlg.GetPathName().GetBuffer());
		//HSaveBmp((LPBYTE)m_Buffer,m_bmi->bmiHeader.biWidth,
			//m_bmi->bmiHeader.biHeight,m_bmi->bmiHeader.biBitCount,fileDlg.GetPathName().GetBuffer());
	}
}

void CVideoTesterDlg::OnBnClickedCreatebtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CString str;
	mCoCameraName.GetLBText(mCoCameraName.GetCurSel(),str);
	for (int i=0;i<10;i++)
	{
		if (mDeviceNewList[i].strDeviceName==str)
		{
			mDeviceNewList[i].ShowWindow(SW_SHOW);
			return;
		}
	}
	if (m_VideoManager)
	{
		mDeviceNewList[miCameraCreateCount].Create(CDeviceConfigureDlg::IDD);	
		mDeviceNewList[miCameraCreateCount].SetVideoDevice(m_VideoManager,str);
		mDeviceNewList[miCameraCreateCount].ShowWindow(SW_SHOW);
		miCameraCreateCount++;
	}		
}


void CVideoTesterDlg::OnDestroy()
{
  OnBnClickedClosecamerabtn();

  __super::OnDestroy();
}
