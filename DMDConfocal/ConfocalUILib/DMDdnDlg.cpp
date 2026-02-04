// DMDdnDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "DMDdnDlg.h"
#include "afxdialogex.h"


// CDMDdnDlg 对话框

IMPLEMENT_DYNAMIC(CDMDdnDlg, CDialogEx)

CDMDdnDlg::CDMDdnDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDMDdnDlg::IDD, pParent)
	, m_PtSize(1)
	, m_PtPeriod(10)
	, m_LineSize(64)
	, m_LinePeriod(1)
	, m_FramRate(3800)
	, mBlackTime(150)
	, miPicCount(0)
	, m_iBackGray(0)
	, m_PtEx(1)
	, m_LineEx(64)
	, m_trigger_delay(0)
	, m_trigger_pw(20)
	, _str_sl_path(_T(".\\SLImages"))
{
	m_DmdManager=0;
	mF = 0;
	m_FramRate=m_ConfFile.GetInt(L"DMDParas",L"m_FrameMargin",L"Confocal_DMDControl");//Active
	mBlackTime=m_ConfFile.GetInt(L"DMDParas",L"m_FrameInterval",L"Confocal_DMDControl");//Black
	m_trigger_delay = m_ConfFile.GetInt(L"DMDParas", L"delay", L"Confocal_DMDControl");//Delay
	m_trigger_pw = m_ConfFile.GetInt(L"DMDParas", L"plusewidth", L"Confocal_DMDControl");//PW

	m_LineSize =m_ConfFile.GetInt(L"DMDParas",L"m_LineSize",L"Confocal_DMDControl");
	m_PtSize =m_ConfFile.GetInt(L"DMDParas",L"m_PtSize",L"Confocal_DMDControl");
	m_PtPeriod =m_ConfFile.GetInt(L"DMDParas",L"m_PtPeriod",L"Confocal_DMDControl");
	m_PtEx = m_ConfFile.GetInt(L"DMDParas", L"m_PtEx", L"Confocal_DMDControl");
	m_LinePeriod =m_ConfFile.GetInt(L"DMDParas",L"m_LinePeriod",L"Confocal_DMDControl");
	m_LineEx = m_ConfFile.GetInt(L"DMDParas", L"m_LineEx", L"Confocal_DMDControl");

	miPicCount=m_ConfFile.GetInt(L"DMDParas",L"MergeCount",L"Confocal_DMDControl");

	mDMDParas.m_FrameInterval=m_ConfFile.GetInt(L"DMDParas",L"m_FrameInterval",L"Confocal_DMDControl");
	mDMDParas.m_FrameMargin=m_ConfFile.GetInt(L"DMDParas",L"m_FrameMargin",L"Confocal_DMDControl");
	mDMDParas.gray=m_ConfFile.GetInt(L"DMDParas",L"gray",L"Confocal_DMDControl");
	mDMDParas.delay=m_ConfFile.GetInt(L"DMDParas",L"delay",L"Confocal_DMDControl");
	mDMDParas.plusewidth=m_ConfFile.GetInt(L"DMDParas",L"plusewidth",L"Confocal_DMDControl");
	mDMDParas.polay = false;
	mDMDParas.trigeMode=0x00000006;
	mDMDParas.trigeSync=0x00000008;

	 m_start = false;
	 m_Open = false;
	_si_frames = 0;
	_str_sl_path = HGetStartPath() + L"\\SLImages";

	strShow = L"";
	strShowEx = L"";
};

CDMDdnDlg::~CDMDdnDlg()
{
}

void CDMDdnDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_WIDTH2, m_PtSize);
	DDX_Text(pDX, IDC_WIDTH4, m_PtPeriod);
	DDX_Text(pDX, IDC_WIDTH, m_LineSize);
	DDX_Text(pDX, IDC_WIDTH5, m_LinePeriod);
	DDX_Text(pDX, IDC_WIDTH3, m_FramRate);
	//DDV_MinMaxInt(pDX, m_FramRate, 0, 40000);
	//DDV_MinMaxInt(pDX, m_PtSize, 1, 20);
	DDX_Text(pDX, IDC_WIDTH6, mBlackTime);
	DDX_Control(pDX, IDC_COMBO1, comboGray);
	DDX_Text(pDX, IDC_ConfocalCount, miPicCount);
	DDX_Text(pDX, IDC_BackGrayTxt, m_iBackGray);
	DDX_Text(pDX, IDC_EXTRA_PERIOD, m_PtEx);
	DDX_Text(pDX, IDC_EXTRA_SPACE, m_LineEx);
	DDX_Text(pDX, IDC_EDIT_TRIGGER_DELAY, m_trigger_delay);
	DDX_Text(pDX, IDC_EDIT_TRIGGER_PW, m_trigger_pw);
	DDX_Control(pDX, IDC_COMBO2, m_trigger_mode);
	DDX_Control(pDX, IDC_SL_PATH, _sl_path);
	DDX_Text(pDX, IDC_SL_PATH, _str_sl_path);
	DDX_Control(pDX, IDC_COMBODMDType, comboDMDType);
}

BEGIN_MESSAGE_MAP(CDMDdnDlg, CDialogEx)
	ON_BN_CLICKED(IDC_WhiteDMDBtn, &CDMDdnDlg::OnBnClickedWhitedmdbtn)
	ON_BN_CLICKED(IDC_BlackDMDBtn, &CDMDdnDlg::OnBnClickedBlackdmdbtn)
	ON_BN_CLICKED(IDC_PicBtn, &CDMDdnDlg::OnBnClickedPicbtn)
	ON_BN_CLICKED(IDC_ResetBtn, &CDMDdnDlg::OnBnClickedResetbtn)
	ON_BN_CLICKED(IDC_StopBtn, &CDMDdnDlg::OnBnClickedStopbtn)
	ON_BN_CLICKED(IDC_StartDMDBtn, &CDMDdnDlg::OnBnClickedStartdmdbtn)
	ON_BN_CLICKED(IDC_UpdateFrame, &CDMDdnDlg::OnBnClickedUpdateframe)
	ON_BN_CLICKED(IDC_PointScan, &CDMDdnDlg::OnBnClickedPointscan)
	ON_BN_CLICKED(IDC_HR, &CDMDdnDlg::OnBnClickedHr)
	ON_BN_CLICKED(IDC_VR, &CDMDdnDlg::OnBnClickedVr)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_StartConBtn, &CDMDdnDlg::OnBnClickedStartconbtn)
	ON_BN_CLICKED(IDC_StopConBtn, &CDMDdnDlg::OnBnClickedStopconbtn)
	ON_BN_CLICKED(IDC_SaveResBtn, &CDMDdnDlg::OnBnClickedSaveresbtn)
	ON_BN_CLICKED(IDC_CHECKProcess, &CDMDdnDlg::OnBnClickedCheckprocess)
	ON_WM_TIMER()
  ON_EN_CHANGE(IDC_WIDTH2, &CDMDdnDlg::OnEnChangeWidth2)
  ON_EN_CHANGE(IDC_WIDTH, &CDMDdnDlg::OnEnChangeWidth)
  ON_BN_CLICKED(IDC_BUTTON_STARTSI, &CDMDdnDlg::OnBnClickedButtonStartsi)
  ON_BN_CLICKED(IDC_BUTTON_STOPSI, &CDMDdnDlg::OnBnClickedButtonStopsi)
  ON_BN_CLICKED(IDC_BUTTON_SI_PATTERN, &CDMDdnDlg::OnBnClickedButtonSiPattern)
	ON_BN_CLICKED(IDC_InitDMDBtn, &CDMDdnDlg::OnBnClickedInitdmdbtn)
END_MESSAGE_MAP()

// CDMDdnDlg 消息处理程序
bool CDMDdnDlg::InitPanel( LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	HConfocalPlug* m=m_ConfocalCore->GetPlugin(PLUGIN_CONFOCALMODE);
	((IHsmSubject*)this)->Attach(m);
	m->Attach(this);

	m = m_ConfocalCore->GetPlugin(PLUGIN_CONFOCALMODEEX);
	((IHsmSubject*)this)->Attach(m);
	m->Attach(this);

	// 增加结构光相关
	HConfocalPlug* si_m = m_ConfocalCore->GetPlugin(PLUGIN_SIMODE);
	((IHsmSubject*)this)->Attach(si_m);
	si_m->Attach(this);

	mF=m_ConfocalCore->GetFunction();
	m_DmdManager=m_ConfocalCore->GetDmdManager();
	if (m_DmdManager==0)
	{
		return false;
	}
	m_DmdManager->Attach(this);

	UpdateUI();

	m_ConfocalCore->GetDockablePanel(DOCKPANEL_VIDEO)->Attach(this);
	return true;
}

