
// StageTesterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "StageTester.h"
#include "StageTesterDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CStageTesterDlg 对话框


CStageTesterDlg::CStageTesterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CStageTesterDlg::IDD, pParent)
	, m_XStep(0)
	, m_YStep(0)
	, m_ZStep(0)
	, m_XSpeed(0)
	, m_YSpeed(0)
	, m_ZSpeed(0)
	, m_TextField(_T(""))
	, iObjt(0)
	, miPIZPos(0.1)
	, iTurn(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_Gearbox=0;
	m_AxisX=0;
	m_AxisY=0;
	m_AxisZ=0;
	m_AxisW=0;
	m_AxisPIZ=0;
	m_AXISO=0;
	m_AXIST=0;
	iLight=0;

	m_bJoyStick=false;
	strStagName=L"";
}

void CStageTesterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_MesaageDis);
	DDX_Text(pDX, IDC_EDIT3, m_XStep);
	DDX_Text(pDX, IDC_EDIT5, m_YStep);
	DDX_Text(pDX, IDC_EDIT7, m_ZStep);
	DDX_Text(pDX, IDC_EDIT2, m_XSpeed);
	DDX_Text(pDX, IDC_EDIT4, m_YSpeed);
	DDX_Text(pDX, IDC_EDIT6, m_ZSpeed);
	DDX_Control(pDX, IDC_SLIDER1, m_XSlider);
	DDX_Control(pDX, IDC_SLIDER2, m_YSlider);
	DDX_Control(pDX, IDC_SLIDER3, m_ZSlider);
	DDX_Control(pDX, IDC_SLIDER4, m_XJoy);
	DDX_Control(pDX, IDC_SLIDER5, m_YJoy);
	DDX_Control(pDX, IDC_SLIDER6, m_ZJoy);
	DDX_Text(pDX, IDC_EDIT1, m_TextField);
	DDX_Control(pDX, IDC_COMBO1, mGearCoBox);
	DDX_Text(pDX, IDC_EDITIObjt, iObjt);
	DDX_Text(pDX, IDC_EDIT10, miPIZPos);
	DDX_Control(pDX, IDC_SLIDER7, mSliderLight);
	DDX_Text(pDX, IDC_EDITITurn, iTurn);
}

BEGIN_MESSAGE_MAP(CStageTesterDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CStageTesterDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CStageTesterDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON4, &CStageTesterDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CStageTesterDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CStageTesterDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CStageTesterDlg::OnBnClickedButton7)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CStageTesterDlg::OnNMCustomdrawSlider1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, &CStageTesterDlg::OnNMCustomdrawSlider2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, &CStageTesterDlg::OnNMCustomdrawSlider3)
	ON_BN_CLICKED(IDC_BUTTON9, &CStageTesterDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON10, &CStageTesterDlg::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON11, &CStageTesterDlg::OnBnClickedButton11)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON3, &CStageTesterDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON8, &CStageTesterDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON12, &CStageTesterDlg::OnBnClickedButton12)
	ON_BN_CLICKED(IDC_XStopBtn, &CStageTesterDlg::OnBnClickedXstopbtn)
	ON_BN_CLICKED(IDC_YStopBtn, &CStageTesterDlg::OnBnClickedYstopbtn)
	ON_BN_CLICKED(IDC_ZStopBtn, &CStageTesterDlg::OnBnClickedZstopbtn)
	ON_BN_CLICKED(IDC_InitBtn, &CStageTesterDlg::OnBnClickedInitbtn)
	ON_BN_CLICKED(IDC_ZBtn, &CStageTesterDlg::OnBnClickedZbtn)
	ON_BN_CLICKED(IDC_BUTTON14, &CStageTesterDlg::OnBnClickedButton14)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_EN_CHANGE(IDC_EDIT1, &CStageTesterDlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_ZResetBtn, &CStageTesterDlg::OnBnClickedZresetbtn)
	ON_BN_CLICKED(IDC_PIZDownBtn, &CStageTesterDlg::OnBnClickedPizdownbtn)
	ON_BN_CLICKED(IDC_PIZUpBtn, &CStageTesterDlg::OnBnClickedPizupbtn)
	ON_BN_CLICKED(IDC_PIZResetBtn, &CStageTesterDlg::OnBnClickedPizresetbtn)
	ON_BN_CLICKED(IDC_ReadLBtn, &CStageTesterDlg::OnBnClickedReadlbtn)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_ZBtn3, &CStageTesterDlg::OnBnClickedZbtn3)
	ON_BN_CLICKED(IDC_ZResetBtn2, &CStageTesterDlg::OnBnClickedZresetbtn2)
