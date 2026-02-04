#include "StdAfx.h"
#include "ConfocalCoreModule.h"
#include "afxdialogex.h"


// 对话框

IMPLEMENT_DYNAMIC(ConfocalCoreModule, CDialogEx)

ConfocalCoreModule::ConfocalCoreModule(CWnd* pParent /*=NULL*/)
	: CDialogEx(ConfocalCoreModule::IDD, pParent)
	, mMsgShow(_T(""))
{
	for (int i=0;i<MAX_PANEL_NUM;i++)
	{
		m_CoreViewPanel[i]=0;
		m_CoreDockPanel[i]=0;
		m_CorePlugin[i]=0;
		m_CoreProcess[i]=0;
		m_CoreMsgPanel[i]=0;
	}
	m_VideoManageEx = 0;
	m_ConfocalVideoManage = 0;
	m_CurVideoDevice=0;
	m_VideoDeviceEx = 0;
	m_DmdManager=0;
	m_GearBox=0;
	m_EasyF=0;
	_bmp=0;
	mProcessMg = 0;
	mVideoType = 0;
	VERIFY(font.CreateFont(
		20,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_NORMAL,                 // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		L"Arial"));
		//L"楷体"));                 // lpszFacename
	pBrush=CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));
}

ConfocalCoreModule::~ConfocalCoreModule()
{
}

void ConfocalCoreModule::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_Status, mMsgShow);
}

