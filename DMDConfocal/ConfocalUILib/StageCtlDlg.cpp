// StageCtlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "StageCtlDlg.h"
#include "afxdialogex.h"


// CStageCtlDlg 对话框

IMPLEMENT_DYNAMIC(CStageCtlDlg, CDialogEx)

CStageCtlDlg::CStageCtlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CStageCtlDlg::IDD, pParent)
	, m_XYMove(10)
	, m_ZMove(5)
	, mRadio(0)
	, iTurnOrder(0)
{
	m_GearBox=0;
	m_AxisX=0;
	m_AxisY=0;
	m_AxisZ=0;
	m_AxisW=0;
	m_AxisT=0;
	m_AxisO=0;

	m_GearBox=0;
	m_ConfocalCore=0;
	m_hMoveEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_VideoDevice=0;
	iZoffset=2;
}

CStageCtlDlg::~CStageCtlDlg()
{
}

void CStageCtlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDITXY, m_XYMove);
	DDX_Text(pDX, IDC_EDITZ, m_ZMove);
	DDX_Radio(pDX, IDC_RADIO0, mRadio);
	DDX_Control(pDX, IDC_YAddBtn, m_btn_top);
	DDX_Control(pDX, IDC_CBOObser, mObserComBo);
	DDX_Radio(pDX, IDC_RADIO6, iTurnOrder);
	DDX_Control(pDX, IDC_SLIDERLight, mSliderLight);
}


BEGIN_MESSAGE_MAP(CStageCtlDlg, CDialogEx)
	ON_BN_CLICKED(IDC_YAddBtn, &CStageCtlDlg::OnBnClickedYaddbtn)
	ON_BN_CLICKED(IDC_XAddBtn, &CStageCtlDlg::OnBnClickedXaddbtn)
	ON_BN_CLICKED(IDC_XSubBtn, &CStageCtlDlg::OnBnClickedXsubbtn)
	ON_BN_CLICKED(IDC_YSubBtn, &CStageCtlDlg::OnBnClickedYsubbtn)
	ON_BN_CLICKED(IDC_ZAddBtn, &CStageCtlDlg::OnBnClickedZaddbtn)
	ON_BN_CLICKED(IDC_ZSubBtn, &CStageCtlDlg::OnBnClickedZsubbtn)
	ON_BN_CLICKED(IDC_ResetX, &CStageCtlDlg::OnBnClickedResetx)
	ON_BN_CLICKED(IDC_ResetY, &CStageCtlDlg::OnBnClickedResety)
	ON_BN_CLICKED(IDC_ResetZ, &CStageCtlDlg::OnBnClickedResetz)
	ON_BN_CLICKED(IDC_StopX, &CStageCtlDlg::OnBnClickedStopx)
	ON_BN_CLICKED(IDC_StopY, &CStageCtlDlg::OnBnClickedStopy)
	ON_BN_CLICKED(IDC_StopZ, &CStageCtlDlg::OnBnClickedStopz)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_RADIO0, &CStageCtlDlg::OnBnClickedRadio0)
	ON_BN_CLICKED(IDC_RADIO1, &CStageCtlDlg::OnBnClickedRadio0)
	ON_BN_CLICKED(IDC_RADIO2, &CStageCtlDlg::OnBnClickedRadio0)
	ON_BN_CLICKED(IDC_RADIO3, &CStageCtlDlg::OnBnClickedRadio0)
	ON_BN_CLICKED(IDC_RADIO4, &CStageCtlDlg::OnBnClickedRadio0)
	ON_BN_CLICKED(IDC_InitBtn, &CStageCtlDlg::OnBnClickedInitbtn)
	ON_CBN_SELCHANGE(IDC_CBOObser, &CStageCtlDlg::OnCbnSelchangeCboobser)
	ON_BN_CLICKED(IDC_RADIO6, &CStageCtlDlg::OnBnClickedRadio6)
	ON_BN_CLICKED(IDC_RADIO7, &CStageCtlDlg::OnBnClickedRadio6)
	ON_BN_CLICKED(IDC_RADIO8, &CStageCtlDlg::OnBnClickedRadio6)
	ON_BN_CLICKED(IDC_RADIO9, &CStageCtlDlg::OnBnClickedRadio6)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CStageCtlDlg 消息处理程序