END_MESSAGE_MAP()


// CStageTesterDlg 消息处理程序

BOOL CStageTesterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	FindBox();
	mXPos=mYPos=mZPos=0.0f;
	SetTimer(529,30,NULL);

	((CButton*)GetDlgItem(IDC_RADIO2))->SetCheck(true);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CStageTesterDlg::FindBox()
{
	CFileFind finder;
	HMODULE t_mudule=GetModuleHandle(0);
	CString pfileName;
	GetModuleFileName(t_mudule,pfileName.GetBufferSetLength(MAX_PATH),MAX_PATH);
	pfileName.ReleaseBuffer();
	int nPos=pfileName.ReverseFind('\\');
	pfileName=pfileName.Left(nPos);
	pfileName+=L"\\*.stg";
	bool bWork=finder.FindFile(pfileName);//设置路径
	while(bWork)
	{
		bWork=finder.FindNextFileW();
		if(finder.IsDots())
			continue;//跳过
		CString t_moudulename=finder.GetFilePath();
		//切取出来库的名字
		int nPos=t_moudulename.ReverseFind('\\');
		CString strV=t_moudulename.Mid(nPos+1);//pPath.Right(nPos);
		nPos=strV.ReverseFind('.');
		strV=strV.Left(nPos);
		mGearCoBox.AddString(strV);
	}
}

bool CStageTesterDlg::OpenBox(CString mPluginName)
{
	RestPanelValue();//读值
	m_Gearbox=LoadDlls(mPluginName);//动态载入库
	if(!m_Gearbox)
		return FALSE;//没有找到，返回
	InitGearBox(m_Gearbox);//初始化它！！！

	if(m_AxisX)
		m_AxisX->GetSubject()->Attach(this);
	if(m_AxisY)
		m_AxisY->GetSubject()->Attach(this);
	if(m_AxisZ)
		m_AxisZ->GetSubject()->Attach(this);
	if (m_AxisW)
		m_AxisW->GetSubject()->Attach(this);
	if (m_AxisPIZ)
		m_AxisPIZ->GetSubject()->Attach(this);
	if (m_AXISO)
		m_AXISO->GetSubject()->Attach(this);
	if (m_AXIST)
		m_AXIST->GetSubject()->Attach(this);
	//else
		//return FALSE;
	return TRUE;
}

void CStageTesterDlg::RestPanelValue()
{
	m_XStep=m_conf.GetDouble(L"Panel",L"m_XStep",L"StageTester_"+strStagName);
	m_YStep=m_conf.GetDouble(L"Panel",L"m_YStep",L"StageTester_"+strStagName);
	m_ZStep=m_conf.GetDouble(L"Panel",L"m_ZStep",L"StageTester_"+strStagName);

	m_XSpeed=m_conf.GetDouble(L"Panel",L"m_XSpeed",L"StageTester_"+strStagName);
	m_YSpeed=m_conf.GetDouble(L"Panel",L"m_YSpeed",L"StageTester_"+strStagName);
	m_ZSpeed=m_conf.GetDouble(L"Panel",L"m_ZSpeed",L"StageTester_"+strStagName);

	m_XSlider.SetRange(0,1024);
	m_YSlider.SetRange(0,1024);
	m_ZSlider.SetRange(0,1024);

	m_XJoy.SetRange(0,2048);
	m_YJoy.SetRange(0,2048);
	m_ZJoy.SetRange(0,2048);

	UpdateData(FALSE);
}

