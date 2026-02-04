#pragma once
#include "HOpenCVBase.h"
#include "HCoreProcess.h"

class AutoWB:public HImgProcess
{
public:
	AutoWB(void);
	~AutoWB(void);

	virtual bool InitProcess(LPVOID p_param=0){return true;};//初始化库
	virtual bool UnInitProcess() {return false;};
	virtual long ProcessImg(HVideoHeader* pOriginHeader,HVideoHeader* pEndHeader,float fPiexlSize,void* OutPutPara=0);//处理图像，返回Good或者NotGood
	virtual void SetDebug(bool bV){bDebug=bV;};

private:
	struct mRECT
	{
		long left;//矩形左上角横坐标
		long top;//矩形左上角纵坐标
		long right;//矩形右下角横坐标
		long bottom;//矩形右下角纵坐标
	};

	bool Cam_AutoAWB(LPBYTE pBuffer,long IWidth, long IHeight,double& iRed,double& iGreen,double& iBlue,RECT &MaxRec);
	void selectWhiteBlance(LPBYTE pBuffer,RECT pRect,int iWidth, int iHeight,double &kR,double &kG,double &kB);
	int SmoothMax( float* pValue, int w, int h, int matrix, float* max );
	float GetSmooth(int w,int h,const RECT& r,unsigned char *pData);

	bool bDebug;
};

