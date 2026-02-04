
#pragma once

#include "stdafx.h"
#include "IHsmObserver.h"

class HVideoManager;
class HVideoDevice;

#define Notify_CarlVideo_OpenCamera	3000  //发送的指针参数一 Void* 代表CString相机名称 
#define Notify_CarlVideo_CloseCamera 3001
#define Notify_CarlVideo_RunCamera 3003
#define Notify_CarlVideo_PauseCamera 3004
#define Notify_CarlVideo_ExpTime 3005 //发送调节曝光时间 ，fParam 发送出曝光的时间
#define Notify_CarlVideo_Triger 3006  //触发方式改变，Void* 传送触发方式
#define Notify_CarlVideo_Flip 3007  //第一个Void* 传递的是bool数据，指定Flip状态
#define Notify_CarlVideo_Mirror 3008

typedef enum
{
	VIDEO_PID_EXMAX,
	VIDEO_PID_EXMIN,
	VIDEO_PID_GAINMAX,
	VIDEO_PID_GAINMIN,
	VIDEO_PID_RGAIN,
	VIDEO_PID_GGAIN,
	VIDEO_PID_BGAIN,
	VIDEO_PID_FLIP,
	VIDEO_PID_MIRROR,
	VIDEO_PID_GAIN,	
	VIDEO_PID_TRIGER,
	VIDEO_PID_WhitBalance,
	VIDEO_PID_AutoExp
}HVideoPID;


typedef enum
{
	TRIGER_SOFT,
	TRIGER_INTERAL,
	TRIGER_OUT
}TrigerType;

typedef struct 
{
	long	Vwidth;
	long	Vheight;
	BYTE	VwBit;		// bits per channel
	long	Vsize;		//帧大小		
	LPBYTE	Vbuffer;	//内存地址
	long	VnCount;	//帧数
	DWORD	VcaptureTime;//捕捉时间
	long	Vuser;		//用户定义
	float VFps;		//帧频
	float fPixelSize;//像素大小，转化为mm
}HVideoHeader;


typedef struct 
{
	CString strCameraType;
	HVideoManager* mVideoManager;
}VideoTypeParas;

class HVideoRender
{
public:	
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer)=0;
};


class HVideoDevice:public IHsmSubject
{
public:
	virtual CString GetCameraName()=0;

	virtual int GetBiningCount()=0;
	virtual CString GetBining(int pIndex)=0;
	virtual void SetBinning(int pIndex,CPoint Pt=CPoint(0,0))=0;
	virtual void SetBinningPt(int pIndex,CPoint Pt)=0;
	virtual int GetBiningIndex()=0;

	virtual bool OpenCamera()=0;
	virtual bool Run()=0;
	virtual bool Pause()=0;
	virtual bool CloseCamera()=0;

	virtual bool GetEnable(HVideoPID pPID)=0;
	virtual void SetValue(HVideoPID pPID,float pValue)=0;
	virtual float GetValue(HVideoPID pPID)=0;
	virtual void SetExposure(float pExpos)=0;
	virtual float GetExposure()=0;
	virtual void AutoAWB(bool pAuto)=0;
	virtual bool SetFlip(bool pFlip)=0;
	virtual bool SetMirror(bool pMirror)=0;

	virtual HVideoHeader GetImgHeader()=0;
	virtual bool GrabImage(HVideoHeader* pHeader,LPBYTE pBuffer)=0;
	virtual void SetTrigerMode(TrigerType pMode)=0;
	virtual void TrigerVideoData()=0;
	virtual void SetRender(HVideoRender* pRender)=0;
};

class HVideoManager:public IHsmSubject,public IHsmObserver
{
public:
	virtual ~HVideoManager() {};
	virtual void SearchCameras()=0;
	virtual int GetCameraCount()=0;
	virtual CString GetCameraName(int pIndex)=0;
	virtual HVideoDevice* GetCamerDevice(int pIndex)=0;
	virtual void CloseCamera(int pIndex)=0;
	virtual HVideoDevice* OpenCamera(int pIndex)=0;
	virtual HVideoDevice* OpenCamera(CString pName)=0;
};

class HVideoType
{
public:
	virtual int FindCameraType()=0;
	virtual int GetTypeCount()=0;
	virtual VideoTypeParas* GetCameraType(int pIndex)=0;
	virtual VideoTypeParas* GetCameraType(CString mCameraType)=0;
};