BOOL CStageCtlDlg::OnInitDialog()
{
	__super::OnInitDialog();
	// TODO:  在此添加额外的初始化
	this->SetWindowTextW(L"StageControl");
	m_XYMove=m_ConfFile.GetDouble(L"Paras",L"XYMoveStep",L"Confocal_Stage");
	m_ZMove=m_ConfFile.GetDouble(L"Paras",L"ZMoveStep",L"Confocal_Stage");
	CString strObject=m_ConfFile.GetString(L"Object",L"curobj",L"Confocal_Microscope");
	if (strObject==L"4x")
		mRadio=0;
	else if (strObject==L"10x")
		mRadio=1;
	else if (strObject==L"20x")
		mRadio=2;
	else if (strObject==L"40x")
		mRadio=3;
	else if (strObject==L"100x")
		mRadio=4;
	UpdateData(false);
	UpdateData();
	//HBITMAP    hBitmap = ::LoadBitmap(::AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_PNG1));
	//m_btn_top.SetBitmap(hBitmap);
	//m_btn_top.SetIcon(::LoadIcon(::GetModuleHandle(L"ConfocalUILib.dll"),MAKEINTRESOURCE(IDI_ICON_TOP)));
	//((IHsmSubject)m_btn_top).Attach(this);
	InitObservePt();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CStageCtlDlg::InitObservePt()
{
	CString strAll=m_ConfFile.GetString(L"Paras",L"Name",L"Confocal_Observate");
	CStringArray mStrArray;
	int k=splitString(strAll,L'_',mStrArray);
	//strAll.Format(L"%s,%d",strAll,k);
	//AfxMessageBox(strAll);
	for(int i=0;i<k;i++)
	{
		ObservePt mV;
		mV.strName=mStrArray.GetAt(i);
		mV.PtX=m_ConfFile.GetDouble(mV.strName,L"X",L"Confocal_Observate");
		mV.PtY=m_ConfFile.GetDouble(mV.strName,L"Y",L"Confocal_Observate");
		mV.PtZ=m_ConfFile.GetDouble(mV.strName,L"Z",L"Confocal_Observate");
		mObserComBo.AddString(mV.strName);
		mObservePt.push_back(mV);
	}
	//mObserComBo.EnableWindow(false);
}

int CStageCtlDlg::splitString(CString str, char split, CStringArray& strArray) 
{ 
	strArray.RemoveAll();
	CString strTemp = str; //此赋值不能少
	int nIndex = 0; 
	while(1) 
	{ 
		nIndex = strTemp.Find( split ); 
		if( nIndex >= 0 ) 
		{ 
			strArray.Add( strTemp.Left( nIndex ) ); 
			strTemp = strTemp.Right(strTemp.GetLength()-nIndex-1); 
		} 
		else 
			break; 
	} 
	strArray.Add( strTemp ); 
	return strArray.GetSize(); 
} 

void CStageCtlDlg::SaveParas()
{
	m_ConfFile.RecValue(L"Paras",L"XYMoveStep",m_XYMove,L"Confocal_Stage");
	m_ConfFile.RecValue(L"Paras",L"ZMoveStep",m_ZMove,L"Confocal_Stage");
}

bool CStageCtlDlg::InitPanel( LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	m_GearBox=m_ConfocalCore->GetGearBox();
	m_VideoDevice=m_ConfocalCore->GetCurVideo();
	if(m_GearBox)
	{
		m_AxisX=m_GearBox->GetAxis(AXIS_X);
		if (m_AxisX)
			m_AxisX->GetSubject()->Attach(this);
		m_AxisY=m_GearBox->GetAxis(AXIS_Y);
		if (m_AxisY)
			m_AxisY->GetSubject()->Attach(this);
		m_AxisZ=m_GearBox->GetAxis(AXIS_Z);
		if (m_AxisZ)
			m_AxisZ->GetSubject()->Attach(this);
		//m_AxisW=m_GearBox->GetAxis(AXIS_W);	// 屏蔽物镜转换器
		if (m_AxisW)
			m_AxisW->GetSubject()->Attach(this);
		m_AxisT=m_GearBox->GetAxis(AXIS_T);
		if (m_AxisT)
			m_AxisT->GetSubject()->Attach(this);
		m_AxisO=m_GearBox->GetAxis(AXIS_O);
		if (m_AxisO)
			m_AxisO->GetSubject()->Attach(this);

		mSliderLight.SetRange(0,999);
		ReadPosValue();	
		//CreateThread(NULL, 0, InitPosProc, this, 0, 0);
		//AFX_MANAGE_STATE(AfxGetStaticModuleState());
		//MakeMsgDlg();
	}
	HCorePanel* m=m_ConfocalCore->GetViewPanel(VIEWPANEL_REALTIME);
	((IHsmSubject*)this)->Attach(m);
	m = m_ConfocalCore->GetViewPanel(VIEWPANEL_REALTIMEEX);
	((IHsmSubject*)this)->Attach(m);
	return true;
}


void CStageCtlDlg::OnBnClickedYaddbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if(m_AxisY)
		m_AxisY->MoveRef(m_XYMove);
	SaveParas();
}


