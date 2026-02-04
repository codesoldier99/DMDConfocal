#pragma once

#include "HConfocalCore.h"
#include "HTimer.h"
#include "CarlVideo.h"
#include <vector>
using namespace std;

class DiffMeasurePro:public HConfocalPlug,public HVideoRender
{
public:
	DiffMeasurePro(void);
	~DiffMeasurePro(void);

	friend DWORD WINAPI DiffMeasureProProc(LPVOID lp);
	friend DWORD WINAPI DiffMeasureProTestProc(LPVOID lp);

	friend DWORD WINAPI Cap0Proc(LPVOID lp);
	friend DWORD WINAPI Cap1Proc(LPVOID lp);
	friend DWORD WINAPI AllProc(LPVOID lp);

	void ScanImage();
	void ScanImage0();
	void ScanImage1();
	void ProcessImage();
	void DoTest();

public://实现
	virtual bool InitPlugin(LPVOID p_Param=0, LPVOID p_Param2 = 0);//初始化库
	virtual bool UnInitPlugin();
	virtual bool Set(void* WParas);//设置参数
	virtual bool Start(void* vParas=0);//启动扫描
	virtual bool Stop();
	virtual CString GetPluginName() {return L"DiffMeasurePro";};
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);

	bool Capture0(bool bTrigger = false);
	bool Capture1(bool bTrigger = false);
private:

	void Func_FlushImg(HVideoHeader* pSrcHeader, LPBYTE pBuffer
		, HVideoHeader &DstHeader, HANDLE hCaptureEvent);

	HVideoHeader m_CaptureVideo0;//滨松相机（含共焦）存图
	HVideoHeader m_CaptureVideo1;//鑫图相机（含共焦）存图
	HANDLE m_hCaptureEvent0;
	HANDLE m_hCaptureEvent1;

	HVideoDevice* m_CurVideoDevice0;//滨松相机
	HVideoDevice* m_CurVideoDevice1;//鑫图相机

	//CString strSavsPath;

	DiffMeasureParam _dmp;

	  // 采集到的图像信息
	  int _img_w;
	  int _img_h;
	  vector<unsigned char> _img_a, _img_b, _img_c;
protected:
	HConfocalCore* m_ConfocalCore;
	HANDLE m_ScanThread;// 采集数据线程

	HANDLE m_ScanThread0;
	HANDLE m_ScanEvent0;
	HANDLE m_ScanThread1;
	HANDLE m_ScanEvent1;
	HANDLE m_ProcessThread;
	CString strMsg;
};

