#include "StdAfx.h"
#include "ReBuild.h"


CReBuild::CReBuild(void)
{
	m_HeightBuffer=0;
	m_RenderBuffer=0;
	bNewBegin=true;
}

CReBuild::~CReBuild(void)
{
}

bool CReBuild::InitProcess(LPVOID p_param)
{
	if (p_param!=0)
	{
		ReBuildParas* mV=(ReBuildParas*)p_param;
		mParas.iIdxSize=mV->iIdxSize;
		mParas.iImgSize=mV->iImgSize;
		mParas.iImgStep=mV->iImgStep;
		mParas.iScaleV=mV->iScaleV;
		/////////////////////////如果有背景读取//////////////////////////////////
		if (mParas.strBKImg!=mV->strBKImg&&mV->strBKImg!=L"")
		{
			mParas.strBKImg=mV->strBKImg;
			std::string tempName=(LPCSTR)CStringA(mParas.strBKImg);
			const char *tmp=tempName.c_str();
			Mat mBKImg = cvLoadImage(tmp,CV_LOAD_IMAGE_GRAYSCALE);
			if (mBKImg.data)
			{
				cv::flip(mBKImg,mBKImg,0);
				mBinaryImg=cv::Mat(mBKImg.rows,mBKImg.cols,CV_8U);
				cv::threshold(mBKImg,mBinaryImg,150,255,CV_THRESH_BINARY);
			}		
		}
		/////////////////////////如果有背景图，进行背景图去除//////////////////////////////////
	}
	vector<cv::Mat>().swap(mImgs);//释放资源
	mImgs.clear();
	vector<cv::Mat>().swap(mGradImgs);
	mGradImgs.clear();
	return true;
}

bool CReBuild::UnInitProcess()
{
	if(m_HeightBuffer)
		delete(m_HeightBuffer);
	if(m_RenderBuffer)
		delete(m_RenderBuffer);
	m_HeightBuffer=0;
	m_RenderBuffer=0;
	vector<cv::Mat>().swap(mImgs);
	mImgs.clear();
	vector<cv::Mat>().swap(mGradImgs);
	mGradImgs.clear();
	return true;
}

void CReBuild::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
{
	//HvideoHeader里面有输入的图片大小及编号，pbuffer是图像数据
	if (bNewBegin)
	{
		bNewBegin=false;
		vector<cv::Mat>().swap(mImgs);
		mImgs.clear();

		iTotal=0;
	}		
	ReBuildImgs(pHeader,pBuffer);
}

bool CReBuild::ProcessImg(void* InputPara,void* OutPutPara,HVideoHeader* pHeader)
{
	if (InputPara==0||OutPutPara==0)
		return false;
	if (mImgs.size()<=0)
		return false;
	int iCur=*(int*)InputPara;
	//int iTotal=mImgs.size();
	*(int*)OutPutPara=iTotal;

	//CString str;
	//str.Format(L"%d",iCur);
	//AfxMessageBox(str);

	iCur=iCur>(iTotal-1)?0:iCur;
	iCur=iCur<0?(iTotal-1):iCur;
	if (iCur>=mImgs.size())
	{
		iTotal=mImgs.size();
		iCur=iCur>(iTotal-1)?0:iCur;
		iCur=iCur<0?(iTotal-1):iCur;
	}
	//str.Format(L"%d",iCur);
	//AfxMessageBox(str);

	*(int*)InputPara=iCur;
	cv::Mat mCurImg=mImgs[iCur];
	if (pHeader)
	{
		if (pHeader->Vbuffer)
		{
			if (pHeader->Vwidth!=mCurImg.cols||pHeader->Vheight!=mCurImg.rows||
				pHeader->VwBit!=mCurImg.channels()*8||pHeader->Vsize==0)
			{
				delete pHeader->Vbuffer;
				pHeader->Vwidth=mCurImg.cols;
				pHeader->Vheight=mCurImg.rows;
				pHeader->VwBit=mCurImg.channels()*8;
				pHeader->Vsize=mCurImg.cols*pHeader->Vheight*mCurImg.channels();
				pHeader->Vbuffer=(LPBYTE)malloc(pHeader->Vsize);
			}
		}
		else
		{
			pHeader->Vwidth=mCurImg.cols;
			pHeader->Vheight=mCurImg.rows;
			pHeader->VwBit=mCurImg.channels()*8;
			pHeader->Vsize=mCurImg.cols*pHeader->Vheight*mCurImg.channels();
			pHeader->Vbuffer=(LPBYTE)malloc(pHeader->Vsize);
		}
	}
	else
	{
		pHeader->Vwidth=mCurImg.cols;
		pHeader->Vheight=mCurImg.rows;
		pHeader->VwBit=mCurImg.channels()*8;
		pHeader->Vsize=mCurImg.cols*pHeader->Vheight*mCurImg.channels();
		pHeader->Vbuffer=(LPBYTE)malloc(pHeader->Vsize);
	}
	memcpy(pHeader->Vbuffer,mCurImg.data,pHeader->Vsize);
	return true;
}