void CStageTesterDlg::InitGearBox(HGearBox* pGearBox)
{	
	float x_mm2count=m_conf.GetDouble(L"Stage",L"x_mm2count",L"StageTester_"+strStagName);
	float y_mm2count=m_conf.GetDouble(L"Stage",L"y_mm2count",L"StageTester_"+strStagName);
	float z_mm2count=m_conf.GetDouble(L"Stage",L"z_mm2count",L"StageTester_"+strStagName);

	float x_mm2Encoder=m_conf.GetDouble(L"Stage",L"x_mm2Encoder",L"StageTester_"+strStagName);
	float y_mm2Encoder=m_conf.GetDouble(L"Stage",L"y_mm2Encoder",L"StageTester_"+strStagName);
	float z_mm2Encoder=m_conf.GetDouble(L"Stage",L"z_mm2Encoder",L"StageTester_"+strStagName);

	double x_limit=m_conf.GetDouble(L"Stage",L"x_limit",L"StageTester_"+strStagName);
	double y_limit=m_conf.GetDouble(L"Stage",L"y_limit",L"StageTester_"+strStagName);
	double z_limit=m_conf.GetDouble(L"Stage",L"z_limit",L"StageTester_"+strStagName);
	bool zInvert=m_conf.GetBool(L"Stage",L"z_Invert",L"StageTester_"+strStagName);
	bool yInvert=m_conf.GetBool(L"Stage",L"y_Invert",L"StageTester_"+strStagName);
	bool xInvert=m_conf.GetBool(L"Stage",L"x_Invert",L"StageTester_"+strStagName);

	bool zEncoderInvert=m_conf.GetBool(L"Stage",L"z_EncoderInvert",L"StageTester_"+strStagName);
	bool yEncoderInvert=m_conf.GetBool(L"Stage",L"y_EncoderInvert",L"StageTester_"+strStagName);
	bool xEncoderInvert=m_conf.GetBool(L"Stage",L"x_EncoderInvert",L"StageTester_"+strStagName);

	if (strStagName==L"HdsStage")
	{
		AxisInfo tInfo[5]={{AXIS_Z,'Z',0.0,z_limit,z_mm2count,z_mm2Encoder,m_ZSpeed,0,0,zInvert,zEncoderInvert,false},
				{AXIS_Y,'Y',0.0,y_limit,y_mm2count,y_mm2Encoder,m_YSpeed,0,0,yInvert,yEncoderInvert,false},//28lim
				{AXIS_X,'X',0.0,x_limit,x_mm2count,x_mm2Encoder,m_XSpeed,0,0,xInvert,xEncoderInvert,false},//85lim
				{AXIS_PIZ,'N',0.0,0,1000,1000,0,0,0,0,0,false},
				{AXIS_W,'W',0.0,10,1,1,500,0,0,true,true,false}};//目前W轴只有0~4 四个物镜位置！

		HdsStageParas mHdsParas;
		mHdsParas.strIP=m_conf.GetString(L"Stage",L"Ip",L"StageTester_"+strStagName);
		mHdsParas.iZCom=m_conf.GetInt(L"Stage",L"iZCom",L"StageTester_"+strStagName);
		mHdsParas.iWCom=m_conf.GetInt(L"Stage",L"iWCom",L"StageTester_"+strStagName);
		if (!pGearBox->InitGearBox(L"Carl's GearBox",&mHdsParas,tInfo,4))
		{
			AfxMessageBox(L"初始化轴失败！");
			return;
		}
	}
	else if (strStagName==L"MoticStage")
	{
		bool bCheck=((CButton*)GetDlgItem(IDC_RADIO1))->GetCheck();
		int iType=8,iLimit=999;
		if (bCheck)
		{
			iLimit=99;
			iType=6;
		}
		mSliderLight.SetPos(0);
		mSliderLight.SetRange(0,iLimit);

		AxisInfo tInfo[7]={{AXIS_Z,'Z',0.0,z_limit,z_mm2count,z_mm2Encoder,m_ZSpeed,0,0,zInvert,zEncoderInvert,false},
			{AXIS_Y,'Y',0.0,y_limit,y_mm2count,y_mm2Encoder,m_YSpeed,0,0,yInvert,yEncoderInvert,false},//28lim
			{AXIS_X,'X',0.0,x_limit,x_mm2count,x_mm2Encoder,m_XSpeed,0,0,xInvert,xEncoderInvert,false},//85lim
			{AXIS_PIZ,'N',0.0,0,1000,1000,0,0,0,0,0,false},
			{AXIS_O,'O',0.0,iLimit,0,0,0,0,iType,true,true,false},
			{AXIS_T,'T',0.0,10,1,1,500,0,0,true,true,false},
			{AXIS_W,'W',0.0,10,1,1,500,0,0,true,true,false}};//目前W轴只有0~4 四个物镜位置！	
		
		//int mCom=m_conf.GetInt(L"Stage",L"Port",L"StageTester_"+strStagName);
		MoticStageParas mParas;
		mParas.iAxisCom=m_conf.GetInt(L"Stage",L"Port",L"StageTester_"+strStagName);
		mParas.iTurnCom=m_conf.GetInt(L"Stage",L"TPort",L"StageTester_"+strStagName);
		if (!pGearBox->InitGearBox(L"Carl's GearBox",&mParas,tInfo,7))
		{
			AfxMessageBox(L"初始化轴失败！");
			return;
		}
	}
	
	if(!m_AxisX)
		m_AxisX=pGearBox->GetAxis(AXIS_X);
	if(!m_AxisY)
		m_AxisY=pGearBox->GetAxis(AXIS_Y);
	if(!m_AxisZ)
		m_AxisZ=pGearBox->GetAxis(AXIS_Z);
	if (!m_AxisW)
		m_AxisW=pGearBox->GetAxis(AXIS_W);
	if (!m_AxisPIZ)
		m_AxisPIZ=pGearBox->GetAxis(AXIS_PIZ);
	if (!m_AXISO)
		m_AXISO=pGearBox->GetAxis(AXIS_O);
	if (!m_AXIST)
		m_AXIST=pGearBox->GetAxis(AXIS_T);
}

