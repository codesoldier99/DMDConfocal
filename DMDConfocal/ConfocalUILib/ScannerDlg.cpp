// ScannerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "ScannerDlg.h"
#include "afxdialogex.h"
#include "ScannerParas.h"
#include "HGloableFunction.h"

// CScannerDlg 对话框

IMPLEMENT_DYNAMIC(CScannerDlg, CDialogEx)

CScannerDlg::CScannerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CScannerDlg::IDD, pParent)
	, m_fUpPos(0)
	, m_fDownPos(2)
	, m_fStepPos(0.1)
	, m_iSpeed(2)
	, miMapStartX(0)
	, miMapStartY(0)
	, miMapEndX(75)
	, miMapEndY(24)
	, mScanType(0)
	, iScanSave(100)
	, mdPIUp(0)
	, mdPIDown(0.4)
	, mdPIStep(0.01)
	, mdPIMove(0.1)
	, m_iCtrlAxisMode(0)
{
	m_ConfocalCore=0;
	m_AxisZ=0;
	m_AxisPIZ=0;
}

BOOL CScannerDlg::OnInitDialog()
{
	__super::OnInitDialog();

	// TODO:  在此添加额外的初始化
	this->SetWindowTextW(L"ScanDlg");
	m_fUpPos=m_ConfFile.GetDouble(L"3DRebuild",L"UpPos",L"Confocal_Scaner");
	m_fDownPos=m_ConfFile.GetDouble(L"3DRebuild",L"DownPos",L"Confocal_Scaner");
	m_fStepPos=m_ConfFile.GetDouble(L"3DRebuild",L"StepPos",L"Confocal_Scaner");
	m_iSpeed=m_ConfFile.GetDouble(L"3DRebuild",L"Speed",L"Confocal_Scaner");

	mdPIUp=m_ConfFile.GetDouble(L"3DRebuild",L"PIUpPos",L"Confocal_Scaner");
	mdPIDown=m_ConfFile.GetDouble(L"3DRebuild",L"PIDownPos",L"Confocal_Scaner");
	mdPIStep=m_ConfFile.GetDouble(L"3DRebuild",L"PIStep",L"Confocal_Scaner");
	mdPIMove=m_ConfFile.GetDouble(L"3DRebuild",L"PIMove",L"Confocal_Scaner");

	miMapStartX=m_ConfFile.GetDouble(L"Range",L"StartX",L"Confocal_Map");
	miMapStartY=m_ConfFile.GetDouble(L"Range",L"StartY",L"Confocal_Map");
	miMapEndX=m_ConfFile.GetDouble(L"Range",L"EndX",L"Confocal_Map");
	miMapEndY=m_ConfFile.GetDouble(L"Range",L"EndY",L"Confocal_Map");

	UpdateData(false);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CScannerDlg::SaveParas()
{
	m_ConfFile.RecValue(L"3DRebuild",L"UpPos",m_fUpPos,L"Confocal_Scaner");
	m_ConfFile.RecValue(L"3DRebuild",L"DownPos",m_fDownPos,L"Confocal_Scaner");
	m_ConfFile.RecValue(L"3DRebuild",L"StepPos",m_fStepPos,L"Confocal_Scaner");

	m_ConfFile.RecValue(L"3DRebuild",L"PIUpPos",mdPIUp,L"Confocal_Scaner");
	m_ConfFile.RecValue(L"3DRebuild",L"PIDownPos",mdPIDown,L"Confocal_Scaner");
	m_ConfFile.RecValue(L"3DRebuild",L"PIStep",mdPIStep,L"Confocal_Scaner");


	m_ConfFile.RecValue(L"3DRebuild", L"Speed", m_iSpeed, L"Confocal_Scaner"); //共用速度
	m_ConfFile.RecValue(L"3DRebuild",L"PIMove",mdPIMove,L"Confocal_Scaner");//单独控制PI移动量
}

CScannerDlg::~CScannerDlg()
{
}

void CScannerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT3, m_fUpPos);
	DDX_Text(pDX, IDC_EDIT4, m_fDownPos);
	DDX_Text(pDX, IDC_EDIT5, m_fStepPos);
	DDX_Text(pDX, IDC_EDIT6, m_iSpeed);
	DDX_Text(pDX, IDC_EDIT1, miMapStartX);
	DDX_Text(pDX, IDC_EDIT2, miMapStartY);
	DDX_Text(pDX, IDC_EDIT7, miMapEndX);
	DDX_Text(pDX, IDC_EDIT8, miMapEndY);
	DDX_Radio(pDX, IDC_NormalRadio, mScanType);
	DDX_Text(pDX, IDC_EDIT9, iScanSave);
	DDX_Text(pDX, IDC_PIUpTxt, mdPIUp);
	DDX_Text(pDX, IDC_PIDownTxt, mdPIDown);
	DDX_Text(pDX, IDC_PIStep, mdPIStep);
	DDX_Text(pDX, IDC_PIMove, mdPIMove);
	DDX_Radio(pDX, IDC_RADIOSTEP, m_iCtrlAxisMode);
}