void CStageCtlDlg::OnBnClickedXaddbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();	
	if(m_AxisX)
		m_AxisX->MoveRef(m_XYMove);
	SaveParas();
}


void CStageCtlDlg::OnBnClickedXsubbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if(m_AxisX)
		m_AxisX->MoveRef(-m_XYMove);
	SaveParas();
}


void CStageCtlDlg::OnBnClickedYsubbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if(m_AxisY)
		m_AxisY->MoveRef(-m_XYMove);
	SaveParas();
}


void CStageCtlDlg::OnBnClickedZaddbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if(m_AxisZ)
		m_AxisZ->MoveRef(m_ZMove);
	SaveParas();
}


void CStageCtlDlg::OnBnClickedZsubbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if(m_AxisZ)
		m_AxisZ->MoveRef(-m_ZMove);
	SaveParas();
}


void CStageCtlDlg::OnBnClickedResetx()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_AxisX)
		m_AxisX->Rest();
}


void CStageCtlDlg::OnBnClickedResety()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_AxisY)
		m_AxisY->Rest();
}


void CStageCtlDlg::OnBnClickedResetz()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_AxisZ)
		m_AxisZ->Rest();
}


void CStageCtlDlg::OnBnClickedStopx()
{
	// TODO: 在此添加控件通知处理程序代码
	//MakeMsgDlg();
	if(m_AxisX)
		m_AxisX->Stop();
}


void CStageCtlDlg::OnBnClickedStopy()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_AxisY)
		m_AxisY->Stop();
}


void CStageCtlDlg::OnBnClickedStopz()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_AxisZ)
		m_AxisZ->Stop();
}


HBRUSH CStageCtlDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	return afxGlobalData.brBarFace;
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