HGearBox* CStageTesterDlg::LoadDlls(CString t_name)
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

	if(!t_GetInterface)
		return 0;
	CString tname=t_GetName();
	TRACE(tname);
	HGearBox* t;
	t=(HGearBox*)t_GetInterface();
	return t;
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CStageTesterDlg::OnPaint()
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
HCURSOR CStageTesterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CString CStageTesterDlg::GetMoveStr(HAxis* mAxis,float fVStep)
{
	CString strVS;
	strVS.Format(L"Move Count To:%.0f\r\n",mAxis->GetCount());
	return strVS;
}



void CStageTesterDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if(m_AxisX)
		m_AxisX->MoveRef(m_XStep,m_XSpeed);
	m_TextField+=L"\r\nX "+GetMoveStr(m_AxisX,m_XStep);
}


void CStageTesterDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if(m_AxisX)
		m_AxisX->MoveRef(-m_XStep,m_XSpeed);
	m_TextField+=L"\r\nX "+GetMoveStr(m_AxisX,-m_XStep);
}


void CStageTesterDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if(m_AxisY)
		m_AxisY->MoveRef(m_YStep,m_YSpeed);
	m_TextField+=L"\r\nY "+GetMoveStr(m_AxisY,m_YStep);
}


void CStageTesterDlg::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if(m_AxisY)
		m_AxisY->MoveRef(-m_YStep,m_YSpeed);
	m_TextField+=L"\r\nY "+GetMoveStr(m_AxisY,-m_YStep);
}


void CStageTesterDlg::OnBnClickedButton6()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if(m_AxisZ)
		m_AxisZ->MoveRef(m_ZStep,m_ZSpeed);
	m_TextField+=L"\r\nZ "+GetMoveStr(m_AxisZ,m_ZStep);
}


void CStageTesterDlg::OnBnClickedButton7()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if(m_AxisZ)
		m_AxisZ->MoveRef(-m_ZStep,m_ZSpeed);
	m_TextField+=L"\r\nZ "+GetMoveStr(m_AxisZ,-m_ZStep);
}


void CStageTesterDlg::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}


void CStageTesterDlg::OnNMCustomdrawSlider2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}


void CStageTesterDlg::OnNMCustomdrawSlider3(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}


void CStageTesterDlg::OnBnClickedButton9()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if(m_AxisX)
		m_AxisX->Rest();
}


void CStageTesterDlg::OnBnClickedButton10()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if(m_AxisY)
		m_AxisY->Rest();
}


void CStageTesterDlg::OnBnClickedButton11()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if(m_AxisZ)
		m_AxisZ->Rest();
}


void CStageTesterDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (strStagName!=L"")
	{
		m_conf.RecValue(L"Panel",L"m_XStep",m_XStep,L"StageTester_"+strStagName);
		m_conf.RecValue(L"Panel",L"m_YStep",m_YStep,L"StageTester_"+strStagName);
		m_conf.RecValue(L"Panel",L"m_ZStep",m_ZStep,L"StageTester_"+strStagName);
		m_conf.RecValue(L"Panel",L"m_XSpeed",m_XSpeed,L"StageTester_"+strStagName);
		m_conf.RecValue(L"Panel",L"m_YSpeed",m_YSpeed,L"StageTester_"+strStagName);
		m_conf.RecValue(L"Panel",L"m_ZSpeed",m_ZSpeed,L"StageTester_"+strStagName);
	}
	CDialogEx::OnClose();
}
//回复的消息
int CStageTesterDlg::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	CString strV;
	if(m_AxisX)
	{
		if(pSubject==m_AxisX->GetSubject())
		{
			strV.Format(L"AixsX Arrived Count:%.0f\r\n",m_AxisX->GetEncorder());
			if(ID==NOTIFY_AXIS_MOVED)
				m_TextField+=L"AixsX  _Moved\r\n";
			else if(ID==NOTIFY_AXIS_POSITION)
				m_TextField+=L"AixsX  _POSITION\r\n";
			else if(ID==NOTIFY_AXIS_RESTED)
				m_TextField+=L"AixsX  _RESTED\r\n";
			else if(ID==NOTIFY_AXIS_ENCORDER)
				m_TextField+=L"AixsX  _ENCORDER\r\n";
			m_TextField+=strV;
			mXPos=*((float*)pParam);
		}
	}
	if(m_AxisY)
	{
		if (pSubject==m_AxisY->GetSubject())
		{
			strV.Format(L"AixsY Arrived Count:%.0f\r\n",m_AxisY->GetEncorder());
			if(ID==NOTIFY_AXIS_MOVED)
				m_TextField+=L"AixsY  _Moved\r\n";
			else if(ID==NOTIFY_AXIS_POSITION)
				m_TextField+=L"AixsY  _POSITION\r\n";
			else if(ID==NOTIFY_AXIS_RESTED)
				m_TextField+=L"AixsY  _RESTED\r\n";
			else if(ID==NOTIFY_AXIS_ENCORDER)
				m_TextField+=L"AixsY  _ENCORDER\r\n";
			m_TextField+=strV;
			mYPos=*((float*)pParam);
		}
	}
	if(m_AxisZ)
	{
		if (pSubject==m_AxisZ->GetSubject())
		{
			strV.Format(L"AixsZ Arrived Count:%.0f\r\n",m_AxisZ->GetEncorder());
			if(ID==NOTIFY_AXIS_MOVED)
				m_TextField+=L"AixsZ  _Moved\r\n";
			else if(ID==NOTIFY_AXIS_POSITION)
				m_TextField+=L"AixsZ  _POSITION\r\n";
			else if(ID==NOTIFY_AXIS_RESTED)
				m_TextField+=L"AixsZ  _RESTED\r\n";
			else if(ID==NOTIFY_AXIS_ENCORDER)
				m_TextField+=L"AixsZ  _ENCORDER\r\n";
			m_TextField+=strV;
			mZPos=*((float*)pParam);
		}
	}
	if (m_AXISO)
	{
		if (pSubject==m_AXISO->GetSubject())
			iLight=wParam;
		mSliderLight.SetPos(iLight);
	}

	if (ID==NOTIFY_AXIST_CHANGING)
	{
		strV.Format(L"色轮转换到%d\r\n",wParam);
		m_TextField+=strV;
		//UpdateData();
	}

	strV.Format(L"X:%0.3fmm,Y:%0.3fmm,Z:%0.3fmm,Light:%d",mXPos,mYPos,mZPos,iLight);
	SetDlgItemText(IDC_EDITPosition,strV);
	m_MsgIn=true;//更新消息
	return 1;
}


void CStageTesterDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if(nIDEvent==529)
	{
		if(m_MsgIn)
		{
			UpdateData(FALSE);
			m_MsgIn=false;
		}
	}
	__super::OnTimer(nIDEvent);
}


void CStageTesterDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	m_TextField=L"";
	m_MsgIn=true;
}


void CStageTesterDlg::OnBnClickedButton8()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CStageTesterDlg::OnBnClickedButton12()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CStageTesterDlg::OnBnClickedXstopbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_AxisX)
		m_AxisX->Stop();
}


void CStageTesterDlg::OnBnClickedYstopbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_AxisY)
		m_AxisY->Stop();
}


void CStageTesterDlg::OnBnClickedZstopbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_AxisZ)
		m_AxisZ->Stop();
}


void CStageTesterDlg::OnBnClickedInitbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	strStagName=L"";
	GetDlgItemText(IDC_COMBO1,strStagName);
	if (strStagName!=L"")
	{
		if (!OpenBox(strStagName+L".stg"))
			AfxMessageBox(L"Open the Box failed!");
	}
}


