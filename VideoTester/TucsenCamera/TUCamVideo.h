#pragma once

#include "CarlVideo.h"
#include "TUCamApi.h"

struct TUCameraPara 
{
	int iSel;
	CString m_CameraName;
	int m_iBitDepth;
	int m_iColorChannel;
	int m_iBinningCount;
	int m_iBinningIndx;
	HDTUCAM m_handle;
	float m_fExposure;
	float m_fMinExposure;
	float m_fMaxExposure;
	int m_Gain;
	BYTE* m_pBuffer;
	unsigned short* m_pDcamBuffer;//底层数据传送上来是16位的
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

class TUCamVideo :public HVideoDevice
{
public:
	TUCamVideo(int idx,CString strName);
	~TUCamVideo(void);

  static void __cdecl WaitForFrameThread(LPVOID lParam); // 等待数据线程实体

	CString GetCameraName(){return m_TUCameraParas.m_CameraName;};

	int GetBiningCount(){return m_TUCameraParas.m_iBinningCount;};
	CString GetBining(int pIndex);
	void SetBinning(int pIndex,CPoint Pt=CPoint(0,0));
	virtual void SetBinningPt(int pIndex,CPoint Pt);

	int GetBiningIndex(){return m_TUCameraParas.m_iBinningIndx;};

	bool OpenCamera();
	bool Run();
	virtual bool Pause();
	bool CloseCamera();

	bool GetEnable(HVideoPID pPID);
	void SetValue(HVideoPID pPID,float pValue);
	float GetValue(HVideoPID pPID);
	void SetExposure(float pExpos);
	float GetExposure(){return m_TUCameraParas.m_fExposure;};
	void AutoAWB(bool pAuto);
	virtual bool SetFlip(bool pFlip);
	virtual bool SetMirror(bool pMirror);

	HVideoHeader GetImgHeader();
	bool GrabImage(HVideoHeader* pHeader,LPBYTE pBuffer);
	void SetTrigerMode(TrigerType pMode);
	void SetRender(HVideoRender* pRender);
	void TrigerVideoData();

private:
	bool GrabBuffer(int& pwidth,int& pheight);
	void UpdateBufferSize(int width, int height);
	void RenderVideoData();
	void MirrorConvert(LPBYTE Src,LPBYTE Dst,HVideoHeader mPara);
	void FlipConvert(LPBYTE Src,LPBYTE Dst,HVideoHeader mPara);
	CPoint mBinPt[3];

  void StartWaitForFrame();

  void StopWaitForFrame();

  void ClearBuffer();
protected:
	HVideoHeader m_VideoFormate;
	HVideoRender* m_VideoRender;
	bool m_AWB;//白平衡
	int width,height;
	bool bMirror,bFlip,bOpen;
	bool bRun;

	FlipMirrorParas mFlipMirrorParas;

  TUCAM_FRAME         m_frame;                        // 帧对象

  int m_nTriMode;

  BOOL                m_bWaitting;                    // 捕获相机数据

  HANDLE              m_hThdWaitForFrame;             // 数据捕获线程句柄
private:
	TUCameraPara m_TUCameraParas;
	CRITICAL_SECTION m_crtSec;

};