void CStageCtlDlg::OnBnClickedRadio0()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();	
	CString strObject;
	float mPixelSize;
	float mVSize=m_ConfFile.GetDouble(L"Size",L"ImageROISize",L"Confocal_Map");
	int mV=0;
	if (m_AxisZ)
		mZPos=m_AxisZ->GetPosition();
	else 
		mZPos=1.0f;
	//记录下变更
	switch(mRadio)
	{
		case 0:
			strObject=L"4x_mm2pixel";
			mPixelSize=m_ConfFile.GetDouble(L"PixelSize",strObject,L"Confocal_Microscope");
			mV=mPixelSize*mVSize*1000;
			m_ConfFile.RecValue(L"Object",L"curobj",L"4x",L"Confocal_Microscope");
			iObj=i4XObj;
			mNowExp=m_4XExp;
			break;
		case 1:
			strObject=L"10x_mm2pixel";
			mPixelSize=m_ConfFile.GetDouble(L"PixelSize",strObject,L"Confocal_Microscope");
			mV=mPixelSize*mVSize*1000;
			m_ConfFile.RecValue(L"Object",L"curobj",L"10x",L"Confocal_Microscope");
			iObj=i10XObj;
			mNowExp=m_10XExp;
			break;
		case 2:
			strObject=L"20x_mm2pixel";
			mPixelSize=m_ConfFile.GetDouble(L"PixelSize",strObject,L"Confocal_Microscope");
			mV=mPixelSize*mVSize*1000;
			m_ConfFile.RecValue(L"Object",L"curobj",L"20x",L"Confocal_Microscope");
			iObj=i20XObj;
			mNowExp=m_20XExp;
			break;
		case 3:
			strObject=L"40x_mm2pixel";
			mPixelSize=m_ConfFile.GetDouble(L"PixelSize",strObject,L"Confocal_Microscope");
			mV=mPixelSize*mVSize*1000;
			m_ConfFile.RecValue(L"Object",L"curobj",L"40x",L"Confocal_Microscope");
			iObj=i40XObj;
			mNowExp=m_40XExp;
			break;
		case 4:
			strObject=L"100x_mm2pixel";
			mPixelSize=m_ConfFile.GetDouble(L"PixelSize",strObject,L"Confocal_Microscope");
			mV=mPixelSize*mVSize*1000;
			m_ConfFile.RecValue(L"Object",L"curobj",L"100x",L"Confocal_Microscope");
			iObj=i100XObj;
			mNowExp=m_100XExp;//加装滴油的对话框
			break;
	}
	if (mRadio>=0) 
	{
		Notify(this,NOTIFY_MAPROISIZE_FLUSH,mV,0,mPixelSize);
		CreateThread(NULL, 0, SwitchObjProc, this, 0, 0);
		if (mRadio!=4)
			MakeMsgDlg();		
	}		
}

DWORD WINAPI InitPosProc( LPVOID lp )
{
	((CStageCtlDlg*)lp)->InitPos();
	return 0;
}

DWORD WINAPI SwitchObjProc( LPVOID lp )
{
	((CStageCtlDlg*)lp)->SwitchObj();
	return 0;
}

void CStageCtlDlg:: ReadPosValue()
{
	m_ZInitPos=m_ConfocalCore->GetConfigure()->GetDouble(L"StageParas",L"ZInitPos",L"Confocal_Stage");
	m_XInitPos=m_ConfocalCore->GetConfigure()->GetDouble(L"StageParas",L"XInitPos",L"Confocal_Stage");
	m_YInitPos=m_ConfocalCore->GetConfigure()->GetDouble(L"StageParas",L"YInitPos",L"Confocal_Stage");
	i4XObj=m_ConfocalCore->GetConfigure()->GetDouble(L"StageParas",L"i4XObj",L"Confocal_Stage");
	i10XObj=m_ConfocalCore->GetConfigure()->GetDouble(L"StageParas",L"i10XObj",L"Confocal_Stage");
	i20XObj=m_ConfocalCore->GetConfigure()->GetDouble(L"StageParas",L"i20XObj",L"Confocal_Stage");
	i40XObj=m_ConfocalCore->GetConfigure()->GetDouble(L"StageParas",L"i40XObj",L"Confocal_Stage");
	iMidObj=m_ConfocalCore->GetConfigure()->GetDouble(L"StageParas",L"iMidObj",L"Confocal_Stage");
	i100XObj=m_ConfocalCore->GetConfigure()->GetDouble(L"StageParas",L"i100XObj",L"Confocal_Stage");
	iZoffset=m_ConfocalCore->GetConfigure()->GetDouble(L"StageParas",L"iZoffset",L"Confocal_Stage");

	m_4XExp=m_ConfocalCore->GetConfigure()->GetDouble(L"Video",L"4X_exposure",L"Confocal_Camera");
	m_10XExp=m_ConfocalCore->GetConfigure()->GetDouble(L"Video",L"10X_exposure",L"Confocal_Camera");
	m_20XExp=m_ConfocalCore->GetConfigure()->GetDouble(L"Video",L"20X_exposure",L"Confocal_Camera");
	m_40XExp=m_ConfocalCore->GetConfigure()->GetDouble(L"Video",L"40X_exposure",L"Confocal_Camera");
	m_100XExp=m_ConfocalCore->GetConfigure()->GetDouble(L"Video",L"100X_exposure",L"Confocal_Camera");
	if (m_AxisO)
	{
		int iV=m_AxisO->GetPosition();	
		mSliderLight.SetPos(iV);
	}
}

