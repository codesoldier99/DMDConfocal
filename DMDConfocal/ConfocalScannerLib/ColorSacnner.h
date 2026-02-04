#pragma once
#include "HConfocalCore.h"
#include "CarlVideo.h"

class CColorSacnner:public HConfocalPlug,public HVideoRender//,public HMapScanner
{
public:
	CColorSacnner(void);
	~CColorSacnner(void);

	virtual bool InitPlugin(LPVOID p_Param=0, LPVOID p_Param2 = 0);//初始化
	virtual bool UnInitPlugin();
	virtual bool Stop();
	virtual bool Set(void* WParas){return true;};
	virtual bool Start(void* vParas=0);//启动颜色还原
	virtual CString GetPluginName() {return L"Color Build Scanner";};
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);
	virtual void* GetUserInterface(int p_Type=0) {return (HConfocalPlug*)this;};//获取扫描接口
	friend DWORD WINAPI ColorProc(LPVOID lp);
	void ScanImage();

private:
	HVideoHeader m_CaptureVideo;
	bool Capture();
	bool bScan;
	HANDLE m_ScanThread;// 采集数据线程
	HANDLE m_hCaptureEvent;// 线程与采图同步

protected:
	HConfocalCore* m_ConfocalCore;
	HCoreProcess* m_ColorBuildPro;
	HVideoDevice* m_CurVideoDevice;
};

