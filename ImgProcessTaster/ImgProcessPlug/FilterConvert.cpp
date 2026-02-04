#include "StdAfx.h"
#include "FilterConvert.h"


CFilterConvert::CFilterConvert(void)
{
	bDebug=false;
	InitializeCriticalSection(&m_crtSec);
}


CFilterConvert::~CFilterConvert(void)
{
}

bool CFilterConvert::InitProcess(LPVOID p_param)
{
	mProcessV=(ProcessParas*)p_param;
	if (mProcessV)
		return true;
	return false;
}

bool CFilterConvert::PreProcessImg(HVideoHeader* pOriginHeader,HVideoHeader* pEndHeader)
{
	EnterCriticalSection(&m_crtSec);
	int iScale=mProcessV->iParas[0];
	LeaveCriticalSection(&m_crtSec);
	if (iScale<=1)
		return false;

	int width=pOriginHeader->Vwidth;
	int height=pOriginHeader->Vheight;
	int wBit=pOriginHeader->VwBit/8;//这里图的位数
	cv::Mat srcImg;
	if (wBit==3)
		srcImg =cv::Mat(height,width,CV_8UC3,pOriginHeader->Vbuffer);
	else if(wBit==1)
		srcImg =cv::Mat(height,width,CV_8U,pOriginHeader->Vbuffer);

	Size mSize=cv::Size();	
	mSize.width= srcImg.size().width/iScale;
	mSize.height= srcImg.size().height/iScale;
	if (mSize.width<=10||mSize.height<=10)
	{
		return false;
	}
	Mat dstImg;
	if (bDebug)
	{
		cv::namedWindow("Before PreProcessImg",0);
		cv::imshow("Before PreProcessImg", srcImg);
	}	

	cv::GaussianBlur(srcImg, dstImg, Size(15, 15), 11, 11);//高斯滤波

	//更新图像大小信息
	pEndHeader->Vheight=mSize.height;
	pEndHeader->Vwidth=mSize.width;
	pEndHeader->VwBit=pOriginHeader->VwBit;
	pEndHeader->fPixelSize=pOriginHeader->fPixelSize*iScale;
	pEndHeader->Vsize=pEndHeader->Vheight*dstImg.step;
	if (pEndHeader->Vbuffer)
		delete(pEndHeader->Vbuffer); 

	pEndHeader->Vbuffer=(LPBYTE)malloc(pEndHeader->Vsize);
	memcpy(pEndHeader->Vbuffer, dstImg.data,pEndHeader->Vsize);
	if (bDebug)
	{
		cv::namedWindow("After PreProcessImg",0);
		cv::imshow("After PreProcessImg", dstImg);
	}
	imwrite("c:\\Res.bmp", dstImg);

	return true;
}

