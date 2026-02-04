#pragma once

#include "HConfocalCore.h"
#include "HTimer.h"
#include "CarlVideo.h"


class C3DScanner:public HConfocalPlug,public HVideoRender
{

public:
	C3DScanner(void);
	~C3DScanner(void);

	friend DWORD WINAPI ScanProc(LPVOID lp);
	void ScanImage();

	friend DWORD WINAPI ScanPIZProc(LPVOID lp);
	void ScanPIZ3D();

public://实现
	virtual bool InitPlugin(LPVOID p_Param=0, LPVOID p_Param2 = 0);//初始化库
	virtual bool UnInitPlugin();
	virtual bool Set(void* WParas);//设置参数
	virtual bool Start(void* vParas=0);//启动扫描
	virtual bool Stop();
	virtual CString GetPluginName() {return L"3DScanner";};
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);
	bool Capture();
	bool BuildDirectory(CString strPath);
private:
	float m_ScanStart,m_ScanStop,m_ScanStep;
	int m_ScanCount,m_ScanCountRec,iSleep;//扫描张数、计数
	float m_ScanSpeed;// Z轴速度
	HTimer m_HTimer;
	HVideoHeader m_CaptureVideo;//帧频的数据
	CString strSavsPath;

protected:
	HVideoDevice* m_CurVideoDevice;
	HConfocalCore* m_ConfocalCore;
	HConfigure* m_ConfigureFile;
	HCoreProcess* m_ReBuildPro;
	HAxis* m_AxisZ;
	HAxis* m_AxisPIZ;
	float m_Axis_Next;
	float m_Axis_Pos;

	HANDLE m_ScanThread;// 采集数据线程
	HANDLE m_hMoveEvent;// 线程与载物台同步
	HANDLE m_hCaptureEvent;// 线程与采图同步
	bool bScan;
	bool bSaveImg;
	CString strMsg;
	bool bConfocalMode;
	bool bPIZ;
};