BOOL CDMDdnDlg::OnInitDialog()
{
	__super::OnInitDialog();

	this->SetWindowTextW(L"DmdControl");
	UpdateData(false);
	if (mDMDParas.gray == 1)
		comboGray.SetCurSel(0);
	else
		comboGray.SetCurSel(1);

	m_trigger_mode.InsertString(0, L"mode four, WLP_TRIGGER_LOOP");
	m_trigger_mode.InsertString(0, L"mode three,WLP_TRIGGER_FRAMES_AND_STOP");
	m_trigger_mode.InsertString(0, L"mode two,  WLP_TRIGGER_FRAMES");
	m_trigger_mode.InsertString(0, L"mode one,  WLP_TRIGGER_SINGLE_FRAME");
	m_trigger_mode.SetCurSel(3);
	comboDMDType.SetCurSel(0);
	
	//CRect r;
	//this->GetWindowRect(&r);
	////r.MoveToXY(0,0);
	//m_wndScroll.Create(NULL,L"",WS_CHILD | WS_VISIBLE,r,this,1000);
	//m_wndScroll.SetChild(this);
	//CSize m_szOriginal=r.Size();
	//m_wndScroll.SetScrollSize(m_szOriginal.cx,m_szOriginal.cy);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDMDdnDlg::OnBnClickedWhitedmdbtn()
{
	if(!m_DmdManager) return;
	UpdateData();
	int iW=-1,iH=-1;
	m_DmdManager->GetDownSize(&iW,&iH);
	if (iW>0)
	{
		int LargeImageBuffSize = iW * iH/8*mDMDParas.gray;
		unsigned char* pLargeImageBuff = (unsigned char*)malloc(sizeof(BYTE)*LargeImageBuffSize);
		memset(pLargeImageBuff,255,LargeImageBuffSize);
		m_DmdManager->DownLoadFrame(pLargeImageBuff,1);
		free(pLargeImageBuff);
    CString str = L" State：Have downloaded All-On picture!";
    SetDlgItemText(IDC_EDIT_INFO, str);
    ResetSIFrames();
	}
}

void CDMDdnDlg::OnBnClickedBlackdmdbtn()
{
	int iW=-1,iH=-1;
	m_DmdManager->GetDownSize(&iW,&iH);
	if (iW>0)
	{
		int LargeImageBuffSize = iW * iH/8*mDMDParas.gray;
		unsigned char* pLargeImageBuff = (unsigned char*)malloc(sizeof(BYTE)*LargeImageBuffSize);
		memset(pLargeImageBuff,0,LargeImageBuffSize);
		m_DmdManager->DownLoadFrame(pLargeImageBuff,1);
		free(pLargeImageBuff);
    CString str = L" State：Have downloaded All-off picture!";
    SetDlgItemText(IDC_EDIT_INFO, str);
    ResetSIFrames();
  }
}

void CDMDdnDlg::OnBnClickedPicbtn()
{
	if(!m_DmdManager) return;
	LPBYTE buffer=0;
	CFileDialog dlg(true);
	dlg.m_ofn.lpstrTitle=_T("Image Select");
	dlg.m_ofn.lpstrFilter=_T("All Files(*.*)\0*.*\0Color Image(*.jpg)\0*.jpg\0All Files(*.*)\0*.*\0\0");
	if (IDOK==dlg.DoModal())
	{
    int w(0), h(0);
    m_DmdManager->GetDownSize(&w, &h);
		HVideoHeader pHeader;
    memset(&pHeader, 0, sizeof(pHeader));
		if (mF && mF->OpenFile(dlg.GetPathName(),&pHeader,0,w,h))
		{
      mF->ShowImage(&pHeader, _T("image"), pHeader.Vwidth > 1200 ? 0.5 : 1.0);
			if (mDMDParas.gray==8)
			{
				m_DmdManager->DownLoadFrame(pHeader.Vbuffer,1);
			}
			else
			{
				buffer=(LPBYTE)malloc(h*w/8);
				ChangeFormate(w, h, pHeader.Vbuffer,buffer);
				m_DmdManager->DownLoadFrame(buffer,1);
			}
			delete(pHeader.Vbuffer);
      if (buffer)
      {
			  free(buffer);
      }
			//AfxMessageBox(L"DownLoad Success!");
      CString str = L" State：Have downloaded picture: " + dlg.GetPathName();
      SetDlgItemText(IDC_EDIT_INFO, str);
      ResetSIFrames();
		}
	}
	else
		return;
}

void CDMDdnDlg::OnBnClickedResetbtn()//触发
{
	if(!m_DmdManager) 
		return;
	m_DmdManager->SetStatus(DMD_Trigger);
}

void CDMDdnDlg::OnBnClickedStopbtn()
{
	if(!m_DmdManager) 
		return;
	m_DmdManager->SetStatus(DMD_STOP);

  m_start = false;

  UpdateUI();
}

void CDMDdnDlg::OnBnClickedStartdmdbtn()
{
	if(!m_DmdManager) 
		return;
	m_DmdManager->SetStatus(DMD_START);
	m_start = true;
	UpdateUI();

	//UpdateData();
	//CRect r;
	//this->GetWindowRect(&r);
	////m_wndScroll.Create(NULL,L"",WS_CHILD | WS_VISIBLE,r,this,1000);
	////m_wndScroll.SetChild(this);
	//CSize m_szOriginal=r.Size();
	////m_wndScroll.SetScrollSize(m_szOriginal.cx,m_szOriginal.cy);
	////this->MoveWindow(m_PtSize,m_PtPeriod,m_szOriginal.cx,m_szOriginal.cy);
	////Invalidate();
	////this->GetParent()->Invalidate();
	//::SendMessage(this->GetParent()->m_hWnd,WM_ChangePos,m_PtSize,m_PtPeriod);
}

void CDMDdnDlg::Func_FlushDMDParas()
{
	UpdateData(TRUE);
	if (comboGray.GetCurSel() == 0)
		mDMDParas.gray = 1;
	else
		mDMDParas.gray = 8;

	mDMDParas.m_FrameMargin = m_FramRate;
	mDMDParas.m_FrameInterval = mBlackTime;
	mDMDParas.delay = m_trigger_delay;
	mDMDParas.plusewidth = m_trigger_pw;

	switch (m_trigger_mode.GetCurSel())//触发模式
	{
	case 0:
		mDMDParas.trigeMode = 0x00000000;//WLP_TRIGGER_SINGLE_FRAME;
		break;
	case 1:
		mDMDParas.trigeMode = 0x00000002;//WLP_TRIGGER_FRAMES;
		break;
	case 2:
		mDMDParas.trigeMode = 0x00000004;//WLP_TRIGGER_FRAMES_AND_AUTOSTOP;
		break;
	case 3:
		mDMDParas.trigeMode = 0x00000006;//WLP_TRIGGER_LOOP;
		break;
	}
}

void CDMDdnDlg::OnBnClickedUpdateframe()
{
	if (!m_DmdManager) 
		return;
	Func_FlushDMDParas();
	if (m_DmdManager->RestPara(&mDMDParas))
	{
		CString str = L" State: Update success!";
		SetDlgItemText(IDC_EDIT_INFO, str);
	}
	else
	{
		CString str = L" State: Update failed!";
		SetDlgItemText(IDC_EDIT_INFO, str);
	}
}

void CDMDdnDlg::OnBnClickedPointscan()
{
	if(!m_DmdManager) 
		return;
	UpdateData();

	int iW=-1,iH=-1;
	m_DmdManager->GetDownSize(&iW,&iH);
	if (iW<0)
		return;
	IPatternGenerate* pPatterPoint=m_DmdManager->CreateDmdPattern(PATTERN_POINT);
	int w(0), h(0);
	m_DmdManager->GetDownSize(&w, &h);
	pPatterPoint->SetSize(w, h);
	PointPara mV;
	mV.iPtSize = m_PtSize;
	mV.iPtPeriod = m_PtPeriod;
	mV.iPtExSize = m_PtEx;
	pPatterPoint->Gernerate(&mV);//点扫

	m_ConfFile.RecValue(L"DMDParas", L"m_PtSize", m_PtSize, L"Confocal_DMDControl");
	m_ConfFile.RecValue(L"DMDParas", L"m_PtPeriod", m_PtPeriod, L"Confocal_DMDControl");
	m_ConfFile.RecValue(L"DMDParas", L"m_PtEx", m_PtEx, L"Confocal_DMDControl");

	CString str;
	str.Format(L" State：Download %dX%d point picture(Total:%d)", mV.iPtSize, mV.iPtPeriod, pPatterPoint->GetFrameCount());
	if (m_DmdManager->DownLoadPattern(pPatterPoint))
	{
		str += L" success!";
		miPicCount = m_PtPeriod * m_PtPeriod;
		UpdateData(false);
	}
	else
		str += L" failed!";
	SetDlgItemText(IDC_EDIT_INFO, str);
	m_DmdManager->DestroyDmdPattern(pPatterPoint);
	ResetSIFrames();
}

void CDMDdnDlg::OnBnClickedHr()
{
	Func_DownLinePic(PATTERN_LINE_HORIZ);
}

void CDMDdnDlg::Func_DownLinePic(int iLineType)
{
	if (!m_DmdManager)
		return;
	UpdateData(TRUE);
	IPatternGenerate* pPatter = m_DmdManager->CreateDmdPattern(PATTERN_LINE);
	int w(0), h(0);
	m_DmdManager->GetDownSize(&w, &h);
	pPatter->SetSize(w, h);

	LinePara mV;
	mV.iLineW = m_LineSize;
	mV.iLType = iLineType;
	mV.iLPeriod = m_LinePeriod;
	mV.iLExW = m_LineEx;
	pPatter->Gernerate(&mV);

	m_ConfFile.RecValue(L"DMDParas", L"m_LineSize", m_LineSize, L"Confocal_DMDControl");
	m_ConfFile.RecValue(L"DMDParas", L"m_LinePeriod", m_LinePeriod, L"Confocal_DMDControl");
	m_ConfFile.RecValue(L"DMDParas", L"m_LineEx", m_LineEx, L"Confocal_DMDControl");

	CString str;
	str.Format(L" State：Download %dX%d line picture(Total:%d)", m_LineSize, m_LinePeriod, pPatter->GetFrameCount());
	if (m_DmdManager->DownLoadPattern(pPatter))
	{
		str += L" success!";
		miPicCount = pPatter->GetFrameCount();
		UpdateData(false);
	}
	else
		str += L" failed!";
	m_DmdManager->DestroyDmdPattern(pPatter);
	ResetSIFrames();
}

void CDMDdnDlg::OnBnClickedVr()
{
	Func_DownLinePic(PATTERN_LINE_VERTIAL);
}

HBRUSH CDMDdnDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	switch(pWnd->GetDlgCtrlID())
	{
	case IDC_STATIC:
		pDC->SetBkMode(TRANSPARENT);
		break;
	}
	return afxGlobalData.brBarFace;
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

int CDMDdnDlg::OnSubjectNotified( IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if(ID==Notify_WLPDMD_OPEN)
	{
	}
	else if (ID==Notify_WLPDMD_STOP)
	{	
	}
	else if (ID==Notify_WLPDMD_START)
	{	
	}
	else if (ID==NOTIFY_CONFOCAL_SAVEPROCESSEND)
	{
		((CButton*)GetDlgItem(IDC_CHECKProcess))->SetCheck(false);
	}
	else if (ID==NOTIFY_CONFOCAL_INPROCESS)
	{
		strShow.Format(L"%d/%d",wParam,miPicCount);
	}
	else if (ID == NOTIFY_CONFOCAL_INPROCESSEX)
	{
		strShowEx.Format(L"%d/%d", wParam, miPicCount);
	}
	else if (ID == NOTIFY_VIDEO_FORMAT_EXP)
	{
		CalcDMDTime(*(int*)pParam, *(int*)mParam, fParam);
	}
	return 0;
}

void CDMDdnDlg::OnBnClickedStartconbtn()
{
	UpdateData();
	if (miPicCount<=0)
	{
		AfxMessageBox(L"请先下载一下DMD的图像！");
		return;
	}
	if (m_ConfocalCore->GetCurVideo())
	{
		m_ConfocalCore->GetCurVideo()->SetTrigerMode(TRIGER_OUT);
	}
	if (m_ConfocalCore->GetCurVideoEx())
	{
		m_ConfocalCore->GetCurVideoEx()->SetTrigerMode(TRIGER_OUT);
	}

	Sleep(300);//等待设置内部触发成功，否则容易在内部模式下触发共聚焦的第一帧
	this->Notify(this,NOTIFY_CONFOCAL_START,miPicCount,NULL,m_iBackGray);
	GetDlgItem(IDC_StartConBtn)->EnableWindow(false);     //按钮的禁用与启用
	GetDlgItem(IDC_StopConBtn)->EnableWindow(true);
	SetTimer(539,100,NULL);
}

void CDMDdnDlg::OnBnClickedStopconbtn()
{
	if (m_ConfocalCore->GetCurVideo())
	{
		m_ConfocalCore->GetCurVideo()->SetTrigerMode(TRIGER_INTERAL);
	}
	if (m_ConfocalCore->GetCurVideoEx())
	{
		m_ConfocalCore->GetCurVideoEx()->SetTrigerMode(TRIGER_INTERAL);
	}
	this->Notify(this,NOTIFY_CONFOCAL_STOP);
	GetDlgItem(IDC_StartConBtn)->EnableWindow(true);
	GetDlgItem(IDC_StopConBtn)->EnableWindow(false);
	KillTimer(539);
}



void CDMDdnDlg::OnBnClickedSaveresbtn()
{
	CString ImgName;
	SYSTEMTIME st;
	GetLocalTime(&st);
	CFileDialog fileDlg(false);
	fileDlg.m_ofn.lpstrTitle = _T("SaveImg");
	fileDlg.m_ofn.lpstrFilter = _T("Bmp Files(*.bmp)\0*.bmp\0All Files(*.*)\0*.*\0\0");
	fileDlg.m_ofn.lpstrDefExt = _T("bmp");
	ImgName.Format(_T("%02d-%02d_%02d%02d%02d.bmp"), st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	fileDlg.m_ofn.lpstrFile = ImgName.GetBuffer(200);
	fileDlg.m_ofn.nMaxFile = 200;
	if (IDOK == fileDlg.DoModal())
	{
		CString strSaveFile = fileDlg.GetPathName();
		this->Notify(this, NOTIFY_CONFOCAL_SAVERESULT, 0, &strSaveFile);
	}
}

void CDMDdnDlg::OnBnClickedCheckprocess()
{
	//if (((CButton*)GetDlgItem(IDC_CHECKProcess))->GetCheck())
	//{
		this->Notify(this,NOTIFY_CONFOCAL_SAVEPROCESS);
	//}
}

void CDMDdnDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (539==nIDEvent)
	{
		SetDlgItemText(IDC_ConfocalState,strShow);
		SetDlgItemText(IDC_ConfocalState2, strShowEx);
	}
	__super::OnTimer(nIDEvent);
}

void CDMDdnDlg::UpdateUI()
{
  bool valid(false);
  if (m_DmdManager)
  {
	  valid = true;
	  /*int iRAMsizeMB = m_DmdManager->GetInfo()->RAMsizeMB;
	  if (iRAMsizeMB > 0)
		  valid = true;*/
  }
  if (!valid)
  {
	GetDlgItem(IDC_InitDMDBtn)->EnableWindow(FALSE);
	GetDlgItem(IDC_StartDMDBtn)->EnableWindow(FALSE);
	GetDlgItem(IDC_WhiteDMDBtn)->EnableWindow(FALSE);
    GetDlgItem(IDC_BlackDMDBtn)->EnableWindow(FALSE);
    GetDlgItem(IDC_PicBtn)->EnableWindow(FALSE);
    GetDlgItem(IDC_ResetBtn)->EnableWindow(FALSE);
    GetDlgItem(IDC_StopBtn)->EnableWindow(FALSE);
    GetDlgItem(IDC_UpdateFrame)->EnableWindow(FALSE);
    GetDlgItem(IDC_PointScan)->EnableWindow(FALSE);
    GetDlgItem(IDC_HR)->EnableWindow(FALSE);
    GetDlgItem(IDC_VR)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_SI_PATTERN)->EnableWindow(FALSE);
  }
  else
  {
	GetDlgItem(IDC_InitDMDBtn)->EnableWindow(!m_start&!m_Open);
    GetDlgItem(IDC_StartDMDBtn)->EnableWindow(!m_start&m_Open);
    GetDlgItem(IDC_WhiteDMDBtn)->EnableWindow(!m_start&m_Open);
    GetDlgItem(IDC_BlackDMDBtn)->EnableWindow(!m_start&m_Open);
    GetDlgItem(IDC_PicBtn)->EnableWindow(!m_start&m_Open);
    GetDlgItem(IDC_ResetBtn)->EnableWindow(m_start&m_Open);
    GetDlgItem(IDC_StopBtn)->EnableWindow(m_start&m_Open);
    GetDlgItem(IDC_UpdateFrame)->EnableWindow(!m_start&m_Open);
    GetDlgItem(IDC_PointScan)->EnableWindow(!m_start&m_Open);
    GetDlgItem(IDC_HR)->EnableWindow(!m_start&m_Open);
    GetDlgItem(IDC_VR)->EnableWindow(!m_start&m_Open);
    GetDlgItem(IDC_BUTTON_SI_PATTERN)->EnableWindow(!m_start&m_Open);
  }
}