BEGIN_MESSAGE_MAP(ConfocalCoreModule, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_PAINT()
END_MESSAGE_MAP()

//初始化各种设备
bool ConfocalCoreModule::Init(LPVOID p_param/* =0 */)
{
	if (p_param==0)//初始化东西
	{
		HMODULE  mPreModule=GetModuleHandle(0);//记录当前的模块地址，结尾处帮它替换过来！
		AfxSetResourceHandle(GetModuleHandle(L"ConfocalCore.dll"));
		//AfxSetResourceHandle(mParentModule);//使用主Dll的theApp.m_hInstance 启动画面没有显示出来！

		mTxtclr=RGB(255,255,255);
		Confocal_ConfigSingleton::instance().Load(L"Confocal_Config");
		_bmp = new Bitmap(Confocal_ConfigSingleton::instance().m_strPlash);	
		if (_bmp)
		{
			VERIFY(Create());
			CRect rc;
			GetWindowRect(rc);
			SetWindowPos(0,rc.left,rc.top,_bmp->GetWidth(),_bmp->GetHeight(),SWP_NOZORDER);
		}
		PrintMessage(L"Init the Video plug……");
		int iStopTime=3000;

    HConfigure* t_configure = this->GetConfigure();

		//////以下的两种方式是存在不显示问题，J光切割的闪屏放在View类当中，而这边直接放在了App的初始化接口当中，不能显示
		////_splash.Create(IDD_DLGFLASH);
		////_splash.SetPath(L".\\splash.png");
		////_splash.ShowWindow(SW_SHOW);
		////m_Splash.Show(); 
		////m_Splash.Print(L"Init the Video plug……");

    int video_module = t_configure->GetInt(L"Module", L"Video", L"Confocal_Module");
    if (video_module)
    {
      //1、初始化相机
      mVideoType = (HVideoType*)LoadDlls(L"VideoPlugin.dll");//加入库
      if (mVideoType == 0)
      {
        //AfxMessageBox(L"没有发现相机插件！");
        //m_Splash.Print(L"Video plug is not exist!");
        mTxtclr = RGB(255, 255, 0);
        PrintMessage(L"Video plug is not exist!");
        Sleep(iStopTime);
        return false;//返回
      }
      int iCount = mVideoType->FindCameraType();//查找了所有相机类型
      if (iCount <= 0)
      {
        //AfxMessageBox(L"没有发现相机！");
        //m_Splash.Print(L"Video is not exist!");
        mTxtclr = RGB(255, 255, 0);
        PrintMessage(L"Video is not exist!");
        Sleep(iStopTime);
        //return false;//返回
      }
	  if (iCount == 1)
	  {
		  //m_Splash.Print(L"Open the Video……");
		  PrintMessage(L"Open the Video……");
		  m_ConfocalVideoManage = mVideoType->GetCameraType(0)->mVideoManager;
		  int tcur = m_ConfocalVideoManage->GetCameraCount();
		  if (tcur <= 0)
			  m_ConfocalVideoManage->SearchCameras();
		  if (tcur == 1)
		  {
			  m_CurVideoDevice = m_ConfocalVideoManage->OpenCamera(0);
		  }
		  else if (tcur > 1)
		  {
			  m_VideoManageEx	= m_ConfocalVideoManage;
			  m_CurVideoDevice	= m_ConfocalVideoManage->OpenCamera(0);
			  m_VideoDeviceEx	= m_ConfocalVideoManage->OpenCamera(1);
		  }
	  }
	  else if (iCount > 1)
	  {
		  PrintMessage(L"Open the Videos1……");
		  m_ConfocalVideoManage = mVideoType->GetCameraType(L"HamamatsuCamera")->mVideoManager;
		  m_VideoManageEx       = mVideoType->GetCameraType(L"TucsenCamera")->mVideoManager;
		  if (m_ConfocalVideoManage && m_VideoManageEx)
		  {
			  //m_ConfocalVideoManage = mVideoType->GetCameraType(L"HamamatsuCamera")->mVideoManager;
			  int tcur = m_ConfocalVideoManage->GetCameraCount();
			  if (tcur <= 0)
				  m_ConfocalVideoManage->SearchCameras();
			  m_CurVideoDevice = m_ConfocalVideoManage->OpenCamera(0);

			  //m_VideoManageEx = mVideoType->GetCameraType(L"TucsenCamera")->mVideoManager;
			  tcur = m_VideoManageEx->GetCameraCount();
			  if (tcur <= 0)
				  m_VideoManageEx->SearchCameras();
			  m_VideoDeviceEx = m_VideoManageEx->OpenCamera(0);
		  }
	  }
    }

    int dmd_module = t_configure->GetInt(L"Module", L"DMD", L"Confocal_Module");
    if (dmd_module)
    {
      //m_Splash.Print(L"Init the DMD……");
      PrintMessage(L"Init the DMD……");
      //2、初始化DMD
      m_DmdManager = (IDMDManager*)LoadDlls(L"WlpDMDLib.dll");
      if (m_DmdManager)
      {
		  /*mWlpDMDParas.delay = t_configure->GetInt(L"DMDParas", L"delay", L"Confocal_DMDControl");
        mWlpDMDParas.gray = t_configure->GetInt(L"DMDParas", L"gray", L"Confocal_DMDControl");
        mWlpDMDParas.m_FrameInterval = t_configure->GetInt(L"DMDParas", L"m_FrameInterval", L"Confocal_DMDControl");
        mWlpDMDParas.m_FrameMargin = t_configure->GetInt(L"DMDParas", L"m_FrameMargin", L"Confocal_DMDControl");
        mWlpDMDParas.plusewidth = t_configure->GetInt(L"DMDParas", L"plusewidth", L"Confocal_DMDControl");
        mWlpDMDParas.polay = true;
        mWlpDMDParas.trigeMode = 0x00000006;
        mWlpDMDParas.trigeSync = 0x00000008;
        if (m_DmdManager->InitDMD(&mWlpDMDParas, 1))
          TRACE(L"Init DMD success!");
        else
        {
          m_DmdManager->UnInitDMD();
          PrintMessage(L"Init the DMD failed!");
          Sleep(iStopTime);
        }*/
      }
    }

    int stage_module = t_configure->GetInt(L"Module", L"Stage", L"Confocal_Module");
    if (stage_module)
    {
      //m_Splash.Print(L"Init the stage……");
      PrintMessage(L"Init the stage……");
      //3、初始化自动化载物台
      CString mStageName = t_configure->GetString(L"Plugin", L"Name", L"Confocal_Stage");
      m_GearBox = (HGearBox*)LoadDlls(mStageName + L".stg");
      if (!m_GearBox)
        return false;//没有找到，返回
      int x_mm2count = t_configure->GetDouble(L"Paras_" + mStageName, L"x_mm2count", L"Confocal_Stage");
      int y_mm2count = t_configure->GetDouble(L"Paras_" + mStageName, L"y_mm2count", L"Confocal_Stage");
      int z_mm2count = t_configure->GetDouble(L"Paras_" + mStageName, L"z_mm2count", L"Confocal_Stage");

      float x_mm2Encoder = t_configure->GetDouble(L"Paras_" + mStageName, L"x_mm2Encoder", L"Confocal_Stage");
      float y_mm2Encoder = t_configure->GetDouble(L"Paras_" + mStageName, L"y_mm2Encoder", L"Confocal_Stage");
      float z_mm2Encoder = t_configure->GetDouble(L"Paras_" + mStageName, L"z_mm2Encoder", L"Confocal_Stage");

      double x_limit = t_configure->GetDouble(L"Paras_" + mStageName, L"x_limit", L"Confocal_Stage");
      double y_limit = t_configure->GetDouble(L"Paras_" + mStageName, L"y_limit", L"Confocal_Stage");
      double z_limit = t_configure->GetDouble(L"Paras_" + mStageName, L"z_limit", L"Confocal_Stage");
      bool zInvert = t_configure->GetBool(L"Paras_" + mStageName, L"z_Invert", L"Confocal_Stage");
      bool yInvert = t_configure->GetBool(L"Paras_" + mStageName, L"y_Invert", L"Confocal_Stage");
      bool xInvert = t_configure->GetBool(L"Paras_" + mStageName, L"x_Invert", L"Confocal_Stage");

      bool zEncoderInvert = t_configure->GetBool(L"Paras_" + mStageName, L"z_EncoderInvert", L"Confocal_Stage");
      bool yEncoderInvert = t_configure->GetBool(L"Paras_" + mStageName, L"y_EncoderInvert", L"Confocal_Stage");
      bool xEncoderInvert = t_configure->GetBool(L"Paras_" + mStageName, L"x_EncoderInvert", L"Confocal_Stage");

      float mXspeed = t_configure->GetInt(L"Paras_" + mStageName, L"XSpeed", L"Confocal_Stage");
      float mYspeed = t_configure->GetInt(L"Paras_" + mStageName, L"YSpeed", L"Confocal_Stage");
      float mZspeed = t_configure->GetDouble(L"Paras_" + mStageName, L"ZSpeed", L"Confocal_Stage");
      int iAxisCount = t_configure->GetInt(L"Paras_" + mStageName, L"AxisCount", L"Confocal_Stage");
      int iType = 8, iLimit = 999;
      AxisInfo t[7] = { {AXIS_Z,'Z',0.0,z_limit,z_mm2count,x_mm2Encoder,mZspeed,0,0,zInvert,zEncoderInvert,false},
          {AXIS_Y,'Y',0.0,y_limit,y_mm2count,y_mm2Encoder,mYspeed,0,0,yInvert,yEncoderInvert,false},//28lim
          {AXIS_X,'X',0.0,x_limit,x_mm2count,z_mm2Encoder,mXspeed,0,0,xInvert,xEncoderInvert,false},//85 lim
          {AXIS_PIZ,'N',0.0,0,1000,1000,0,0,0,0,0,false},
          {AXIS_O,'O',0.0,iLimit,0,0,0,0,iType,true,true,false},
          {AXIS_T,'T',0.0,10,1,1,500,0,0,true,true,false},
          {AXIS_W,'W',0.0,10,1,1,500,0,0,true,true,false} };
      if (mStageName == L"HdsStage")
      {
        HdsStageParas mHdsParas;
        mHdsParas.strIP = t_configure->GetString(L"Paras_" + mStageName, L"Ip", L"Confocal_Stage");
        mHdsParas.iWCom = t_configure->GetInt(L"Paras_" + mStageName, L"iWCom", L"Confocal_Stage");
        mHdsParas.iZCom = t_configure->GetInt(L"Paras_" + mStageName, L"iZCom", L"Confocal_Stage");
        if (!m_GearBox->InitGearBox(L"Carl's GearBox", &mHdsParas, t, 4))
        {
          //AfxMessageBox(L"初始化自动轴失败！");
          PrintMessage(L"Init the stage failed!");
          Sleep(iStopTime);
          //return false;
        }
      }
      else if (mStageName == L"MoticStage")
      {
        int mCom = t_configure->GetInt(L"Paras_" + mStageName, L"Port", L"Confocal_Stage");
        MoticStageParas mParas;
        mParas.iAxisCom = t_configure->GetInt(L"Paras_" + mStageName, L"Port", L"Confocal_Stage");
        mParas.iTurnCom = t_configure->GetInt(L"Paras_" + mStageName, L"TPort", L"Confocal_Stage");
        if (!m_GearBox->InitGearBox(L"Carl's GearBox", &mParas, t, iAxisCount))
        {
          //AfxMessageBox(L"初始化自动轴失败！");
          PrintMessage(L"Init the stage failed!");
          Sleep(iStopTime);
          //return false;
        }
      }
    }

    int imgproc_module = t_configure->GetInt(L"Module", L"ImgProc", L"Confocal_Module");
    if (imgproc_module)
    {
      PrintMessage(L"Init the image process lib……");
      //4、初始化图像处理库
      mProcessMg = 0;
      mProcessMg = (HCoreProcessMg*)LoadDlls(L"ImgProcessPlug.pro");
      if (!mProcessMg)
      {
        //AfxMessageBox(L"初始化图像处理库失败！");
        PrintMessage(L"Init the image process lib failed!");
        Sleep(iStopTime);
        //return false;
      }
    }

		PrintMessage(L"Init other plugs……");
		//5、初始化其他插件
		GetPlugin(PLUGIN_CONFOCALMODE)->InitPlugin(this);
		bool bValue = true;
		GetPlugin(PLUGIN_CONFOCALMODEEX)->InitPlugin(this, &bValue);
		GetPlugin(PLUGIN_3DScanner)->InitPlugin(this);
		GetPlugin(PLUGIN_3DCapture)->InitPlugin(this);
		GetPlugin(PLUGIN_ColorScanner)->InitPlugin(this);
		GetPlugin(PLUGIN_MAP)->InitPlugin(this);
		GetPlugin(PLUGIN_AutoFocus)->InitPlugin(this);
		GetPlugin(PLUGIN_AutoFocusEx)->InitPlugin(this);
		GetPlugin(PLUGIN_ContinueScan)->InitPlugin(this);
		GetPlugin(PLUGIN_SIMODE)->InitPlugin(this);
		GetPlugin(PLUGIN_DIFFMEASURE)->InitPlugin(this);
		GetPlugin(PLUGIN_DIFFMEASUREPRO)->InitPlugin(this);
		GetPlugin(PLUGIN_AutoFocusDAC)->InitPlugin(this);
		GetCoreProcess(PROCESS_CalValue);//加载CV计算库,它会读取已经存好的CV计算矩阵

		//m_Splash.Print(L"Start sucess！");
		PrintMessage(L"Start sucess！");
		Sleep(1000);
		//m_Splash.DestroyWnd();
		DestroyWindow();
		//InitLog();
		//long  iScaleV=t_configure->GetInt(L"3DRebuild",L"ScaleV",L"Confocal_Scaner");//尺度缩小
		//int iImgSize=t_configure->GetInt(L"3DRebuild",L"ImgSize",L"Confocal_Scaner");//贴图大小
		//int iIdxSize=t_configure->GetInt(L"3DRebuild",L"IdxSize",L"Confocal_Scaner");//高度图大小
		//int iImgStep=t_configure->GetInt(L"3DRebuild",L"ImgStep",L"Confocal_Scaner");//高度图步长
		//ReBuildParas mV={iScaleV,iImgSize,iIdxSize,iImgStep};
		//GetCoreProcess(PROCESS_3DReBuild)->Init(&mV);
		AfxSetResourceHandle(mPreModule);

		InitViewCfg();
	}
	else//初始化窗口
	{		
		//GetDockablePanel(DOCKPANEL_VIDEO)->InitPanel(m_CurVideoDevice);	

		bool bValue = true;
		//AfxMessageBox(L"初始化相机控制面板！");
		GetDockablePanel(DOCKPANEL_VIDEO)->InitPanel(this);//初始化相机控制面板	
		GetDockablePanel(DOCKPANEL_VIDEOEX)->InitPanel(this, &bValue);
		//AfxMessageBox(L"初始化dmd控制面板！");
		GetDockablePanel(DOCKPANEL_DMDCTL)->InitPanel(this);//dmd控制面板
		//AfxMessageBox(L"初始化设置载物台！");
		GetDockablePanel(DOCKPANEL_STAGECTL)->InitPanel(this);//设置载物台
		//AfxMessageBox(L"初始化SCANERDLG！");
		GetDockablePanel(DOCKPANEL_SCANERDLG)->InitPanel(this);
		//AfxMessageBox(L"初始化MAPVIEWDLG！");
		GetDockablePanel(DOCKPANEL_MAPVIEWDLG)->InitPanel(this);//
		GetDockablePanel(DOCKPANEL_DIFF)->InitPanel(this); //初始化差动测量面板

		if (m_stViewCfg.ViewShow[VIEWPANEL_REALTIME])
			GetViewPanel(VIEWPANEL_REALTIME)->InitPanel(this);//实时view窗口
		if (m_stViewCfg.ViewShow[VIEWPANEL_REALTIMEEX])
			GetViewPanel(VIEWPANEL_REALTIMEEX)->InitPanel(this,&bValue);//实时view窗口
		if (m_stViewCfg.ViewShow[VIEWPANEL_CONFOCAL])
			GetViewPanel(VIEWPANEL_CONFOCAL)->InitPanel(this);
		if (m_stViewCfg.ViewShow[VIEWPANEL_CONFOCALEX])
			GetViewPanel(VIEWPANEL_CONFOCALEX)->InitPanel(this, &bValue);
		//GetViewPanel(VIEWPANEL_3D)->InitPanel(this);
		if (m_stViewCfg.ViewShow[VIEWPANEL_ViewFocus])
			GetViewPanel(VIEWPANEL_ViewFocus)->InitPanel(this);
		if (m_stViewCfg.ViewShow[VIEWPANEL_3DRange])
			GetViewPanel(VIEWPANEL_3DRange)->InitPanel(this);
		if (m_stViewCfg.ViewShow[VIEWPANEL_SI])
			GetViewPanel(VIEWPANEL_SI)->InitPanel(this);
		if (m_stViewCfg.ViewShow[VIEWPANEL_DIFF_MEASURE_VIEW])
			GetViewPanel(VIEWPANEL_DIFF_MEASURE_VIEW)->InitPanel(this);

		//设置相机
		if (m_CurVideoDevice)
		{
			m_CurVideoDevice->SetRender(&m_RenderChain);//设置渲染
			float texposure=GetConfigure()->GetDouble(L"Video",L"exposure",L"Confocal_Camera");
			if(texposure>-1.0)
				m_CurVideoDevice->SetExposure(texposure);
			bool iBool=GetConfigure()->GetBool(L"Video",L"bMirror",L"Confocal_Camera");
			m_CurVideoDevice->SetMirror(iBool);
			iBool=GetConfigure()->GetBool(L"Video",L"bFlip",L"Confocal_Camera");

			int iX=GetConfigure()->GetInt(L"Video",L"BinningPt1X",L"Confocal_Camera");
			int iY=GetConfigure()->GetInt(L"Video",L"BinningPt1Y",L"Confocal_Camera");
			m_CurVideoDevice->SetBinningPt(1,CPoint(iX,iY));
			iX=GetConfigure()->GetInt(L"Video",L"BinningPt2X",L"Confocal_Camera");
			iY=GetConfigure()->GetInt(L"Video",L"BinningPt2Y",L"Confocal_Camera");
			m_CurVideoDevice->SetBinningPt(2,CPoint(iX,iY));

			m_CurVideoDevice->SetFlip(iBool);
			m_CurVideoDevice->Run();
		}

		if (m_VideoDeviceEx)
		{
			m_VideoDeviceEx->SetRender(&m_RenderChainEx);//设置渲染
			float texposure = GetConfigure()->GetDouble(L"Video", L"exposure", L"Confocal_Camera");
			if (texposure > -1.0)
				m_VideoDeviceEx->SetExposure(texposure);
			bool iBool = GetConfigure()->GetBool(L"Video", L"bMirror", L"Confocal_Camera");
			m_VideoDeviceEx->SetMirror(iBool);
			iBool = GetConfigure()->GetBool(L"Video", L"bFlip", L"Confocal_Camera");

			int iX = GetConfigure()->GetInt(L"Video", L"BinningPt1X", L"Confocal_Camera");
			int iY = GetConfigure()->GetInt(L"Video", L"BinningPt1Y", L"Confocal_Camera");
			m_VideoDeviceEx->SetBinningPt(1, CPoint(iX, iY));
			iX = GetConfigure()->GetInt(L"Video", L"BinningPt2X", L"Confocal_Camera");
			iY = GetConfigure()->GetInt(L"Video", L"BinningPt2Y", L"Confocal_Camera");
			m_VideoDeviceEx->SetBinningPt(2, CPoint(iX, iY));

			m_VideoDeviceEx->SetFlip(iBool);
			m_VideoDeviceEx->Run();
		}
	}
	return true;
}

//注销各个设备
bool ConfocalCoreModule::Uninit(LPVOID p_param/* =0 */)
{
	if (m_CurVideoDevice)
		m_CurVideoDevice->SetRender(0);//清空渲染

	if (m_VideoDeviceEx)
		m_VideoDeviceEx->SetRender(0);//清空渲染

	//关闭相机
	int iTypeCount= mVideoType ? mVideoType->GetTypeCount() : 0;
	for (int i=0;i<iTypeCount;i++)
	{
		int iDeviceCount = mVideoType->GetCameraType(i)->mVideoManager->GetCameraCount();
		for (int k = 0; k < iDeviceCount; k++)
		{
			mVideoType->GetCameraType(i)->mVideoManager->GetCamerDevice(k)->CloseCamera();
		}
	}
	//清理资源
	for (int i=0;i<MAX_PANEL_NUM;i++)
	{
		if(m_CorePlugin[i])
		{
			m_CorePlugin[i]->UnInitPlugin();
			m_CorePlugin[i]=0;
			//delete(m_CorePlugin[i]);
		}			
		if (m_CoreProcess[i])
		{
			m_CoreProcess[i]->UnInitProcess();
			m_CoreProcess[i]=0;
			//delete(m_CoreProcess[i]);
		}
		if (m_CoreDockPanel[i])
		{
			m_CoreDockPanel[i]->UnInitPanel();
			m_CoreDockPanel[i]=0;
			//delete(m_CoreDockPanel[i]);
		}
		if (m_CoreViewPanel[i])
		{
			m_CoreViewPanel[i]->UnInitPanel();
			m_CoreViewPanel[i]=0;
			//delete(m_CoreViewPanel[i]);
		}
		if (m_CoreMsgPanel[i])
		{
			m_CoreMsgPanel[i]->UnInitPanel();
			m_CoreMsgPanel[i]=0;
			//delete(m_CoreMsgPanel[i]);
		}
	}
	////关闭相机
	//int iTypeCount=mVideoType->GetTypeCount();
	//for (int i=0;i<iTypeCount;i++)
	//{
	//	int iMgCount=mVideoType->GetCameraType(i)->mVideoManager->GetCameraCount();
	//	for (int j=0;j<iMgCount;j++)
	//	{
	//		int iDeviceCount=mVideoType->GetCameraType(i)->mVideoManager->GetCameraCount();
	//		for (int k=0;k<iDeviceCount;k++)
	//		{
	//			//mVideoType->GetCameraType(i)->mVideoManager->GetCamerDevice(k)->Pause();
	//			mVideoType->GetCameraType(i)->mVideoManager->GetCamerDevice(k)->CloseCamera();
	//		}
	//	}
	//}
	//关闭载物台
	if (m_GearBox)
		m_GearBox->UnInit();
	mLogLib.ShutdownLogging();//关闭日志
	return true;
}

bool ConfocalCoreModule::InitViewCfg()
{
	m_stViewCfg.Count = 0;
	HConfigure* t_configure = this->GetConfigure();
	m_stViewCfg.ViewShow[VIEWPANEL_REALTIME] = t_configure->GetInt(L"Module", L"VIEWPANEL_REALTIME", L"Confocal_Module");
	if (m_stViewCfg.ViewShow[VIEWPANEL_REALTIME])
		m_stViewCfg.Count++;

	m_stViewCfg.ViewShow[VIEWPANEL_CONFOCAL] = t_configure->GetInt(L"Module", L"VIEWPANEL_CONFOCAL", L"Confocal_Module");
	if (m_stViewCfg.ViewShow[VIEWPANEL_CONFOCAL])
		m_stViewCfg.Count++;

	m_stViewCfg.ViewShow[VIEWPANEL_REALTIMEEX] = t_configure->GetInt(L"Module", L"VIEWPANEL_REALTIMEEX", L"Confocal_Module");
	if (m_stViewCfg.ViewShow[VIEWPANEL_REALTIMEEX])
		m_stViewCfg.Count++;

	m_stViewCfg.ViewShow[VIEWPANEL_CONFOCALEX] = t_configure->GetInt(L"Module", L"VIEWPANEL_CONFOCALEX", L"Confocal_Module");
	if (m_stViewCfg.ViewShow[VIEWPANEL_CONFOCALEX])
		m_stViewCfg.Count++;

	m_stViewCfg.ViewShow[VIEWPANEL_3DRange] = t_configure->GetInt(L"Module", L"VIEWPANEL_3DRange", L"Confocal_Module");
	if (m_stViewCfg.ViewShow[VIEWPANEL_3DRange])
		m_stViewCfg.Count++;

	m_stViewCfg.ViewShow[VIEWPANEL_COLOR] = t_configure->GetInt(L"Module", L"VIEWPANEL_COLOR", L"Confocal_Module");
	if (m_stViewCfg.ViewShow[VIEWPANEL_COLOR])
		m_stViewCfg.Count++;

	m_stViewCfg.ViewShow[VIEWPANEL_ViewFocus] = t_configure->GetInt(L"Module", L"VIEWPANEL_ViewFocus", L"Confocal_Module");
	if (m_stViewCfg.ViewShow[VIEWPANEL_ViewFocus])
		m_stViewCfg.Count++;

	m_stViewCfg.ViewShow[VIEWPANEL_SI] = t_configure->GetInt(L"Module", L"VIEWPANEL_SI", L"Confocal_Module");
	if (m_stViewCfg.ViewShow[VIEWPANEL_SI])
		m_stViewCfg.Count++;

	m_stViewCfg.ViewShow[VIEWPANEL_DIFF_MEASURE_VIEW] = t_configure->GetInt(L"Module", L"VIEWPANEL_DIFF_MEASURE_VIEW", L"Confocal_Module");
	if (m_stViewCfg.ViewShow[VIEWPANEL_DIFF_MEASURE_VIEW])
		m_stViewCfg.Count++;
	return true;
}

//获取停靠的窗口对象
HCorePanel* ConfocalCoreModule::GetDockablePanel(DOCKPANEL_TYPE p_Panel)
{
	//HCorePanel* mTest=0;
	if (!m_CoreDockPanel[p_Panel])
	{
		m_CoreDockPanel[p_Panel]=m_ConfocalUILib.GetDockPanel(p_Panel);
	}
	return m_CoreDockPanel[p_Panel];
}

//获取框架中的文档对象
HCorePanel* ConfocalCoreModule::GetViewPanel(VIEWPANEL_TYPE p_Panel)
{
	if (!m_CoreViewPanel[p_Panel])
	{
		m_CoreViewPanel[p_Panel]=m_ConfocalUILib.GetViewPanel(p_Panel);
	}
	return m_CoreViewPanel[p_Panel];
}

HCorePanel* ConfocalCoreModule::GetMsgPanel(MESSAGE_TYPE p_Panel,bool bModal)
{
	if (!m_CoreMsgPanel[p_Panel])
	{
		m_CoreMsgPanel[p_Panel]=m_ConfocalUILib.GetMsgPanel(p_Panel,bModal);
	}
	return m_CoreMsgPanel[p_Panel];
}

//获取框架的插件
HConfocalPlug* ConfocalCoreModule::GetPlugin(PLUGIN_TYPE p_Plugin)
{
	if (!m_CorePlugin[p_Plugin])
	{
		m_CorePlugin[p_Plugin]=m_ScannerLib.GetPlugin(p_Plugin);
	}
	return m_CorePlugin[p_Plugin];
}

HCoreProcess* ConfocalCoreModule::GetCoreProcess(PROCESS_TYPE m_ProcessType)
{
	if (!m_CoreProcess[m_ProcessType])
	{
		if (mProcessMg)
			m_CoreProcess[m_ProcessType]=mProcessMg->GetProcessCore(m_ProcessType);
		//m_CoreProcess[m_ProcessType]=m_ProcessLib->GetProcessCore(m_ProcessType);
	}
	return m_CoreProcess[m_ProcessType];
}

HEasyFunction* ConfocalCoreModule::GetFunction()
{
	if (!m_EasyF)
	{
    if (mProcessMg)
    {
	  	m_EasyF=mProcessMg->GetFunction();
    }
	}
	return m_EasyF;
}

//返回相机控件
HVideoManager* ConfocalCoreModule::GetVideoManager()
{
	return m_ConfocalVideoManage;
}

HVideoDevice* ConfocalCoreModule::GetCurVideo()
{
	return m_CurVideoDevice;
}

HVideoManager* ConfocalCoreModule::GetVideoManagerEx()
{
	return m_VideoManageEx;
}

HVideoDevice* ConfocalCoreModule::GetCurVideoEx()
{
	return m_VideoDeviceEx;
}

//返回控制盒
HGearBox* ConfocalCoreModule::GetGearBox()
{
	return m_GearBox;
}

//返回DMD集合
IDMDManager* ConfocalCoreModule::GetDmdManager()
{
	return m_DmdManager;
}

//获取渲染对象
HVideoRenderChain* ConfocalCoreModule::GetRenderChain(RenderChainType pMode)
{
  if (pMode == RenderChain_SL)
  {
    return &m_SLChain;
  }
  else if (pMode == RenderChain_Confocal)
  {
    return &m_ConfocalChain;
  }
  else if (pMode == RenderChain_Normal)
  {
    return &m_RenderChain;
  }
  else if (pMode == RenderChain_ConfocalEx)
  {
	  return &m_ConfocalChainEx;
  }
  else if (pMode == RenderChain_NormalEx)
  {
	  return &m_RenderChainEx;
  }
}

//返回配置文件对象
HConfigure* ConfocalCoreModule::GetConfigure()
{
	return &m_ConfigureFileLib;
}

int ConfocalCoreModule::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	return 0;
}