void CStageCtlDlg::InitPos()
{
	MoveAxis(m_AxisZ,-1,L"Status:  Z RestMoving");//为了Z轴更新位置
	MoveAxis(m_AxisZ,iZoffset,L"Status:  Z Moving");
	MoveAxis(m_AxisX,-1,L"Status:  X RestMoving");
	MoveAxis(m_AxisY,-1,L"Status:  Y RestMoving");
	if (m_AxisW)
	{
		m_AxisW->MoveTo(1);
		MoveAxis(m_AxisW,-1,L"Status:  W RestMoving");
	}
	Sleep(1000);
	MoveAxis(m_AxisX,m_XInitPos,L"Status:  X Moving");
	MoveAxis(m_AxisY,m_YInitPos,L"Status:  Y Moving");
	MoveAxis(m_AxisZ,m_ZInitPos,L"Status:  Z Moving");
	Notify(this,NOTIFY_MSGVIEW_END,0,0);
}

void CStageCtlDlg::MoveAxis(HAxis *m_Axis,float mPos,CString strOut)
{
	int iTime=10000;
	Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strOut);
	ResetEvent(m_hMoveEvent);
	if (mPos<0)
		m_Axis->Rest();
	else
		m_Axis->MoveTo(mPos);
	if (WaitForSingleObject(m_hMoveEvent,iTime)==WAIT_TIMEOUT)
	{
		CString strMsg=strOut+L"  TIMEOUT!";
		Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
		Sleep(3000);
		//AfxMessageBox(strOut+L"  TIMEOUT!");
	}
}