void CDMDdnDlg::OnEnChangeWidth2()
{
  UpdateData(TRUE);
  m_PtEx = m_PtSize;
  UpdateData(FALSE);
}

void CDMDdnDlg::OnEnChangeWidth()
{
  UpdateData(TRUE);
  m_LineEx = m_LineSize;
  UpdateData(FALSE);
}

void CDMDdnDlg::CalcDMDTime(int width, int height, float exposure)
{
  float mCameraScanTime = 0.0f;
  if (width >= 2000)
    mCameraScanTime = 0.0097 * 2048 / 2;
  else if (width >= 1000)
    mCameraScanTime = 0.0097 * 1024 / 2;
  else if (width >= 500)
    mCameraScanTime = 0.0097 * 512 / 2;
  mBlackTime = 0;
  m_FramRate = (exposure + mCameraScanTime + 0.1)*1000.0f + 100;//累加0.1ms，并且m_FrameMargin也要再累加100，这样才可以保证采集频率跟上
  UpdateData(FALSE);
}

void CDMDdnDlg::OnBnClickedButtonStartsi()
{
  UpdateData();
  if (_si_frames <= 0)
  {
    return;
  }
  if (m_ConfocalCore->GetCurVideo())
  {
    m_ConfocalCore->GetCurVideo()->SetTrigerMode(TRIGER_OUT);
  }
  Sleep(300);//等待设置内部触发成功，否则容易在内部模式下触发第一帧
  this->Notify(this, NOTIFY_SI_START, _si_frames, NULL, 0);
  GetDlgItem(IDC_BUTTON_STARTSI)->EnableWindow(false);
  GetDlgItem(IDC_BUTTON_STOPSI)->EnableWindow(true);
  //SetTimer(539, 100, NULL);
}