void CReBuild::ReBuildImgs( HVideoHeader* pHeader,LPBYTE pBuffer)
{
	int width=pHeader->Vwidth;
	int height=pHeader->Vheight;
	int index=int(pHeader->VcaptureTime);//这里暂用capture time代替编号
	int count=pHeader->VnCount;//这里代表有多少张图合成
	iTotal=pHeader->VnCount;
	//复制新的内存
	//LPBYTE pBufferNew=(LPBYTE)malloc(pHeader->Vsize);
	//memcpy(pBufferNew,pBuffer,pHeader->Vsize);
	//复制新的内存
	cv::Mat frame;//=cv::Mat(height,width,CV_8U,pBufferNew);
	int wBit=pHeader->VwBit/8;//这里图的位数
	cv::Mat mImg;
	if (wBit==3)
		frame=cv::Mat(height,width,CV_8UC3);//,pBufferNew);	
	else if(wBit==1)
		frame=cv::Mat(height,width,CV_8U);//,pBufferNew);

	//复制内存
	memcpy(frame.data,pBuffer,pHeader->Vsize);

	//图像非正方形的话，取它的中心部分
	if (height>width)
	{
		int iC=(height-width)/2;
		frame=frame(Rect(0,iC,width,width));
	}
	else if (width>height)
	{
		int iC=(width-height)/2;
		frame=frame(Rect(iC,0,height,height));
	}
	mImgs.push_back(frame);

	//////////////计算梯度图//////////////
	Mat mOrtImg;
	Size mSize=cv::Size();
	mSize.width=frame.size().width/mParas.iScaleV;
	mSize.height=frame.size().height/mParas.iScaleV;
	cv::resize(mImgs[mImgs.size()-1],mOrtImg,mSize);
	Mat mGradImg;
	GetAbsGradient(mOrtImg,mGradImg);//可以接收彩色图进去
	mGradImgs.push_back(mGradImg);
	/////////////计算梯度图//////////////
	if (mImgs.size()==count)//融合结束
	{
		int iStep=256/count;
		int iOldStep=mParas.iImgStep;
		if (iStep<mParas.iImgStep)
			mParas.iImgStep=iStep;
		mGlobeImg=cv::Mat(mImgs[0].size(), mImgs[0].type());
		mIdxImg=cv::Mat(mImgs[0].size(), CV_8U);

		//AfxMessageBox(L"开始合成图像！");

		OptionMerge mOptionMerge;
		mOptionMerge.fMax=-1;
		mOptionMerge.fmean=-1;
		mOptionMerge.iMaxIdx=-1;
		mOptionMerge.fstd=-1;
		SelectWindows(mImgs,mGradImgs,mGlobeImg,mIdxImg,mOptionMerge,8);
		//for (int i=0;i<5;i++)
		//{
		//	CString pName;
		//	pName.Format(L"E:\\All%03d.jpg",i);
		//	std::string tempName=(LPCSTR)CStringA(pName);
		//	const char* mN=tempName.c_str();
		//	imwrite(mN,mImgs[i]);
		//}
		bNewBegin=true;

		vector<cv::Mat>().swap(mGradImgs);
		mGradImgs.clear();

		ResultNotify();//合成结束
		mParas.iImgStep=iOldStep;
	}
	Notify(this,NOTIFY_3DSCAN_FLUSH,mImgs.size()-1);
}

