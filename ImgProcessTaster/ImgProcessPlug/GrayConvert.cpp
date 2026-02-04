#include "StdAfx.h"
#include "GrayConvert.h"


CGrayConvert::CGrayConvert(void)
{
	bDebug=false;
	InitializeCriticalSection(&m_crtSec);
}


CGrayConvert::~CGrayConvert(void)
{
}

bool CGrayConvert::InitProcess(LPVOID p_param)
{
	mProcessV=(ProcessParas*)p_param;
	if (mProcessV)
		return true;
	return false;
}

bool CGrayConvert::PreProcessImg(HVideoHeader* pOriginHeader,HVideoHeader* pEndHeader)
{
	int wBit=pOriginHeader->VwBit/8;
	if (wBit==1)
		return false;
	int width=pOriginHeader->Vwidth;
	int height=pOriginHeader->Vheight;	
	cv::Mat mOldImg=cv::Mat(height,width,CV_8UC3,pOriginHeader->Vbuffer);
	//if (!pEndHeader->Vbuffer)
	//{
	//	pEndHeader->VwBit=pOriginHeader->VwBit;
	//	pEndHeader->Vheight=pOriginHeader->Vheight;
	//	pEndHeader->Vwidth=pOriginHeader->Vwidth;
	//	pEndHeader->Vsize=pOriginHeader->Vsize;
	//	pEndHeader->Vbuffer=(LPBYTE)malloc(pEndHeader->Vsize);
	//}
	cv::Mat mImg=cv::Mat(height,width,CV_8U);

	if (bDebug)
	{
		cv::namedWindow("Before PreProcessImg");
		cv::imshow("Before PreProcessImg",mOldImg);
	}
	EnterCriticalSection(&m_crtSec);
	CString str=mProcessV->strParas[0];
	LeaveCriticalSection(&m_crtSec);
	if (str==L"B")
		cv::extractChannel(mOldImg,mImg,0);
	else if (str==L"G")
		cv::extractChannel(mOldImg,mImg,1);
	else if (str==L"R")
		cv::extractChannel(mOldImg,mImg,2);
	else 
		cvtColor(mOldImg,mImg,CV_RGB2GRAY);

	if (pEndHeader->VwBit!=8||!pEndHeader->Vbuffer||pEndHeader->Vsize!=pOriginHeader->Vsize/3)
	{
		if (pEndHeader->Vbuffer)
			delete(pEndHeader->Vbuffer); 
		pEndHeader->VwBit=8;
		pEndHeader->Vsize=pOriginHeader->Vsize/3;
		pEndHeader->Vheight=pOriginHeader->Vheight;
		pEndHeader->Vwidth=pOriginHeader->Vwidth;
		pEndHeader->Vbuffer=(LPBYTE)malloc(pEndHeader->Vsize);
	}
	pEndHeader->fPixelSize=pOriginHeader->fPixelSize;
	memcpy(pEndHeader->Vbuffer,mImg.data,pEndHeader->Vsize);
	if (bDebug)
	{
		cv::namedWindow("After PreProcessImg");
		cv::imshow("After PreProcessImg",mImg);
	}
	return true;
}

void CGrayConvert::SavePic( HVideoHeader* pHeader,CString pName,int pType)
{
	SaveImage(pHeader,pName,FLIP_VERTICAL);
}

void CGrayConvert::SetDebug(bool bV)
{
	bDebug=bV;
}