BEGIN_MESSAGE_MAP(CScannerDlg, CDialogEx)
	ON_BN_CLICKED(IDC_ReBuildBtn, &CScannerDlg::OnBnClickedRebuildbtn)
	ON_BN_CLICKED(IDC_SetUpBtn, &CScannerDlg::OnBnClickedSetupbtn)
	ON_BN_CLICKED(IDC_SetDownBtn, &CScannerDlg::OnBnClickedSetdownbtn)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_AdvanceBtn, &CScannerDlg::OnBnClickedAdvancebtn)
	ON_BN_CLICKED(IDC_AutoFocusBtn, &CScannerDlg::OnBnClickedAutofocusbtn)
	ON_BN_CLICKED(IDC_ColorBuildBtn, &CScannerDlg::OnBnClickedColorbuildbtn)
	ON_BN_CLICKED(IDC_MapScanBtn, &CScannerDlg::OnBnClickedMapscanbtn)
	ON_BN_CLICKED(IDC_DefaultBtn, &CScannerDlg::OnBnClickedDefaultbtn)
	ON_BN_CLICKED(IDC_AutoFocusExBtn, &CScannerDlg::OnBnClickedAutofocusexbtn)
	ON_BN_CLICKED(IDC_ComtinueBtn, &CScannerDlg::OnBnClickedComtinuebtn)
	ON_BN_CLICKED(IDC_SetUpPIBtn, &CScannerDlg::OnBnClickedSetuppibtn)
	ON_BN_CLICKED(IDC_SetDownPIBtn, &CScannerDlg::OnBnClickedSetdownpibtn)
	ON_BN_CLICKED(IDC_ReBuildPIBtn, &CScannerDlg::OnBnClickedRebuildpibtn)
	ON_BN_CLICKED(IDC_UPPIBtn, &CScannerDlg::OnBnClickedUppibtn)
	ON_BN_CLICKED(IDC_DOWNPIBtn, &CScannerDlg::OnBnClickedDownpibtn)
	ON_BN_CLICKED(IDC_ResetPosBtn, &CScannerDlg::OnBnClickedResetposbtn)
	ON_BN_CLICKED(IDC_BtnCapture3D, &CScannerDlg::OnBnClickedBtncapture3d)
END_MESSAGE_MAP()


// CScannerDlg 消息处理程序

bool CScannerDlg::InitPanel( LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	if(m_ConfocalCore)
	{
    if (m_ConfocalCore->GetGearBox())
    {
      m_AxisZ = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Z);
      m_AxisPIZ = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_PIZ);
    }
	}		
	if (!m_AxisPIZ)
	{
		GetDlgItem(IDC_SetUpPIBtn)->EnableWindow(false);
		GetDlgItem(IDC_SetDownPIBtn)->EnableWindow(false);
		GetDlgItem(IDC_PIUpTxt)->EnableWindow(false);
		GetDlgItem(IDC_PIDownTxt)->EnableWindow(false);
		GetDlgItem(IDC_PIStep)->EnableWindow(false);
		GetDlgItem(IDC_ReBuildPIBtn)->EnableWindow(false);
		GetDlgItem(IDC_UPPIBtn)->EnableWindow(false);
		GetDlgItem(IDC_PIMove)->EnableWindow(false);
		GetDlgItem(IDC_DOWNPIBtn)->EnableWindow(false);
		GetDlgItem(IDC_ResetPosBtn)->EnableWindow(false);
		GetDlgItem(IDC_RADIOPI)->EnableWindow(false);
	}	

	m_ConfocalCore->GetDockablePanel(DOCKPANEL_MAPVIEWDLG)->Attach(this);
	return true;
}