void CReBuild::ResultNotify()
{
	if(m_HeightBuffer)
		delete(m_HeightBuffer);
	if(m_RenderBuffer)
		delete(m_RenderBuffer);
	m_HeightBuffer=(LPBYTE)malloc(mParas.iIdxSize*mParas.iIdxSize);
	
	/////////////////////////如果有背景图，进行背景图去除//////////////////////////////////(mBinaryImg)
	if (mBinaryImg.cols==mIdxImg.cols&&mBinaryImg.rows==mIdxImg.rows)
	{
		cv::bitwise_and(mBinaryImg,mIdxImg,mIdxImg);
		cv::bitwise_and(mBinaryImg,mGlobeImg,mGlobeImg);
	}
	/////////////////////////如果有背景图，进行背景图去除//////////////////////////////////

	Size mSize=cv::Size();
	mSize.width=mParas.iImgSize;
	mSize.height=mParas.iImgSize;
	Mat mImg1;
	cv::resize(mGlobeImg,mImg1,mSize);
	//cv::flip(mImg1,mImg1,0);//上下颠倒一下再传输
	m_RenderBuffer=(LPBYTE)malloc(mParas.iImgSize*mParas.iImgSize*mImg1.channels());
	memcpy(m_RenderBuffer,mImg1.data,mSize.width*mSize.height*mImg1.channels());

	mSize.width=mParas.iIdxSize;
	mSize.height=mParas.iIdxSize;
	Mat mImg2;
	cv::resize(mIdxImg,mImg2,mSize);
	//cv::flip(mImg2,mImg2,0);//上下颠倒一下再传输
	memcpy(m_HeightBuffer,mImg2.data,mSize.width*mSize.height);

	long lW=mParas.iImgSize;
	float fBit=mImg1.channels()*8;
	Notify(this,NOTIFY_RESULT_PIC,lW,m_RenderBuffer,fBit);//贴图消息	
	lW=mParas.iIdxSize;
	Notify(this,NOTIFY_RESULT_HEIGHT,lW,m_HeightBuffer);//高度图消息
	Notify(this,NOTIFY_RESULT_3D,0,0);
}

bool CReBuild::SelectWindows(vector<Mat>& Imgs,vector<Mat>& GradImg,Mat GlobeImg,
	Mat IdxImg,OptionMerge mOption,int iDev)
{
	int mImgStep=mParas.iImgStep;
	int mScaleV=mParas.iScaleV;

	int mW=GradImg[0].cols,
		mH=GradImg[0].rows;
	if (mW<=15)
		return false;
	int iSize=mW/iDev;
	int iTotalSize=iSize*iSize;
	for (int iW=0;iW<iDev;iW++)
	{
		for (int jH=0;jH<iDev;jH++)
		{
			vector<cv::Mat> mSmallGrads;
			vector<cv::Mat> mSmallOrgs;
			Mat mValue=cv::Mat(1,GradImg.size(), CV_32F);		
			int iX=iW*iSize,iY=jH*iSize;
			for (int i=0;i<GradImg.size();i++)
			{
				Mat frame=GradImg[i](Rect(iX,iY,iSize,iSize));//梯度图像
				mSmallGrads.push_back(frame);
				Scalar mV=cv::sum(frame);
				mValue.at<float>(0,i)=mV[0]/iTotalSize;

				Mat frameOrg=Imgs[i](Rect(iX*mScaleV,iY*mScaleV,iSize*mScaleV,iSize*mScaleV));//原图

				mSmallOrgs.push_back(frameOrg);
			}
			double dMax=0,dMin=0;
			cv::Point minPt=Point(0,0),maxPt=Point(0,0);
			cv::minMaxLoc(mValue,&dMin,&dMax,&minPt,&maxPt);
			cv::Scalar mVmean,mVstd;
			cv::meanStdDev(mValue,mVmean,mVstd);

			cv::Mat mPartImg=GlobeImg(Rect(iX*mScaleV,iY*mScaleV,iSize*mScaleV,iSize*mScaleV));
			cv::Mat mPartIdxImg=IdxImg(Rect(iX*mScaleV,iY*mScaleV,iSize*mScaleV,iSize*mScaleV));

			if (dMax<mVmean[0]+3*mVstd[0]&&dMin>mVmean[0]-3*mVstd[0])
			{
				if (mVmean[0]<mOption.fmean&&mVstd[0]<3*mOption.fstd)
				{
					Mat imgValue=Imgs[mOption.iMaxIdx](Rect(iX*mScaleV,iY*mScaleV,iSize*mScaleV,iSize*mScaleV));
					imgValue.copyTo(mPartImg);
					mPartIdxImg.setTo(mOption.iMaxIdx*mImgStep);
					continue;
				}
			}
			OptionMerge mOptionMerge;
			mOptionMerge.fMax=dMax;
			mOptionMerge.fmean=mVmean[0];
			mOptionMerge.iMaxIdx=maxPt.x;
			mOptionMerge.fstd=mVstd[0];    
			//////////////////////进行图像判断////////////////////////////////
			if (!SelectWindows(mSmallOrgs,mSmallGrads,mPartImg,mPartIdxImg,mOptionMerge,2))
			{
				Mat imgValue=Imgs[mOption.iMaxIdx](Rect(iX*mScaleV,iY*mScaleV,iSize*mScaleV,iSize*mScaleV));
				imgValue.copyTo(mPartImg);
				mPartIdxImg.setTo(mOption.iMaxIdx*mImgStep);
			}
		}
	}
	return true;
}

void CReBuild::SavePic( HVideoHeader* pHeader,CString pName,int pType)
{
	SaveImage(pHeader,pName,(FLIP_TYPE)pType);
}
