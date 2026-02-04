#pragma once

#include "CarlVideo.h"
#include "dcamapi.h"

struct DcamCameraPara 
{
	int iSel;
	CString m_CameraName;
	int m_iBitDepth;
	int m_iColorChannel;
	int m_iBinningCount;
	int m_iBinningIndx;
	HDCAM m_iCurHandle;
	float m_fExposure;
	float m_fMinExposure;
	float m_fMaxExposure;
	int m_Gain;
	BYTE* m_pBuffer;
	unsigned short* m_pDcamBuffer;//底层数据传送上来是18位的
	int * m_pWidth;
	int * m_pHeight;
	long m_DcamSize;
	long m_NowFrameIndex;
	long m_TotalFrame;
};

struct FlipMirrorParas
{
	int *iFlipSrc;
	int *iFlipDst;
	int *iMirrorSrc;
	int *iMirrorDst;
	int iCount;
};


class CDcamVideo :public HVideoDevice
{
public:
	CDcamVideo(int idx,CString strName);
	~CDcamVideo(void);

	CString GetCameraName(){return g_MuCameraParas.m_CameraName;};

	int GetBiningCount(){return g_MuCameraParas.m_iBinningCount;};
	CString GetBining(int pIndex);
	void SetBinning(int pIndex,CPoint Pt=CPoint(0,0));
	virtual void SetBinningPt(int pIndex,CPoint Pt);

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
	friend DWORD WINAPI DCamDataProc(LPVOID lp);
	void GrabVideoData();
	bool GrabBuffer(int& pwidth,int& pheight);
	void UpdateBufferSize(int width, int height);
	void RenderVideoData();
	void MirrorConvert(LPBYTE Src,LPBYTE Dst,HVideoHeader mPara);
	void FlipConvert(LPBYTE Src,LPBYTE Dst,HVideoHeader mPara);
	CPoint mBinPt[3];

protected:
	HANDLE m_hEvent;
	HANDLE m_hGrabberThread;
	int m_nWaitTime;
	HVideoHeader m_VideoFormate;
	HVideoRender* m_VideoRender;
	bool m_AWB;//白平衡
	int width,height;
	bool bMirror,bFlip,bOpen;
	bool bRun;

	FlipMirrorParas mFlipMirrorParas;

private:
	DcamCameraPara g_MuCameraParas;
	CRITICAL_SECTION m_crtSec;

};