#include "StdAfx.h"
#include "CutConvert.h"


CCutConvert::CCutConvert(void)
{
	bDebug=false;
	InitializeCriticalSection(&m_crtSec);
}


CCutConvert::~CCutConvert(void)
{
}

bool CCutConvert::InitProcess(LPVOID p_param)
{
	mProcessV=(ProcessParas*)p_param;
	if (mProcessV)
		return true;
	return false;
}

bool CCutConvert::PreProcessImg(HVideoHeader* pOriginHeader,HVideoHeader* pEndHeader)
{
	EnterCriticalSection(&m_crtSec);
	int iX=mProcessV->iParas[0];
	int iY=mProcessV->iParas[1];
	int iW=mProcessV->iParas[2];
	int iH=mProcessV->iParas[3];
	LeaveCriticalSection(&m_crtSec);

	if (pOriginHeader->Vwidth<=(iX+iW)||pOriginHeader->Vheight<=(iY+iH))
		return false;

	if (pEndHeader->Vbuffer)
	{
		delete pEndHeader->Vbuffer;
		pEndHeader->Vbuffer=0;
	}
	pEndHeader->Vwidth=iW;
	pEndHeader->Vheight=iH;
	pEndHeader->VwBit=pOriginHeader->VwBit;
	pEndHeader->fPixelSize=pOriginHeader->fPixelSize;
	pEndHeader->Vsize=iH*iW*pEndHeader->VwBit/8;
	pEndHeader->Vbuffer=(LPBYTE)malloc(pEndHeader->Vsize);
	if (iX>=0&&iY>=0&&iW>5&&iH>5)
	{
		int iEX=iX+iW;
		int iEY=iY+iH;
		int iDx=0,iDy=0;
		int iBit=pEndHeader->VwBit/8;
		int iStep=pEndHeader->Vwidth*iBit;
		int iStepOri=pOriginHeader->Vwidth*pOriginHeader->VwBit/8;

		int width=pOriginHeader->Vwidth;
		int height=pOriginHeader->Vheight;
		if (bDebug)
		{	
			cv::Mat mImg;
			if (iBit==3)
				mImg=cv::Mat(height,width,CV_8UC3,pOriginHeader->Vbuffer);		
			else if(iBit==1)
				mImg=cv::Mat(height,width,CV_8U,pOriginHeader->Vbuffer);
			cv::namedWindow("Before PreProcessImg");
			cv::imshow("Before PreProcessImg",mImg);
		}
		for(int y = iY; y < iEY; y++)
		{
			for (int x = iX; x < iEX; x++)
			{
				for (int j=0;j<iBit;j++)
				{
					//((uchar*)(pEndHeader->Vbuffer + iStep*iDy))[iDx*iBit+j] = 
					//	((uchar*)(pOriginHeader->Vbuffer + iStepOri*y))[x*iBit+j];
					((uchar*)(pEndHeader->Vbuffer + iStep*(iH-1-iDy)))[iDx*iBit+j] = 
						((uchar*)(pOriginHeader->Vbuffer + iStepOri*(height-1-y)))[x*iBit+j];	
				}
				iDx++;
			}
			iDy++;
			iDx=0;
		}

		if (bDebug)
		{	
			cv::Mat mImg;
			if (iBit==3)
				mImg=cv::Mat(pEndHeader->Vheight,pEndHeader->Vwidth,CV_8UC3,pEndHeader->Vbuffer);		
			else if(iBit==1)
				mImg=cv::Mat(pEndHeader->Vheight,pEndHeader->Vwidth,CV_8U,pEndHeader->Vbuffer);
			cv::namedWindow("After PreProcessImg");
			cv::imshow("After PreProcessImg",mImg);
		}

		return true;
	}
	return false;
}
