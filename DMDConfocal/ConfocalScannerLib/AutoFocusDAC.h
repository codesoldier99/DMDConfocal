#pragma once

#include "HConfocalCore.h"

#include <vector>
using std::vector;

class CAutoFocusDAC:public HConfocalPlug,public HVideoRender//,public HMapScanner
{
public:
	CAutoFocusDAC(void);
	~CAutoFocusDAC(void);

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

	friend DWORD WINAPI AutoFocusDACProc(LPVOID lp);
	friend DWORD WINAPI AutoFocusDACTestProc(LPVOID lp);

	bool Capture0(bool bTrigger = false);
	bool Capture1(bool bTrigger = false);
private:
	CString strMsg;
	HCoreProcess* pDACProcess;//差动聚焦函数

	void ScanAuto();
	bool MoveZ(int iDirection,float fValue);
	bool MoveToZ(float fValue);

	void ScanAutoTest();

	

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

	DiffMeasureParam _dmp;
	void Func_FlushImg(HVideoHeader* pSrcHeader, LPBYTE pBuffer, HVideoHeader &DstHeader, HANDLE hCaptureEvent);

	bool SetImg(int width, int height, int wbit,LPBYTE pBuffer, HVideoHeader &DstHeader);
};

