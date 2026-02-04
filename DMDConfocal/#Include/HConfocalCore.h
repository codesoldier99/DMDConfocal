
#pragma once

#include "CarlVideo.h"//消息从3000开始
#include "HGearBox.h"//消息从4000开始
#include "HCoreProcess.h"//消息从5000开始

#include "HConfFile.h"
#include "IHsmObserver.h"
#include "RenderChain.h"

#include "HConfocalUI.h"
#include "HConfocalPlug.h"
#include "IDMDManager.h"



/////////////////////////消息列表/////////////////////////////////
#define PANEL_MESSAGE_INIT	1
#define PANEL_MESSAGE_CLOSE	2
#define PANEL_VIEW_3DCMOPLETE 3 //3D扫结束

//共聚焦模式的消息
#define  NOTIFY_CONFOCAL_START 4
#define  NOTIFY_CONFOCAL_STOP  5
#define  NOTIFY_CONFOCAL_SAVERESULT  6
#define  NOTIFY_CONFOCAL_RESUM 7
#define  NOTIFY_CONFOCAL_SAVEPROCESS  8
#define  NOTIFY_CONFOCAL_SAVEPROCESSEND  9
#define  NOTIFY_CONFOCAL_INPROCESS   10
#define  NOTIFY_CONFOCAL_INPROCESSEX 11

// 结构光模式消息
#define NOTIFY_SI_START 20
#define NOTIFY_SI_STOP 21
#define NOTIFY_SI_SAVERESULT  22
#define NOTIFY_SI_RESUM 23
#define NOTIFY_SI_SAVEPROCESS  24
#define NOTIFY_SI_SAVEPROCESSEND  25
#define NOTIFY_SI_INPROCESS  26

#define  NOTIFY_VIDEO_FORMAT_EXP 1000 // 视频格式曝光时间发生了变化

#define  NOTIFY_MSGVIEW_SHOW 1990 //MSG对话框的消息
#define  NOTIFY_MSGVIEW_Info 1991
#define  NOTIFY_MSGVIEW_END 1992

#define	WM_ChangePos 1993

#define	NOTIFY_PIZChange 1994

#define NOTIFY_DIFF_MEASURE 2000 // 差动测量获取到了图像

/////////////////////////消息列表/////////////////////////////////


class HCorePanel:public IHsmObserver,public IHsmSubject
{
public:
	virtual bool InitPanel(LPVOID p_Param, LPVOID p_Param2=0)=0;// {return 0;};
	virtual bool UnInitPanel() {return 0;};
	virtual CWnd* GetCWnd()=0;//得到当前窗口
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0,float fParam=0.0f,
		void* mParam=0){return 0;};
};

class HConfocalPlug:public IHsmObserver,public IHsmSubject
{
public:
	virtual bool Set(void* WParas)=0;//设置参数
	virtual bool Start(void* vParas=0)=0;//启动线程
	virtual bool Stop()=0;//关闭
	virtual bool InitPlugin(LPVOID p_Param=0, LPVOID p_Param2 = 0)=0;//初始化库
	virtual bool UnInitPlugin() {return false;};
	virtual CString GetPluginName()=0;//获取名字
	virtual HVideoRender* GetVideoRender() {return 0;};//得到视频渲染接口--入口
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0,float fParam=0.0f,
		void* mParam=0) {return 0;};
};

using namespace WlpDMDControl;

struct stVIEWPANEL
{
	int Count = 0;
	int ViewShow[9];
};

class HConfocalCore:public IHsmObserver,public IHsmSubject
{
public:
  enum RenderChainType
  {
    RenderChain_Normal,
    RenderChain_Confocal,
    RenderChain_SL,
	RenderChain_NormalEx,
	RenderChain_ConfocalEx,
  };
public:
	virtual bool Init(LPVOID p_param=0)=0;//初始化
	virtual bool Uninit(LPVOID p_param=0)=0;//关闭
	virtual HCorePanel* GetDockablePanel(DOCKPANEL_TYPE p_Panel)=0;//获取面板
	virtual HCorePanel* GetViewPanel(VIEWPANEL_TYPE p_Panel)=0;//获取视图
	virtual HCorePanel* GetMsgPanel(MESSAGE_TYPE p_Panel,bool bModal=false)=0;//获取对话框
	virtual HConfocalPlug* GetPlugin(PLUGIN_TYPE p_Plugin)=0;//获取插件
	virtual HCoreProcess* GetCoreProcess(PROCESS_TYPE m_ProcessType)=0;//获取图像处理库
	virtual HEasyFunction* GetFunction()=0;
	virtual void InitLog(bool ExType = false)=0;
	virtual void LogString(CString strMsg, bool ExType = false)=0;
	virtual void CloseLog(bool ExType = false)=0;

	virtual IDMDManager*    GetDmdManager()=0;//获取DMD
	virtual HVideoDevice*   GetCurVideo()=0;//获取当前视频接口
	virtual HVideoManager*  GetVideoManager()=0;//获取视频接口
	virtual HVideoDevice*   GetCurVideoEx() = 0;
	virtual HVideoManager*  GetVideoManagerEx() = 0;
	virtual HGearBox*       GetGearBox()=0;//获取控制盒
	virtual HConfigure*     GetConfigure()=0;//获取配置接口
	virtual HVideoRenderChain* GetRenderChain(RenderChainType pMode= RenderChain_Normal)=0;//获取渲染链	
	
	virtual stVIEWPANEL GetViewCfg() = 0;
};
