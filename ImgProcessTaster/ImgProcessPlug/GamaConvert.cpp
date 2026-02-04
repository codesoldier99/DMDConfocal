#include "StdAfx.h"
#include "GamaConvert.h"


CGamaConvert::CGamaConvert(void)
{
	bDebug=false;
	InitializeCriticalSection(&m_crtSec);
	mfOld=0.0f;
}


CGamaConvert::~CGamaConvert(void)
{
}

bool CGamaConvert::InitProcess(LPVOID p_param)
{
	mProcessV=(ProcessParas*)p_param;
	if (mProcessV)
	{
		if (mfOld!=mProcessV->fParas[0])
			InitGamaValue();
		return true;
	}
		
	return false;
}

bool CGamaConvert::PreProcessImg(HVideoHeader* pOriginHeader,HVideoHeader* pEndHeader)
{
	int wBit=pOriginHeader->VwBit/8;
	if (wBit!=1)
		return false;
	int width=pOriginHeader->Vwidth;
	int height=pOriginHeader->Vheight;	
	cv::Mat mOldImg=cv::Mat(height,width,CV_8U,pOriginHeader->Vbuffer);
	if (!pEndHeader->Vbuffer||pEndHeader->Vsize!=pOriginHeader->Vsize)
	{
		if (pEndHeader->Vbuffer)
			delete pEndHeader->Vbuffer;
		pEndHeader->VwBit=pOriginHeader->VwBit;
		pEndHeader->Vheight=pOriginHeader->Vheight;
		pEndHeader->Vwidth=pOriginHeader->Vwidth;
		pEndHeader->Vsize=pOriginHeader->Vsize;
		pEndHeader->Vbuffer=(LPBYTE)malloc(pEndHeader->Vsize);
	}
	pEndHeader->fPixelSize=pOriginHeader->fPixelSize;
	bool bReturn=true;	
	if (bDebug)
	{	
		cv::namedWindow("Before PreProcessImg");
		cv::imshow("Before PreProcessImg",mOldImg);
	}
	int iStep=mOldImg.step;//直接在寻坏里读取mOldImg.step很消耗时间
	EnterCriticalSection(&m_crtSec);
	int val=0;
	for(int y = 0; y < mOldImg.rows; y++)
	{
		for (int x = 0; x < mOldImg.cols; x++)
		{
			val = ((uchar*)(pOriginHeader->Vbuffer + iStep*y))[x]; 
			((uchar*)(pEndHeader->Vbuffer + iStep*y))[x] = mGamaV[val];
		}
	}
	LeaveCriticalSection(&m_crtSec);
	if (bDebug)
	{
		cv::Mat mImg;
		mImg=cv::Mat(pEndHeader->Vheight,pEndHeader->Vwidth,CV_8U,pEndHeader->Vbuffer);
		cv::namedWindow("After PreProcessImg");
		cv::imshow("After PreProcessImg",mImg);
	}
	return bReturn;
}

int CGamaConvert::ImageAdjust(cv::Mat src, cv::Mat dst, double low, double high, 
	double bottom, double top,double gamma )
{
	if(low<0 && low>1 && high <0 && high>1&&bottom<0 && bottom>1 && top<0 && top>1 && low>high)
		return -1;
	double low2 = low*255;
	double high2 = high*255;
	double bottom2 = bottom*255;
	double top2 = top*255;
	double err_in = high2 - low2;
	double err_out = top2 - bottom2;
	int x,y;
	double val;
	for( y = 0; y < src.rows; y++)
	{
		for (x = 0; x < src.cols; x++)
		{
			val = ((uchar*)(src.data + src.step*y))[x]; 
			val = pow((val - low2)/err_in, gamma) * err_out + bottom2;
			if(val>255) val=255; if(val<0) val=0; // Make sure src is in the range [low,high]
			((uchar*)(dst.data + dst.step*y))[x] = (uchar) val;
		}
	}
	return 0;
}

void CGamaConvert::InitGamaValue()
{
	double val;
	float fGamma=mProcessV->fParas[0];
	for (int i=0;i<256;i++)
	{
		val = pow(i/255.0f, fGamma) * 255.0f;
		if(val>255) val=255; if(val<0) val=0;
		mGamaV[i] = (uchar) val;
	}
	mfOld=fGamma;
}
