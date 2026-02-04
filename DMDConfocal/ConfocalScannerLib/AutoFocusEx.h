#pragma once

#include "HConfocalCore.h"

#include <vector>
using std::vector;

class CAutoFocusEx:public HConfocalPlug,public HVideoRender
{
public:
	CAutoFocusEx(void);
	~CAutoFocusEx(void);

public://实现
	virtual bool InitPlugin(LPVOID p_Param=0, LPVOID p_Param2 = 0);//初始化库
	virtual bool UnInitPlugin();
	virtual bool Stop();
	virtual CString GetPluginName() {return L"AutoFocus";};
	virtual bool Set(void* WParas);//设置参数
	virtual bool Start(void* vParas=0);//启动任务线程
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	virtual int OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);

	friend DWORD WINAPI AutoFocusProcEx(LPVOID lp);
private:
	bool bWaitMode;
	void WaitStart();

	void InitValue();//初始化调焦所需要的所有值
	bool Capture();
	void ScanAuto();
	void ScanAuto2();
	bool MoveZ(int iDirection,float fValue);
	bool MoveToZ(float fValue);

	CString strSaveFile;
	CString GetTimeString();
	bool BuildDirectory(CString strPath);
	void SaveFocusValue();//记录参数


	void GetMax(vector<double> mValues,double *Max,int *Idx,int iStart);
	float GetTriangleValue(double mV1,double mV2,double mV3,int iDr,float mStep);
	float GetTriangleValue2(double mV1,double mV2,double mV3,int iDr,float mStep);
	vector<CStringA> imgFiles;//选择所有图像文件

	void Contrl0();
	void Contrl1();
	bool Contrl();

private:
	HConfocalCore* m_ConfocalCore;
	HVideoDevice* m_CurVideoDevice;
	HVideoHeader m_CaptureVideo;
	AutoFocusParas* mAutoFocusParas;
	HAxis* m_AxisZ;
	HCoreProcess* mAutoProcess;//聚焦评判函数
	HANDLE m_ScanThread;// 采集数据线程
	HANDLE m_hMoveEvent;// 线程与载物台同步
	HANDLE m_hCaptureEvent;// 线程与采图同步
	bool bFocusOver;//控制线程的扫描
	CString strMsg;

protected: 
	CString strFile;
	////////////调焦需要的参数列表////////////
	int iControl;//指示进行细调焦或者粗调焦，0代表
	vector<double> mXiValue;
	vector<float> mXiPt;//位置
	vector<float> mXidw;//记录细定位
	vector<double> mCuValue;//方向判断的粗聚焦值
	vector<float> mCuPt;//位置
	vector<float> mCudw;//记录粗定位
	////////////调焦需要的参数列表////////////

	vector<double> mFocusValue;
	vector<float> mFocusPt;//位置
	vector<float> mFocusdw;//记录细定位
	int iDirect;

};

