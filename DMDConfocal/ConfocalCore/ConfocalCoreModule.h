#pragma once

#include "HConfocalCore.h"
#include "ConfocalUILib.h"
#include "ConfocalScannerLib.h"
#include "resource.h"
#include "SplashWnd.h"
#include "SplashPanel.h"
#include "Confocal_Config.h"

#include <vector>
#include "LogLib.h"

typedef void* (*PluginLib_GetInterface)(void);
typedef CString (*PluginLib_GetName)(void);

class ConfocalCoreModule:public HConfocalCore,public CDialogEx
{
	DECLARE_DYNAMIC(ConfocalCoreModule)

public:
	ConfocalCoreModule(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~ConfocalCoreModule();
	virtual BOOL Create( CWnd* pParentWnd = NULL );

	// 对话框数据
	enum { IDD = IDD_DLGFLASH };
	//ConfocalCoreModule(void);
	//~ConfocalCoreModule(void);

	virtual bool Init(LPVOID p_param=0);//初始化
	virtual bool Uninit(LPVOID p_param=0);//卸载
	virtual HCorePanel* GetDockablePanel(DOCKPANEL_TYPE p_Panel);//获取面板
	virtual HCorePanel* GetViewPanel(VIEWPANEL_TYPE p_Panel);//获取视图
	virtual HCorePanel* GetMsgPanel(MESSAGE_TYPE p_Panel,bool bModal=false);
	virtual HConfocalPlug* GetPlugin(PLUGIN_TYPE p_Plugin);
	virtual IDMDManager* GetDmdManager();
	virtual HVideoManager* GetVideoManager();//获取视频接口
	virtual HVideoDevice* GetCurVideo();
	virtual HVideoManager* GetVideoManagerEx();
	virtual HVideoDevice* GetCurVideoEx();
	virtual HGearBox* GetGearBox();//获取控制盒
	virtual HConfigure* GetConfigure();
	virtual HVideoRenderChain* GetRenderChain(RenderChainType pMode = RenderChain_Normal);
	virtual HCoreProcess* GetCoreProcess(PROCESS_TYPE m_ProcessType);
	virtual HEasyFunction* GetFunction();
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);
	virtual void LogString(CString strMsg,bool ExType = false);
	virtual void InitLog(bool ExType = false);
	virtual void CloseLog(bool ExType = false);

	virtual stVIEWPANEL GetViewCfg() { return m_stViewCfg; };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()

	void* LoadDlls(CString tName);
	CConfocalUILib m_ConfocalUILib;
	CConfocalScannerLib m_ScannerLib;
	HCoreProcessMg* mProcessMg;//图像处理

	HConfigure	m_ConfigureFileLib;
	HVideoRenderChain m_RenderChain;
	HVideoRenderChain m_ConfocalChain;
	HVideoRenderChain m_SLChain;
	HVideoRenderChain m_RenderChainEx;
	HVideoRenderChain m_ConfocalChainEx;
#define MAX_PANEL_NUM	20 //各种面板 
	HConfocalPlug* m_CorePlugin[MAX_PANEL_NUM];
	HCoreProcess* m_CoreProcess[MAX_PANEL_NUM];
	HCorePanel* m_CoreDockPanel[MAX_PANEL_NUM];
	HCorePanel* m_CoreViewPanel[MAX_PANEL_NUM];
	HCorePanel* m_CoreMsgPanel[MAX_PANEL_NUM];
	HEasyFunction* m_EasyF;

private://相机集合、已选相机设备、DMD集合、外部轴控制
	HVideoType*     mVideoType;//所有相机类
	HVideoManager*  m_ConfocalVideoManage;
	HVideoDevice*	m_CurVideoDevice;//当前相机设备
	IDMDManager*	m_DmdManager;
	WlpDMDParas     mWlpDMDParas;
	HGearBox*		m_GearBox;

	HVideoManager*  m_VideoManageEx;
	HVideoDevice*	m_VideoDeviceEx;//当前相机设备

	//闪屏组合
	CSplashWnd m_Splash;//闪屏、初始化的闪屏对象
	SplashPanel _splash;
	Bitmap *_bmp;//启动画面背景图

	std::vector<CString> m_Message;
	//启动绘字
	CFont font;
	CBrush *pBrush;
	COLORREF mTxtclr;//=RGB(255,0,0);

	CLogLib mLogLib;
	CLogLib mLogLibEx; //额外共聚焦使用

	stVIEWPANEL m_stViewCfg;
	bool InitViewCfg();

public:
	void PrintMessage(CString msg);
	CString mMsgShow;
	afx_msg void OnPaint();
};