int CStageCtlDlg::OnSubjectNotified( IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{

	CString strObject;
	float mPixelSize;
	float mVSize=m_ConfFile.GetDouble(L"Size",L"ImageROISize",L"Confocal_Map");
	int mV=0;

	if(ID==NOTIFY_AXIS_MOVED)//||ID==NOTIFY_AXIS_POSITION)
		SetEvent(m_hMoveEvent);
	else if(ID == NOTIFY_AXISW_CHANGING)
	{
		int iONow=*(float*)pParam;
		if (iONow==i4XObj)
		{
			m_ConfFile.RecValue(L"Object",L"curobj",L"4x",L"Confocal_Microscope");
			mRadio=0;strObject=L"4x_mm2pixel";
		}		
		else if (iONow==i10XObj)
		{
			m_ConfFile.RecValue(L"Object",L"curobj",L"10x",L"Confocal_Microscope");
			mRadio=1;strObject=L"10x_mm2pixel";
		}	
		else if (iONow==i20XObj)
		{
			m_ConfFile.RecValue(L"Object",L"curobj",L"20x",L"Confocal_Microscope");
			mRadio=2;strObject=L"20x_mm2pixel";
		}	
		else if (iONow==i40XObj)
		{
			m_ConfFile.RecValue(L"Object",L"curobj",L"40x",L"Confocal_Microscope");
			mRadio=3;strObject=L"40x_mm2pixel";
		}	
		else if (iONow==i100XObj)
		{
			m_ConfFile.RecValue(L"Object",L"curobj",L"100x",L"Confocal_Microscope");
			mRadio=4;strObject=L"100x_mm2pixel";
		}	
		if (mRadio>=0) 
		{
			mPixelSize=m_ConfFile.GetDouble(L"PixelSize",strObject,L"Confocal_Microscope");
			mV=mPixelSize*mVSize*1000;
			Notify(this,NOTIFY_MAPROISIZE_FLUSH,mV,0,mPixelSize);
		}
		UpdateData(false);
	}
	//	TRACE(L"EVENT_BUTTONCLICKED");
	//else if (ID == EVENT_BUTTONDOWN)
	//	TRACE(L"EVENT_BUTTONDOWN");
	//else if (ID == EVENT_BUTTONUP)
	//	TRACE(L"EVENT_BUTTONUP");
	return 0;
}

void CStageCtlDlg::SwitchObj()
{
	if (iObj==i100XObj)
	{
		MoveAxis(m_AxisZ,mZPos+2,L"Status:  Z Moving");
		if (m_AxisW)
			m_AxisW->MoveTo(iMidObj);
		AfxMessageBox(L"请滴油！");
		if (m_AxisW)
			m_AxisW->MoveTo(iObj);
		Sleep(1000);//等待转镜头
		MoveAxis(m_AxisZ,mZPos,L"Status:  Z Moving");
		if (m_VideoDevice)
			m_VideoDevice->SetExposure(mNowExp);
		return;
	}

	MoveAxis(m_AxisZ,mZPos+iZoffset,L"Status:  Z Moving");
	if (m_AxisW)
		m_AxisW->MoveTo(iObj);
	//MoveAxis(m_AxisW,iObj,L"Status:  W Moving");
	Sleep(1000);//等待转镜头
	MoveAxis(m_AxisZ,mZPos,L"Status:  Z Moving");
	if (m_VideoDevice)
	{
		m_VideoDevice->SetExposure(mNowExp);
	}
	Notify(this,NOTIFY_MSGVIEW_END,0,0);
}

void CStageCtlDlg::MakeMsgDlg()
{
	HCorePanel* mDlg=m_ConfocalCore->GetMsgPanel(MSG_TIME);
	mDlg->InitPanel(0);
	((HCorePanel*)this)->Attach(mDlg);
	CWnd::PostMessage(0x118);
	((CDialogEx*)mDlg->GetCWnd())->DoModal();
	((HCorePanel*)this)->Detach(mDlg);
}



void CStageCtlDlg::OnBnClickedInitbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	ReadPosValue();
	CreateThread(NULL, 0, InitPosProc, this, 0, 0);
	MakeMsgDlg();
	mObserComBo.EnableWindow(true);
}


void CStageCtlDlg::OnCbnSelchangeCboobser()
{
	// TODO: 在此添加控件通知处理程序代码
	int nIndex = mObserComBo.GetCurSel();
	if (nIndex<0)
		return;
	if (mObservePt.size()<=nIndex)
		return;
	if (m_AxisX&&mObservePt[nIndex].PtX>=0)
		m_AxisX->MoveTo(mObservePt[nIndex].PtX);
	if (m_AxisY&&mObservePt[nIndex].PtY>=0)
		m_AxisY->MoveTo(mObservePt[nIndex].PtY);
	Sleep(1000);
	if (m_AxisZ&&mObservePt[nIndex].PtZ>=0)
		m_AxisZ->MoveTo(mObservePt[nIndex].PtZ);
}


void CStageCtlDlg::OnBnClickedRadio6()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();	
	if (m_AxisT)
	{
		m_AxisT->MoveTo(iTurnOrder);
	}
	//记录下变更
	//switch(mRadio)
	//{
	//	case 0:

	//		break;
	//	case 1:

	//		break;
	//	case 2:

	//		break;
	//	case 3:

	//		break;
	//}
}


void CStageCtlDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int iV=mSliderLight.GetPos();
	if (m_AxisO)
		m_AxisO->MoveTo(iV);
	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}
