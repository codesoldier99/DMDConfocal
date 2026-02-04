#include "StdAfx.h"
#include "ScaleConvert.h"


CScaleConvert::CScaleConvert(void)
{
	bDebug=false;
	InitializeCriticalSection(&m_crtSec);
}


CScaleConvert::~CScaleConvert(void)
{
}

bool CScaleConvert::InitProcess(LPVOID p_param)
{
	mProcessV=(ProcessParas*)p_param;
	if (mProcessV)
		return true;
	return false;
}

bool CScaleConvert::PreProcessImg(HVideoHeader* pOriginHeader,HVideoHeader* pEndHeader)
{
	EnterCriticalSection(&m_crtSec);
	int iScale=mProcessV->iParas[0];
	LeaveCriticalSection(&m_crtSec);
	if (iScale<=1)
		return false;

	int width=pOriginHeader->Vwidth;
	int height=pOriginHeader->Vheight;
	int wBit=pOriginHeader->VwBit/8;//这里图的位数
	cv::Mat mImg;
	if (wBit==3)
		mImg=cv::Mat(height,width,CV_8UC3,pOriginHeader->Vbuffer);		
	else if(wBit==1)
		mImg=cv::Mat(height,width,CV_8U,pOriginHeader->Vbuffer);

	Size mSize=cv::Size();	
	mSize.width=mImg.size().width/iScale;
	mSize.height=mImg.size().height/iScale;	
	if (mSize.width<=10||mSize.height<=10)
	{
		return false;
	}
	Mat mOrtImg;
	if (bDebug)
	{
		cv::namedWindow("Before PreProcessImg");
		cv::imshow("Before PreProcessImg",mImg);
	}	

	cv::resize(mImg,mOrtImg,mSize);//缩略图

	//更新图像大小信息
	pEndHeader->Vheight=mSize.height;
	pEndHeader->Vwidth=mSize.width;
	pEndHeader->VwBit=pOriginHeader->VwBit;
	pEndHeader->fPixelSize=pOriginHeader->fPixelSize*iScale;
	pEndHeader->Vsize=pEndHeader->Vheight*mOrtImg.step;
	if (pEndHeader->Vbuffer)
		delete(pEndHeader->Vbuffer); 

	pEndHeader->Vbuffer=(LPBYTE)malloc(pEndHeader->Vsize);
	memcpy(pEndHeader->Vbuffer,mOrtImg.data,pEndHeader->Vsize);
	if (bDebug)
	{
		cv::namedWindow("After PreProcessImg");
		cv::imshow("After PreProcessImg",mOrtImg);
	}
	return true;
}