void CStageTesterDlg::OnBnClickedZbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	//if (iObjt>=0)
	//{
		if (m_AxisW)
		{
			m_AxisW->MoveTo(iObjt);
		}
		else
			AfxMessageBox(L"W轴不存在！");
	//}
	//else
		//AfxMessageBox(L"请输入大于零的位置！");
}

void CStageTesterDlg::OnBnClickedZresetbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_AxisW)
	{
		m_AxisW->Rest();
	}
}


void CStageTesterDlg::OnBnClickedButton14()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bJoyStick=!m_bJoyStick;
	if(m_bJoyStick)//使能
	{
		GetCursorPos(&m_posPoint);//当前坐标
		//获取轴
		double x_limit=m_conf.GetDouble(L"Stage",L"x_limit",L"StageTester_"+strStagName);
		double y_limit=m_conf.GetDouble(L"Stage",L"y_limit",L"StageTester_"+strStagName);
		CRect t; 
		this->GetWindowRect(&t);
		int x=t.Width();
		int y=t.Height();
		scalex=(double)75/(double)x;
		scaley=(double)25/(double)y;
	}
}


BOOL CStageTesterDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if(pMsg->message==WM_MOUSEWHEEL)
	{
		//TRACE("Mouse Wheel%d\n",pMsg->wParam);

		if(pMsg->wParam>0)
		{
			if(m_AxisZ)
				m_AxisZ->MoveRef(m_ZStep,m_ZSpeed);
			
		}
		else
		{
			if(m_AxisZ)
				m_AxisZ->MoveRef(-1*m_ZStep,m_ZSpeed);
					
		}
	}

	if(pMsg->message==WM_MOUSEMOVE)
	{
		//if(m_bJoyStick)
			//TRACE("MOUSE Move%d,%d,%d\n",pMsg->pt.x,pMsg->pt.y,pMsg->wParam);
		//TRACE("MOUSE Move%d,%d\n",point.x,point.y);
	}

	if(m_bJoyStick)
	{
		//SetCursorPos(m_posPoint.x,m_posPoint.y);//设为原点
	}

	return __super::PreTranslateMessage(pMsg);
}


void CStageTesterDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

/*	if(m_bJoyStick)
	{
		SetCursorPos(m_posPoint.x,m_posPoint.y);//设为原点


		TRACE("MOUSE Move%d,%d\n",point.x,point.y);

	}
*/
	if(m_bJoyStick)
	{
		CPoint t=point;//-m_posPoint;
		m_posPoint=point;
		double t_posx,t_posy;
		if(m_AxisX)
		{
			t_posx=(double)t.x*scalex;
			//m_AxisX->GetPosition();
			if(!m_AxisX->IsMoveing())
				m_AxisX->MoveTo(t_posx,m_XSpeed);
			
		}
		if(m_AxisY)
		{
			t_posy=(double)t.y*scaley;
			if(!m_AxisY->IsMoveing())
				m_AxisY->MoveTo(t_posy,m_YSpeed);
		}
		TRACE(L"pos:%d,%d---",t.x,t.y);
		TRACE(L"x:%.3f,%.3f\n",t_posx,t_posy);

	}
	__super::OnMouseMove(nFlags, point);
}


BOOL CStageTesterDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return __super::OnMouseWheel(nFlags, zDelta, pt);
}


void CStageTesterDlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

}





void CStageTesterDlg::OnBnClickedPizdownbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if(m_AxisPIZ)
		m_AxisPIZ->MoveRef(miPIZPos);
}


void CStageTesterDlg::OnBnClickedPizupbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if(m_AxisPIZ)
		m_AxisPIZ->MoveRef(-miPIZPos);
}


void CStageTesterDlg::OnBnClickedPizresetbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_AxisPIZ)
		m_AxisPIZ->MoveTo(0);
}


void CStageTesterDlg::OnBnClickedReadlbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_AXISO)
		m_AXISO->GetPosition();
}


void CStageTesterDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int iV=mSliderLight.GetPos();
	if (m_AXISO)
		m_AXISO->MoveTo(iV);
	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CStageTesterDlg::OnBnClickedZbtn3()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_AXIST)
	{
		m_AXIST->MoveTo(iTurn);
	}
	else
		AfxMessageBox(L"T轴不存在！");
}

void CStageTesterDlg::OnBnClickedZresetbtn2()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_AXIST)
	{
		m_AXIST->Rest();
	}
	else
		AfxMessageBox(L"T轴不存在！");
}
