#pragma once
#include "VideoManager.h"

struct MuCameraPara 
{
	CString m_CameraName;
	int m_iBitDepth;
	int m_iColorChannel;
	int m_iBinningCount;
	int m_iBinningIndx;
	MUCam_Handle m_iCurHandle;
	float m_fExposure;
	float m_fMinExposure;
	float m_fMaxExposure;
	int m_Gain;
	BYTE* m_pBuffer; 
	int * m_pWidth;
	int * m_pHeight;

	MUCam_Format m_Format;
};

// 此类是从 MoticVideoLib.dll 导出的
class MucamVideo :public HVideoDevice
{
public:
	MucamVideo(MUCam_Handle t_handle,CString t_name);
	~MucamVideo(void);

	CString GetCameraName(){return g_MuCameraParas.m_CameraName;};

	int GetBiningCount(){return g_MuCameraParas.m_iBinningCount;};
	CString GetBining(int pIndex);
	void SetBinning(int pIndex,CPoint Pt=CPoint(0,0));
	virtual void SetBinningPt(int pIndex,CPoint Pt){};
	int GetBiningIndex(){return g_MuCameraParas.m_iBinningIndx;};

	bool OpenCamera();
	bool Run();
	virtual bool Pause();
	bool CloseCamera();

	bool GetEnable(HVideoPID pPID);
	void SetValue(HVideoPID pPID,float pValue);
	float GetValue(HVideoPID pPID);
	void SetExposure(float pExpos);
	float GetExposure(){return g_MuCameraParas.m_fExposure;};
	void AutoAWB(bool pAuto);
	virtual bool SetFlip(bool pFlip);
	virtual bool SetMirror(bool pMirror);

	HVideoHeader GetImgHeader();
	bool GrabImage(HVideoHeader* pHeader,LPBYTE pBuffer);
	void SetTrigerMode(TrigerType pMode);
	void SetRender(HVideoRender* pRender);
	void TrigerVideoData();


private:
	friend DWORD WINAPI GrabVideoDataProc(LPVOID lp);
	void GrabVideoData();
	bool GrabBuffer(int& pwidth,int& pheight);
	void UpdateBufferSize(int width, int height);

protected:
	HANDLE m_hEvent;
	HANDLE m_hGrabberThread;
	int m_nWaitTime;
	HVideoHeader m_VideoFormate;
	HVideoRender* m_VideoRender;
	bool m_AWB;//白平衡
	int width,height;
	bool bOpen;

private:
	MuCameraPara g_MuCameraParas;
	CRITICAL_SECTION m_crtSec;
};