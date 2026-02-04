#pragma once

#include "HConfocalCore.h"

#include <vector>
using std::vector;

class CAutoFocus:public HConfocalPlug,public HVideoRender//,public HMapScanner
{
public:
	CAutoFocus(void);
	~CAutoFocus(void);

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

	friend DWORD WINAPI AutoFocusProc(LPVOID lp);
private:
	bool bWaitMode;
	void WaitStart();

	void InitValue();//初始化调焦所需要的所有值
	bool Capture();
	void ScanAuto();
	bool MoveZ(int iDirection,float fValue);
	bool MoveToZ(float fValue);

	CString strSaveFile;
	CString GetTimeString();
	bool BuildDirectory(CString strPath);
	void SaveFocusValue();//记录参数

	void Contrl0();
	void Contrl1();
	void Contrl2();
	void Contrl3();
	void Contrl4();
	void Contrl5();
	bool MoveContrl5(int Con5Direct);
	void Contrl6();
	void Contrl7();
	void GetMax(vector<double> mValues,double *Max,int *Idx);
	float GetTriangleValue(double mV1,double mV2,double mV3,int iDr,float mStep);
	float GetTriangleValue2(double mV1,double mV2,double mV3,int iDr,float mStep);
	vector<CStringA> imgFiles;//选择所有图像文件
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
	int iControl;//指示调焦进行的步骤
	vector<double> mPreDr;
	vector<float> mPreDrP;//位置
	int mPreDirection;
	float mPredw;
	vector<double> mCuDr;//方向判断的粗聚焦值
	vector<float> mCuDrP;//位置
	int mDirectionDr;//方向判断方向
	vector<double> mCuContinuous;//粗调采集聚焦值
	vector<float> mCuContinuousP;//位置
	float mRdw;//粗调采集校正值，带正负
	vector<double> mCuTriangle;//粗调三角形聚焦值
	vector<float> mCuTriangleP;//位置
	float mRough;//粗调三角形校正值
	float mRoughxz;//粗调校正值
	vector<double> mXiContinuous1;//细调采集聚焦值
	vector<float> mXiContinuous1P;//位置
	vector<double> mXiContinuous2;//细调三角形聚焦值
	vector<float> mXiContinuous2P;//位置
	int iCon5Direct;//=0;Control5的次数
	float mXdw;//细调采集校正值，带正负
	vector<double> mXiTriangle;
	vector<float> mXiTriangleP;//位置
	float mFocus;//细调三角形校正值，带正负
	float mFocusxz;//细调校正值
	////////////调焦需要的参数列表////////////


};