int CScannerDlg::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if (ID==NOTIFY_MAPROISTART_FLUSH)
	{
		if (pParam!=0)
		{
			CPointF* pt=(CPointF*)pParam;
			miMapStartX=pt->x;
			miMapStartY=pt->y;
			UpdateData(FALSE);
		}
	}
	else if (ID==NOTIFY_MAPROIEND_FLUSH)
	{
		if (pParam!=0)
		{
			CPointF* pt=(CPointF*)pParam;
			miMapEndX=pt->x;
			miMapEndY=pt->y;
			UpdateData(FALSE);
		}
	}
	else if (ID==NOTIFY_PIZChange)
	{
		CString strV;
		strV.Format(L"PI Pos: %g mm\n",m_AxisPIZ->GetPosition());
		SetDlgItemText(IDC_PIPosTxt,strV);
	}
	return 0;
}

void CScannerDlg::OnBnClickedRebuildbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	//H3DScanner* tscan=(H3DScanner*)(m_ConfocalCore->GetPlugin(PLUGIN_3DScanner)->GetUserInterface());
	HConfocalPlug* tscan=m_ConfocalCore->GetPlugin(PLUGIN_3DScanner);
	UpdateData(TRUE);
	SaveParas();
	if(tscan)
	{
		ReBuildParas mReBuildParas;
		mReBuildParas.iScaleV=m_ConfFile.GetInt(L"3DRebuild",L"ScaleV",L"Confocal_Scaner");//尺度缩小
		mReBuildParas.iImgSize=m_ConfFile.GetInt(L"3DRebuild",L"ImgSize",L"Confocal_Scaner");//贴图大小
		mReBuildParas.iIdxSize=m_ConfFile.GetInt(L"3DRebuild",L"IdxSize",L"Confocal_Scaner");//高度图大小
		mReBuildParas.iImgStep=m_ConfFile.GetInt(L"3DRebuild",L"ImgStep",L"Confocal_Scaner");//高度图步长
		mReBuildParas.iSleep=m_ConfFile.GetInt(L"3DRebuild",L"iSleep",L"Confocal_Scaner");
		mReBuildParas.strBKImg=HGetStartPath() + L".//EpiResult//BGImg.bmp";
			//m_ConfFile.GetString(L"3DRebuild",L"strBKImg",L"Confocal_Scaner");
		if (mReBuildParas.strBKImg!=L"")
			mReBuildParas.strBKImg=m_ConfFile.GetModulePath()+mReBuildParas.strBKImg;
		mReBuildParas.bPIZ=false;
		if (m_fUpPos>m_fDownPos)
		{
			float mVPos=m_fDownPos;
			m_fDownPos=m_fUpPos;
			m_fUpPos=mVPos;
			UpdateData(false);
		}
		mReBuildParas.fUpPos=m_fUpPos;
		mReBuildParas.fStepPos=m_fStepPos;
		mReBuildParas.fDownPos=m_fDownPos;
		mReBuildParas.fSpeed=m_iSpeed;
		CButton* pCheck=(CButton*)GetDlgItem(IDC_SaveCheckBtn);
		mReBuildParas.bSaveImg=pCheck->GetCheck();
		tscan->Set(&mReBuildParas);
		CButton* pCheckBtn=(CButton*)GetDlgItem(IDC_ConfocalBtn);
		bool bConfocal=pCheckBtn->GetCheck();
		tscan->Start(&bConfocal);
	}	
}


void CScannerDlg::OnBnClickedSetupbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	float mV=m_AxisZ->GetPosition();
	m_fUpPos=mV;
	UpdateData(FALSE);
}


void CScannerDlg::OnBnClickedSetdownbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	float mV=m_AxisZ->GetPosition();
	m_fDownPos=mV;
	UpdateData(FALSE);
}


