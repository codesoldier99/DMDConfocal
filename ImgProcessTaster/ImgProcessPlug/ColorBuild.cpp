#include "StdAfx.h"
#include "ColorBuild.h"


CColorBuild::CColorBuild(void)
{
}


CColorBuild::~CColorBuild(void)
{
}

bool CColorBuild::InitProcess(LPVOID p_param)
{
	vector<cv::Mat>().swap(mImgs);
	mImgs.clear();
	return true;
}

void CColorBuild::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
{
	//HvideoHeader里面有输入的图片大小 及 编号， pbuffer是图像数据
	ReBuildImgs(pHeader,pBuffer);
}

void CColorBuild::ReBuildImgs( HVideoHeader* pHeader,LPBYTE pBuffer)
{
	int width=pHeader->Vwidth;
	int height=pHeader->Vheight;
	int index=int(pHeader->VcaptureTime);//这里暂用capture time代替编号
	int count=pHeader->VnCount;//这里代表有多少张图合成
	//复制新的内存
	//LPBYTE pBufferNew=(LPBYTE)malloc(pHeader->Vsize);
	//memcpy(pBufferNew,pBuffer,pHeader->Vsize);
	//复制新的内存
	cv::Mat frame;
	int wBit=pHeader->VwBit/8;//这里图的位数
	cv::Mat mImg;
	if (wBit==3)
	{
		frame=cv::Mat(height,width,CV_8UC3);//,pBufferNew);
		cvtColor( frame, frame, CV_RGB2GRAY );
	}		
	else if(wBit==1)
		frame=cv::Mat(height,width,CV_8U);//,pBufferNew);
	memcpy(frame.data,pBuffer,pHeader->Vsize);

	mImgs.push_back(frame);
	if (mImgs.size()==3)//融合结束
	{
		mGlobeImg=cv::Mat(mImgs[0].size(), CV_8UC3);
		cv::merge(mImgs,mGlobeImg);
		cvtColor( mGlobeImg, mGlobeImg, CV_RGB2BGR);
		ResultNotify();//合成结束
	}
}

void CColorBuild::ResultNotify()
{
	HVideoHeader* pResultHeader=new HVideoHeader();
	pResultHeader->Vwidth=mGlobeImg.cols;
	pResultHeader->Vheight=mGlobeImg.rows;
	pResultHeader->Vsize=mGlobeImg.rows*mGlobeImg.cols*mGlobeImg.channels();
	pResultHeader->Vbuffer=(LPBYTE)malloc(pResultHeader->Vsize);
	memcpy(pResultHeader->Vbuffer,mGlobeImg.data,pResultHeader->Vsize);
	pResultHeader->VwBit=mGlobeImg.channels()*8;
	Notify(this,NOTIFY_COLORBUILD_END,0,pResultHeader);
	delete(pResultHeader);
}

