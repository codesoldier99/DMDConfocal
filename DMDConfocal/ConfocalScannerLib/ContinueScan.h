#pragma once
#include "HConfocalCore.h"
#include "CarlVideo.h"

class ContinueScan:public HConfocalPlug,public HVideoRender
{
public:
	ContinueScan(void);
	~ContinueScan(void);

	friend DWORD WINAPI SaveScanProc(LPVOID lp);
	void DoSaveScan();

public://实现
	virtual bool InitPlugin(LPVOID p_Param=0, LPVOID p_Param2 = 0);//初始化库
	virtual bool UnInitPlugin();
	virtual bool Set(void* WParas);//设置参数
	virtual bool Start(void* vParas=0);//启动扫描
	virtual bool Stop();
	virtual CString GetPluginName() {return L"3DScanner";};
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0){return 0;};
	bool Capture();
	bool BuildDirectory(CString strPath);
private:
	HVideoHeader m_CaptureVideo;//帧频的数据
	CString strSavsPath;

protected:
	HVideoDevice* m_CurVideoDevice;
	HConfocalCore* m_ConfocalCore;
	HCoreProcess* m_ReBuildPro;

	HANDLE m_ScanThread;// 采集数据线程
	HANDLE m_hCaptureEvent;// 线程与采图同步

	bool bScan;	
	CString strMsg;
	bool bConfocalMode;
	int iSaveCount;//采集图像个数
};