HBRUSH CScannerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_STATIC:
			pDC->SetBkMode(TRANSPARENT);
			break;
	}
	// TODO:  在此更改 DC 的任何特性
	return afxGlobalData.brBarFace;
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CScannerDlg::OnBnClickedAdvancebtn()
{
	// TODO: 在此添加控件通知处理程序代码
	CScannerParas dlg=new CScannerParas();
	dlg.DoModal();
}


void CScannerDlg::OnBnClickedAutofocusbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	mV.bRecord=((CButton*)GetDlgItem(IDC_SaveProBtn))->GetCheck();
	mV.strSavePath=m_ConfFile.GetString(L"SaveAutoFocus",L"strPath",L"Confocal_AutoFocus");
	CString strObject=m_ConfFile.GetString(L"Object",L"curobj",L"Confocal_Microscope");
	strObject=strObject+L"_AutoFocus";
	mV.mCuScale=m_ConfFile.GetDouble(strObject,L"mCuScale",L"Confocal_AutoFocus");
	mV.mDrStep=m_ConfFile.GetDouble(strObject,L"mDrStep",L"Confocal_AutoFocus");
	mV.mCuStep=m_ConfFile.GetDouble(strObject,L"mCuStep",L"Confocal_AutoFocus");
	mV.mXiStep=m_ConfFile.GetDouble(strObject,L"mXiStep",L"Confocal_AutoFocus");
	mV.mXiScale=m_ConfFile.GetDouble(strObject,L"mXiScale",L"Confocal_AutoFocus");
	mV.iSleepTime=m_ConfFile.GetInt(strObject,L"iSleepTime",L"Confocal_AutoFocus");
	mV.iZSpeed=m_ConfFile.GetDouble(strObject,L"iSpeed",L"Confocal_AutoFocus");
	mV.bDebug=false;
	HConfocalPlug* tscan=m_ConfocalCore->GetPlugin(PLUGIN_AutoFocus);
	tscan->Set(&mV);
	tscan->Start();
}

void CScannerDlg::OnBnClickedAutofocusexbtn()
{
	// TODO:
	UpdateData();
	mV.bRecord=((CButton*)GetDlgItem(IDC_SaveProBtn))->GetCheck();
	mV.bDebug=false;//mV.bRecord;

	mV.strSavePath=m_ConfFile.GetString(L"SaveAutoFocus",L"strPath",L"Confocal_AutoFocusEx");
	mV.UpLimit=m_ConfFile.GetDouble(L"Paras",L"UpLimit",L"Confocal_AutoFocusEx");
	mV.DownLimit=m_ConfFile.GetDouble(L"Paras",L"DownLimit",L"Confocal_AutoFocusEx");
	mV.ScanCount=m_ConfFile.GetInt(L"Paras",L"ScanCount",L"Confocal_AutoFocusEx");
	mV.PreCount=m_ConfFile.GetInt(L"Paras",L"PreCount",L"Confocal_AutoFocusEx");

	CString strObject=m_ConfFile.GetString(L"Object",L"curobj",L"Confocal_Microscope");
	strObject=strObject+L"_AutoFocus";

	mV.mDrStep=10;
	mV.mCuScale=m_ConfFile.GetDouble(strObject,L"mCuScale",L"Confocal_AutoFocusEx");
	mV.mCuStep=m_ConfFile.GetDouble(strObject,L"mCuStep",L"Confocal_AutoFocusEx");
	mV.mXiStep=m_ConfFile.GetDouble(strObject,L"mXiStep",L"Confocal_AutoFocusEx");
	mV.mXiScale=m_ConfFile.GetDouble(strObject,L"mXiScale",L"Confocal_AutoFocusEx");
	mV.iSleepTime=m_ConfFile.GetInt(strObject,L"iSleepTime",L"Confocal_AutoFocusEx");
	mV.iZSpeed=m_ConfFile.GetDouble(strObject,L"iSpeed",L"Confocal_AutoFocusEx");


	HConfocalPlug* tscan=m_ConfocalCore->GetPlugin(PLUGIN_AutoFocusEx);
	tscan->Set(&mV);
	tscan->Start();
}

void CScannerDlg::OnBnClickedColorbuildbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	HConfocalPlug* tscan=m_ConfocalCore->GetPlugin(PLUGIN_ColorScanner);
	tscan->Start();
}



