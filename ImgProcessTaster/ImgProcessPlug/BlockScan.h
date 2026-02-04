#pragma once
#include "HOpenCVBase.h"
#include "HCoreProcess.h"

class CBlockScan:public HImgProcess
{
public:
	CBlockScan(void);
	~CBlockScan(void);

public:
	virtual bool InitProcess(LPVOID p_param=0);//³õÊ¼»¯¿â
	virtual bool UnInitProcess() {return false;};
	//virtual bool ProcessImg(HVideoHeader* pHeader,float fPiexlSize,void* OutPutPara=0,void* InputPara=0);
	virtual long ProcessImg(HVideoHeader* pOriginHeader,HVideoHeader* pEndHeader,float fPiexlSize,void* OutPutPara=0);
	virtual void SavePic( HVideoHeader* pHeader,CString pName,int pType=0){return;};
	virtual void SetDebug(bool bV){bDebug=bV;};

private:
	ProcessParas* mProcessV;
	bool bDebug;
	CRITICAL_SECTION m_crtSec;
	void DrawBinaryImg(cv::Mat mBinaryImg,Mat mImg);
public:
	void GetObjectArea(cv::Mat mBinaryImg,double *MaxArea,double *MinArea,double *dAllArea=0);
};

