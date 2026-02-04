#pragma once

#include "HConfocalCore.h"

#include <vector>
using std::vector;

class C3DCapture:public HConfocalPlug,public HVideoRender//,public HMapScanner
{
public:
	C3DCapture(void);
	~C3DCapture(void);

public://实现
	virtual bool InitPlugin(LPVOID p_Param=0, LPVOID p_Param2 = 0);//初始化库
	virtual bool UnInitPlugin();
	virtual bool Stop();
	virtual CString GetPluginName() {return L"AutoFocusDAC";};
	virtual bool Set(void* WParas);       //设置参数
	virtual bool Start(void* vParas=0);   //启动任务线程
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	virtual int OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);

	friend DWORD WINAPI Capture3DProc(LPVOID lp);

	bool Capture0(bool bTrigger = false);
	bool Capture1(bool bTrigger = false);
private:
	CString strSavsPath;
	CString strMsg;
	HCoreProcess* m_ReBuildPro;

	void Capture3D();
	bool CaptureImg();
	

	HANDLE m_ScanThread;// 采集数据线程
	HConfocalCore* m_ConfocalCore;

	HVideoHeader m_CaptureVideo0;//滨松相机（含共焦）存图
	HVideoDevice* m_CurVideoDevice0;//滨松相机
	HANDLE m_hCaptureEvent0;

	HVideoHeader m_CaptureVideo1;//鑫图相机（含共焦）存图
	HVideoDevice* m_CurVideoDevice1;//鑫图相机
	HANDLE m_hCaptureEvent1;

	HAxis* m_AxisZ;
	HAxis* m_AxisPIZ;
	HAxis* m_UseAxisZ;
	HANDLE m_hMoveEvent;	//线程与载物台同步

	ReBuildParas m_CapParam;//保存传进来的参数结构体
	bool m_bConfocal;		//通过Start()得到当前是否是共焦模式

	void Func_FlushImg(HVideoHeader* pSrcHeader, LPBYTE pBuffer, HVideoHeader &DstHeader, HANDLE hCaptureEvent);

	bool BuildDirectory(CString strPath);
};