void CScannerDlg::OnBnClickedMapscanbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	m_ConfocalCore->GetCurVideo()->SetMirror(true);
	UpdateData();
	mMapBuildParasV.MaxPt.x=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_X)->GetAxialInfo()->sMaxPos;
	mMapBuildParasV.MaxPt.y=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Y)->GetAxialInfo()->sMaxPos;
	mMapBuildParasV.MinPt.x=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_X)->GetAxialInfo()->sMinPos;
	mMapBuildParasV.MinPt.y=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Y)->GetAxialInfo()->sMinPos;
	mMapBuildParasV.EndPt.x=miMapEndX;
	mMapBuildParasV.EndPt.y=miMapEndY;
	mMapBuildParasV.StartPt.x=miMapStartX;
	mMapBuildParasV.StartPt.y=miMapStartY;
	if (mMapBuildParasV.EndPt.x<mMapBuildParasV.StartPt.x)
	{
		float mMiddleV=mMapBuildParasV.EndPt.x;
		mMapBuildParasV.StartPt.x=mMiddleV;
		mMapBuildParasV.EndPt.x=mMapBuildParasV.StartPt.x;
	}
	if (mMapBuildParasV.EndPt.y<mMapBuildParasV.StartPt.y)
	{
		float mMiddleV=mMapBuildParasV.EndPt.y;
		mMapBuildParasV.StartPt.y=mMiddleV;
		mMapBuildParasV.EndPt.y=mMapBuildParasV.StartPt.y;
	}
	if (mMapBuildParasV.EndPt.x>mMapBuildParasV.MaxPt.x)
	{
		mMapBuildParasV.EndPt.x=mMapBuildParasV.MaxPt.x;
		miMapEndX=mMapBuildParasV.EndPt.x;
		UpdateData(false);
	}
	if (mMapBuildParasV.EndPt.y>mMapBuildParasV.MaxPt.y)
	{
		mMapBuildParasV.EndPt.y=mMapBuildParasV.MaxPt.y;
		miMapEndY=mMapBuildParasV.EndPt.y;
		UpdateData(false);
	}
	mMapBuildParasV.ImageROISize=m_ConfFile.GetDouble(L"Size",L"ImageROISize",L"Confocal_Map");
	mMapBuildParasV.TempleSize=mMapBuildParasV.ImageROISize;
	mMapBuildParasV.m_overlap=m_ConfFile.GetDouble(L"Size",L"OverlapSize",L"Confocal_Map");
	mMapBuildParasV.m_rows=mMapBuildParasV.m_cols=mMapBuildParasV.m_overlap+mMapBuildParasV.TempleSize;

	CString strObject=m_ConfFile.GetString(L"Object",L"curobj",L"Confocal_Microscope");
	CString strV=strObject+L"_mm2pixel";
	mMapBuildParasV.PixelSize=m_ConfFile.GetDouble(L"PixelSize",strV,L"Confocal_Microscope");
	mMapBuildParasV.MapSavePath=m_ConfFile.GetString(L"DebugSave",L"MapSavePath",L"Confocal_Map");//存MapImg.jpg路径
	strV=strObject+L"_ScanSpeed";
	mMapBuildParasV.iScanSpeed=m_ConfFile.GetDouble(L"Stage",strV,L"Confocal_Map");
	strV=strObject+L"_SleepTime";
	mMapBuildParasV.iSleepTime=m_ConfFile.GetInt(L"Stage",strV,L"Confocal_Map");
	mMapBuildParasV.iMapSizeX=m_ConfFile.GetInt(L"Size",L"MapSizeX",L"Confocal_Map");
	mMapBuildParasV.bMapSave=((CButton*)GetDlgItem(IDC_SaveMapBtn))->GetCheck();//是否保存图片MapImg.jpg
	mMapBuildParasV.bROIImgSave=((CButton*)GetDlgItem(IDC_SaveROIBtn))->GetCheck();
	mMapBuildParasV.mScanType=(SCAN_TYPE)mScanType;

	if (mMapBuildParasV.mScanType==Scan_Precision)
	{
		//初始化调焦的系统
		mV.bRecord=((CButton*)GetDlgItem(IDC_SaveProBtn))->GetCheck();
		mV.strSavePath=m_ConfFile.GetString(L"SaveAutoFocus",L"strPath",L"Confocal_AutoFocus");
		CString strObject=m_ConfFile.GetString(L"Object",L"curobj",L"Confocal_Microscope");
		strObject=strObject+L"_AutoFocus";
		mV.mCuScale=m_ConfFile.GetDouble(strObject,L"mCuScale",L"Confocal_AutoFocus");
		mV.mDrStep=m_ConfFile.GetDouble(strObject,L"mDrStep",L"Confocal_AutoFocus");
		mV.mCuStep=m_ConfFile.GetDouble(strObject,L"mCuStep",L"Confocal_AutoFocus");
		mV.mXiStep=m_ConfFile.GetDouble(strObject,L"mXiStep",L"Confocal_AutoFocus");
		mV.mXiScale=m_ConfFile.GetDouble(strObject,L"mXiScale",L"Confocal_AutoFocus");
		mV.iSleepTime=m_ConfFile.GetInt(strObject,L"iSleepTime",L"Confocal_AutoFocus");
		mV.iZSpeed=m_ConfFile.GetInt(strObject,L"iSpeed",L"Confocal_AutoFocus");
		mV.bDebug=false;

		mV.UpLimit=m_ConfFile.GetDouble(L"Paras",L"UpLimit",L"Confocal_AutoFocusEx");
		mV.DownLimit=m_ConfFile.GetDouble(L"Paras",L"DownLimit",L"Confocal_AutoFocusEx");
		mV.ScanCount=m_ConfFile.GetInt(L"Paras",L"ScanCount",L"Confocal_AutoFocusEx");
		mV.PreCount=m_ConfFile.GetInt(L"Paras",L"PreCount",L"Confocal_AutoFocusEx");

		//m_ConfocalCore->GetPlugin(PLUGIN_AutoFocus)->Set(&mV);
		m_ConfocalCore->GetPlugin(PLUGIN_AutoFocusEx)->Set(&mV);
	}

	HConfocalPlug* tscan=m_ConfocalCore->GetPlugin(PLUGIN_MAP);
	tscan->Set(&mMapBuildParasV);
	tscan->Start();
}