void CDMDdnDlg::OnBnClickedButtonStopsi()
{
  if (m_ConfocalCore->GetCurVideo())
  {
    m_ConfocalCore->GetCurVideo()->SetTrigerMode(TRIGER_INTERAL);
  }
  this->Notify(this, NOTIFY_SI_STOP);
  GetDlgItem(IDC_BUTTON_STARTSI)->EnableWindow(true);
  GetDlgItem(IDC_BUTTON_STOPSI)->EnableWindow(false);
  //KillTimer(539);
}

void CDMDdnDlg::OnBnClickedButtonSiPattern()
{
  if (!mF)
  {
    return;
  }

  UpdateData();

  // 检查是否存在需要的图案
  CString path;
  _sl_path.GetWindowText(path);
  CString horz_list[3] =
  {
    _T("horz_0.bmp"),
    _T("horz_1.bmp"),
    _T("horz_2.bmp"),
  };
  CString vert_list[3] =
  {
    _T("vert_0.bmp"),
    _T("vert_1.bmp"),
    _T("vert_2.bmp"),
  };

  int dmd_w(1024), dmd_h(768);
  if (m_DmdManager)
  {
    m_DmdManager->GetDownSize(&dmd_w, &dmd_h);
  }
  CString tmppath;
  HVideoHeader horz_header[3];
  for (int i = 0; i < _countof(horz_list); i++)
  {
    tmppath = path + _T("\\") + horz_list[i];
    LPBYTE* tmp(0);
    int w(dmd_w), h(dmd_h);
    memset(horz_header + i, 0, sizeof(horz_header[i]));
    if (!mF->OpenFile(tmppath, horz_header + i, 0, w, h))
    {
      break;
    }
  }
  HVideoHeader vert_header[3];
  for (int i = 0; i < _countof(vert_list); i++)
  {
    tmppath = path + _T("\\") + vert_list[i];
    LPBYTE* tmp(0);
    int w(dmd_w), h(dmd_h);
    memset(vert_header + i, 0, sizeof(vert_header[i]));
    if (!mF->OpenFile(tmppath, vert_header + i, 0, w, h))
    {
      break;
    }
  }
  bool has_horz(horz_header[0].Vbuffer &&
    horz_header[1].Vbuffer && horz_header[2].Vbuffer);
  bool has_vert(vert_header[0].Vbuffer &&
    vert_header[1].Vbuffer && vert_header[2].Vbuffer);

  int frames(0);
  if (has_horz)
  {
    frames += 3;
  }
  if (has_vert)
  {
    frames += 3;
  }
  if (frames < 3)
  {
    for (int i = 0; i < _countof(horz_header); i++)
    {
      delete[]horz_header[i].Vbuffer;
    }
    for (int i = 0; i < _countof(vert_header); i++)
    {
      delete[]vert_header[i].Vbuffer;
    }
    return;
  }

  if (mDMDParas.gray == 8)
  {
    // 加一张黑图像用于区分起始点
    unsigned char* buf = new unsigned char[dmd_w * dmd_h * (frames + 1)];
    memset(buf, 0, dmd_w * dmd_h);
    int k = 1;
    if (has_horz)
    {
      for (int i = 0; i < _countof(horz_header); i++)
      {
        memcpy(buf + dmd_w * dmd_h * (i + k), horz_header[i].Vbuffer, dmd_w * dmd_h);
      }
      k += 3;
    }
    if (has_vert)
    {
      for (int i = 0; i < _countof(vert_header); i++)
      {
        memcpy(buf + dmd_w * dmd_h * (i + k), vert_header[i].Vbuffer, dmd_w * dmd_h);
      }
    }
    if (m_DmdManager)
    {
      m_DmdManager->DownLoadFrame(buf, frames + 1);
    }
    delete[]buf;
  }
  else
  {
    int frame_len = dmd_w * dmd_h / 8;
    unsigned char* buf = new unsigned char[frame_len * (frames + 1)];
    memset(buf, 0, frame_len);
    int k(1);
    if (has_horz)
    {
      for (int i = 0; i < _countof(horz_header); i++)
      {
        ChangeFormate(dmd_w, dmd_h, horz_header[i].Vbuffer, buf + frame_len * (i + k));
      }
      k += 3;
    }
    if (has_vert)
    {
      for (int i = 0; i < _countof(vert_header); i++)
      {
        ChangeFormate(dmd_w, dmd_h, vert_header[i].Vbuffer, buf + frame_len * (i + k));
      }
    }
    if (m_DmdManager)
    {
      m_DmdManager->DownLoadFrame(buf, frames + 1);
    }
    delete[]buf;
  }

  for (int i = 0; i < _countof(horz_header); i++)
  {
    delete[]horz_header[i].Vbuffer;
  }
  for (int i = 0; i < _countof(vert_header); i++)
  {
    delete[]vert_header[i].Vbuffer;
  }
  _si_frames = frames;
}

void CDMDdnDlg::ResetSIFrames()
{
  _si_frames = 0;
}


void CDMDdnDlg::OnBnClickedInitdmdbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_DmdManager)
		return;
	Func_FlushDMDParas();
	if (m_DmdManager->InitDMD(&mDMDParas,comboDMDType.GetCurSel()))
	{
		CString str = L" State: OpenDMD success!";
		SetDlgItemText(IDC_EDIT_INFO, str);
		m_Open = true;
		UpdateUI();
	}
	else
	{
		CString str = L" State: OpenDMD failed!";
		SetDlgItemText(IDC_EDIT_INFO, str);
	}
}