void* ConfocalCoreModule::LoadDlls(CString t_name)
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
	void* t=t_GetInterface();
	return t;
}

BOOL ConfocalCoreModule::Create( CWnd* pParentWnd ) 
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if( ! CDialog::Create(IDD, pParentWnd) )
		return FALSE;
	this->ShowWindow(SW_SHOW);
	return TRUE;
}

void ConfocalCoreModule::PrintMessage(CString msg)
{
	m_Message.push_back(msg);
	//Invalidate();
	//UpdateWindow();
	CClientDC dc(this);

	if (_bmp)
	{
		//Bitmap *_bmp = new Bitmap(Confocal_ConfigSingleton::instance().m_strPlash);
		int _width = _bmp->GetWidth();
		int _height = _bmp->GetHeight();
		Graphics gra(dc);	
		int border = 1;// 绘制边框
		CRect r;
		GetClientRect(&r);
		Pen pen2(Color::Black);
		gra.DrawRectangle(&pen2, 0, 0, r.Width() - border, r.Height() - border);
		int gap = border;
		gra.DrawImage(_bmp, gap, gap, _width - gap * 2,_height - gap * 2);
	}

	COLORREF m_clr=RGB(255,0,0);
	CPen pen(3,3,m_clr);
	dc.SelectObject(&pen);
	dc.SelectObject(pBrush);
	int size=m_Message.size();
	//dc.SetTextColor( RGB(255, 255, 255) );  
	dc.SetTextColor(mTxtclr);  

	dc.SetBkMode(TRANSPARENT);
	RECT rct;
	GetWindowRect(&rct);	
	dc.SelectObject(&font);

	CPoint mPt;
	mPt.x=36;
	mPt.y=30;
	CString text=m_Message[size-1];
	CRect centerRect=new CRect(mPt,CSize(1000,100));
	dc.DrawText(text, -1,centerRect, DT_LEFT|DT_SINGLELINE);

	////////下面是从上往下显示
	//CPoint mPt;
	//mPt.x=36;
	//mPt.y=36;
	//for (int i=0;i<size;i++)
	//{
	//	CString text=m_Message[i];
	//	CRect centerRect=new CRect(mPt,CSize(1000,100));
	//	dc.DrawText(text, -1,centerRect, DT_LEFT|DT_SINGLELINE);
	//	mPt.y+=20;
	//}
	////////下面是从下往上显示
	//mPt.y=rct.bottom-50-rct.top;
	//dc.SelectObject(&font);
	//for (int i=0;i<6;i++)
	//{
	//	if(--size<0)
	//		break;
	//	CString text=m_Message[size];
	//	CRect centerRect=new CRect(mPt,CSize(1000,100));
	//	dc.DrawText(text, -1,centerRect, DT_LEFT|DT_SINGLELINE);
	//	mPt.y+=20;
	//}
	if(m_Message.size()>6)//删除多余的
		m_Message.erase(m_Message.begin()+1);
}

void ConfocalCoreModule::OnPaint()
{
  if (!GetSafeHwnd())
    return;
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码

	// 不为绘图消息调用 __super::OnPaint()
}


void ConfocalCoreModule::LogString(CString strMsg,bool ExType)
{
	if (ExType)
	{
		mLogLibEx.PrintTxt(strMsg);
	} 
	else
	{
		mLogLib.PrintTxt(strMsg);
	}
}

void ConfocalCoreModule::InitLog(bool ExType)
{
	CString str = m_ConfigureFileLib.GetModulePath() + L"\\Log";
	if (ExType)
	{
		mLogLibEx.InitLogging(str);
		mLogLibEx.SetLogSize(5);
	}
	else
	{
		mLogLib.InitLogging(str,L"EX");
		mLogLib.SetLogSize(5);
	}	
}

void ConfocalCoreModule::CloseLog(bool ExType)
{
	if (ExType)
	{
		mLogLibEx.ShutdownLogging();
	}
	else
	{
		mLogLib.ShutdownLogging();
	}
}