void CScannerDlg::OnBnClickedDefaultbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	miMapEndX=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_X)->GetAxialInfo()->sMaxPos;
	miMapEndY=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Y)->GetAxialInfo()->sMaxPos;
	miMapStartX=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_X)->GetAxialInfo()->sMinPos;
	miMapStartY=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Y)->GetAxialInfo()->sMinPos;
	UpdateData(false);
}


void CScannerDlg::OnBnClickedComtinuebtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	HConfocalPlug* tscan=m_ConfocalCore->GetPlugin(PLUGIN_ContinueScan);
	tscan->Set(&iScanSave);
	tscan->Start();
}


void CScannerDlg::OnBnClickedSetuppibtn()
{
	// TODO: 在此添加控件通知处理程序代码
	mdPIUp=m_AxisPIZ->GetPosition();
	UpdateData(false);
}


void CScannerDlg::OnBnClickedSetdownpibtn()
{
	// TODO: 在此添加控件通知处理程序代码
	mdPIDown=m_AxisPIZ->GetPosition();
	UpdateData(false);
}


void CScannerDlg::OnBnClickedRebuildpibtn()
{
	// TODO: 在此添加控件通知处理程序代码
	HConfocalPlug* tscan=m_ConfocalCore->GetPlugin(PLUGIN_3DScanner);
	UpdateData(TRUE);
	SaveParas();
	if(tscan)
	{
		tscan->Attach(this);
		ReBuildParas mReBuildParas;
		mReBuildParas.iScaleV=m_ConfFile.GetInt(L"3DRebuild",L"ScaleV",L"Confocal_Scaner");//尺度缩小
		mReBuildParas.iImgSize=m_ConfFile.GetInt(L"3DRebuild",L"ImgSize",L"Confocal_Scaner");//贴图大小
		mReBuildParas.iIdxSize=m_ConfFile.GetInt(L"3DRebuild",L"IdxSize",L"Confocal_Scaner");//高度图大小
		mReBuildParas.iImgStep=m_ConfFile.GetInt(L"3DRebuild",L"ImgStep",L"Confocal_Scaner");//高度图步长
		mReBuildParas.iSleep=m_ConfFile.GetInt(L"3DRebuild",L"iSleep",L"Confocal_Scaner");
		mReBuildParas.strBKImg= HGetStartPath() + L".//EpiResult//BGImg.bmp";
			//m_ConfFile.GetString(L"3DRebuild",L"strBKImg",L"Confocal_Scaner");
		if (mReBuildParas.strBKImg!=L"")
			mReBuildParas.strBKImg=m_ConfFile.GetModulePath()+mReBuildParas.strBKImg;
		mReBuildParas.bPIZ=true;
		if (mdPIUp>mdPIDown)
		{
			double mVPos=mdPIUp;
			mdPIUp=mdPIDown;
			mdPIDown=mVPos;
			UpdateData(false);
		}
		mReBuildParas.fUpPos=mdPIUp;
		mReBuildParas.fStepPos=mdPIStep;
		mReBuildParas.fDownPos=mdPIDown;
		mReBuildParas.fSpeed=m_iSpeed;
		CButton* pCheck=(CButton*)GetDlgItem(IDC_SaveCheckBtn);
		mReBuildParas.bSaveImg=pCheck->GetCheck();
		tscan->Set(&mReBuildParas);
		CButton* pCheckBtn=(CButton*)GetDlgItem(IDC_ConfocalBtn);
		bool bConfocal=pCheckBtn->GetCheck();
		tscan->Start(&bConfocal);
	}	
}


