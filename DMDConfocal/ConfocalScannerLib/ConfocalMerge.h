#pragma once

#include "IDMDManager.h"
#include "HImgProcess.h"
#include "CarlVideo.h"
#include "HConfocalCore.h"
//#include "MergeView.h"

using namespace WlpDMDControl;

//class mSaveParas
//{
//public:
//	int iSaveW,iSaveH,iBit;
//	CString thSavePath;
//	LPBYTE m_SaveTemp;
//};

class CConfocalMerge: public HVideoRender,public HConfocalPlug//,public IHsmObserver,public IHsmSubject
{

public:
	CConfocalMerge(void);
	~CConfocalMerge(void);
	
	virtual bool Set(void* WParas){return true;};//设置参数
	virtual bool Start(void* vParas=0){return true;};//启动扫描
	virtual bool Stop(){return true;};
	friend DWORD WINAPI ImageProc(LPVOID lp);
	void ProcessImage();

	void GetParam();
	void RestMerge(void);
	void Save_Result(CString PathFile = L"");

	virtual bool InitPlugin(LPVOID p_Param=0, LPVOID p_Param2 = 0);
	virtual CString GetPluginName() {return L"ConfocalMode";};
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	bool BuildDirectory(CString strPath);

	//存图线程
	friend DWORD WINAPI SaveProc(LPVOID lp);
	void DoSaveProc();
	HANDLE m_hSaveThread;//存图线程句柄
	int iSaveW,iSaveH,iBit;
	CString thSavePath;
	LPBYTE m_SaveTemp;//要存储的图

protected:

	typedef struct
	{
		int index;
		LPBYTE buffer;
	}s_ImgBuffer;

	IDMDManager* m_DmdManage;
	HConfocalCore* m_ConfocalCore;
	HVideoRender* m_RenderChain;
	s_ImgBuffer m_Template[100];//读取模板图像
	long iWidth,iHeight;
	int wbit,m_NowIndex;
	LPBYTE m_bmpResult;//存储最终结果图
	LPBYTE m_bmpTemp;//过程图
	long m_imgsize;
	bool m_bStart;
	bool m_bExit;
	HANDLE m_hGrabberThread;// 采集数据线程
	HANDLE m_hEvent;// 线程退出事件

	int m_MergeCount;
	int m_iBackTh;//背景阈值
	CString strPath;

	CString strSaveFile;
	bool m_bSaveBmp;//存储共聚焦结果图
	bool bSaveConfocal;//存储共聚焦过程图
private:
	bool m_bExType;//额外的共聚焦
};