void CScannerDlg::OnBnClickedUppibtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_AxisPIZ)
		m_AxisPIZ->MoveRef(-mdPIMove);
	CString strV;
	strV.Format(L"PI Pos: %g mm\n",m_AxisPIZ->GetPosition());
	SetDlgItemText(IDC_PIPosTxt,strV);
}


void CScannerDlg::OnBnClickedDownpibtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_AxisPIZ)
		m_AxisPIZ->MoveRef(mdPIMove);
	CString strV;
	//strV.Format(L"PI Pos: %g mm\n",m_AxisPIZ->GetPosition());
	strV.Format(L"PI Pos: %g mm\n",m_AxisPIZ->GetPosition());
	SetDlgItemText(IDC_PIPosTxt,strV);
}


void CScannerDlg::OnBnClickedResetposbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_AxisPIZ)
		m_AxisPIZ->MoveRef(0);
	CString strV;
	strV.Format(L"PI Pos: %g mm\n",m_AxisPIZ->GetPosition());
	SetDlgItemText(IDC_PIPosTxt,strV);
}


void CScannerDlg::OnBnClickedBtncapture3d()
{
	// TODO: 在此添加控件通知处理程序代码
	HConfocalPlug* tscan = m_ConfocalCore->GetPlugin(PLUGIN_3DCapture);
	UpdateData(TRUE);
	SaveParas();
	if (tscan)
	{
		ReBuildParas mReBuildParas;
		mReBuildParas.iSleep = m_ConfFile.GetInt(L"3DRebuild", L"iSleep", L"Confocal_Scaner");
		if (m_fUpPos > m_fDownPos)
		{
			float mVPos = m_fDownPos;
			m_fDownPos = m_fUpPos;
			m_fUpPos = mVPos;
			UpdateData(false);
		}

		if (m_iCtrlAxisMode == 1)
		{
			mReBuildParas.bPIZ = true;
			mReBuildParas.fUpPos = mdPIUp;
			mReBuildParas.fStepPos = mdPIStep;
			mReBuildParas.fDownPos = mdPIDown;
			mReBuildParas.fSpeed = m_iSpeed;
		}	
		else
		{
			mReBuildParas.bPIZ = false;
			mReBuildParas.fUpPos = m_fUpPos;
			mReBuildParas.fStepPos = m_fStepPos;
			mReBuildParas.fDownPos = m_fDownPos;
			mReBuildParas.fSpeed = m_iSpeed;
		}

		tscan->Set(&mReBuildParas);
		CButton* pCheckBtn = (CButton*)GetDlgItem(IDC_ConfocalBtn);
		bool bConfocal = pCheckBtn->GetCheck();
		tscan->Start(&bConfocal);
	}
}
