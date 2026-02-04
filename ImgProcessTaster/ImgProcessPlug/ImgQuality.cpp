#include "StdAfx.h"
#include "ImgQuality.h"
#include "GraphUtils.h"
#include "XscFitLines.h"

CImgQuality::CImgQuality(void)
{
}

CImgQuality::~CImgQuality(void)
{
}

bool CImgQuality::InitProcess(LPVOID p_param)
{
	if (p_param!=0)
	{
		mAutoFocusParas=(AutoFocusParas*)p_param;
		dCuValue.clear();
		dXiValue.clear();
		return true;
	}
	return false;
}

bool CImgQuality::UnInitProcess()
{

	return true;
}

bool CImgQuality::ProcessImg(void* InputPara,void* OutPutPara,HVideoHeader* pHeader)
{
	Quality_TYPE mV=*(Quality_TYPE*)InputPara;
	double* dReV=(double*)OutPutPara;
	switch(mV)
	{
		case ImgQuality_CuQuality:
			if (mAutoFocusParas)
				*dReV=GetCuFocus(pHeader,pHeader->Vbuffer,mAutoFocusParas->mCuScale);
			break;
		case ImgQuality_XiQuality:
			if (mAutoFocusParas)
				*dReV=GetXiFocus(pHeader,pHeader->Vbuffer,mAutoFocusParas->mXiScale);
			break;
		case ImgQuality_CuEnd:
			if (mAutoFocusParas)
			{
				double mMax=-1;
				int mIdx=0;
				GetMax(dCuValue,&mMax,&mIdx,0);
				int iMid=dCuValue.size()-mIdx;//靠右边个数
				int iStart=0,iEnd=dCuValue.size();
				if (iMid>mIdx)//最大值靠左
					iEnd=mIdx+mIdx+1;
				else//最大值靠右
					iStart=mIdx-iMid;
				*dReV=GetPeak(dCuValue,iStart,iEnd);
			}				
			break;
		case ImgQuality_XiEnd:
			if (mAutoFocusParas)
			{
				double mMax=-1;
				int mIdx=0;
				GetMax(dXiValue,&mMax,&mIdx,0);
				int iMid=dXiValue.size()-mIdx;//靠右边个数
				int iStart=0,iEnd=dXiValue.size();
				if (iMid>mIdx)//最大值靠左
					iEnd=mIdx+mIdx+1;
				else//最大值靠右
					iStart=mIdx-iMid;
				*dReV=GetPeak(dXiValue,iStart,iEnd);

				//*dReV=GetPeak(dXiValue);
			}			
			break;
		case ImgQuality_CuInvert:
			if (mAutoFocusParas)
				std::reverse(dCuValue.begin(),dCuValue.end());
			break;
		case ImgQuality_XiInvert:
			if (mAutoFocusParas)
				std::reverse(dXiValue.begin(),dXiValue.end());
			break;
	}
	//OutPutPara=&dReV;
	return true;
}

double CImgQuality::GetCuFocus(HVideoHeader* pHeader,LPBYTE pBuffer,int iScale)
{
	//return GetXiFocus(pHeader,pBuffer,iScale);
	int width=pHeader->Vwidth;
	int height=pHeader->Vheight;
	int index=int(pHeader->VcaptureTime);//这里暂用capture time代替编号
	int wBit=pHeader->VwBit/8;//这里图的位数
	cv::Mat mImg;
	if (wBit==3)
	{
		cv::Mat mOldImg=cv::Mat(height,width,CV_8UC3,pBuffer);
		cvtColor(mOldImg,mImg,CV_RGB2GRAY);
	}		
	else if(wBit==1)
		mImg=cv::Mat(height,width,CV_8U,pBuffer);

	Size mSize=cv::Size();
	mSize.width=mImg.size().width/iScale;
	mSize.height=mImg.size().height/iScale;
	Mat mOrtImg;
	cv::resize(mImg,mOrtImg,mSize);//缩略图
	cv::Scalar mVmean,mVstd;
	cv::meanStdDev(mOrtImg,mVmean,mVstd);
	double mCuValue=mVstd[0];
	dCuValue.push_back(mCuValue);
	return mCuValue;
}

double CImgQuality::GetXiFocus(HVideoHeader* pHeader,LPBYTE pBuffer,int iScale)
{
	int width=pHeader->Vwidth;
	int height=pHeader->Vheight;
	int index=int(pHeader->VcaptureTime);//这里暂用capture time代替编号
	int wBit=pHeader->VwBit/8;//这里图的位数
	cv::Mat mImg;
	if (wBit==3)
	{
		cv::Mat mOldImg=cv::Mat(height,width,CV_8UC3,pBuffer);
		cvtColor(mOldImg,mImg,CV_RGB2GRAY);
	}		
	else if(wBit==1)
		mImg=cv::Mat(height,width,CV_8U,pBuffer);

	Size mSize=cv::Size();
	mSize.width=mImg.size().width/iScale;
	mSize.height=mImg.size().height/iScale;
	Mat mOrtImg;
	cv::resize(mImg,mOrtImg,mSize);//缩略图
	Mat mGradImg;
	GetAbsGradient(mOrtImg,mGradImg);//获得梯度图

	cv::Scalar mVmean,mVstd;
	cv::meanStdDev(mGradImg,mVmean,mVstd);
	double mXiCalue=mVmean[0];
	dXiValue.push_back(mXiCalue);
	return mXiCalue;
}

void CImgQuality::GetAbsGradient(Mat& src, Mat& des)
{
	Mat gray, edge, edge8uM,edge8uV;//,gradImg;
	edge = cv::Mat(src.size(), CV_16S);
	edge8uM= cv::Mat(src.size(), CV_8U);
	edge8uV= cv::Mat(src.size(), CV_8U);
	// 将源图像转为灰度图像
	if (src.channels() == 1)
		src.copyTo(gray);
	else
		cvtColor( src, gray, CV_RGB2GRAY );
	//cv::normalize(gray,gray,255.0,0,NORM_MINMAX);
	// 边缘检测
	Sobel(gray, edge, CV_16S, 0, 2);
	cv::convertScaleAbs(edge,edge8uM);
	Sobel(gray, edge, CV_16S, 2, 0);
	cv::convertScaleAbs(edge,edge8uV);
	cv::add(edge8uM,edge8uV,edge8uV);
	//cvConvertScaleAbs(&edge,&edge8u);
	//edge.convertTo(edge8u, CV_8U);
	// 将边缘检测图像转至输出图像格式
	if(des.channels() == 1)
		edge8uV.copyTo(des);
	else
		cvtColor( edge8uV, des, CV_GRAY2BGR );

	if (mAutoFocusParas->bDebug)
	{
		cv::namedWindow("GrayImg",0);
		cv::imshow("GrayImg",gray);
		cv::namedWindow("GradientImg",0);
		cv::imshow("GradientImg",edge8uV);
	}
}

void CImgQuality::SavePic( HVideoHeader* pHeader,CString pName,int pType)
{
	SaveImage(pHeader,pName,FLIP_VERTICAL);
}

double CImgQuality::GetPeak(vector<double> mV)
{
	if (mV.size()<=0)
		return 0.0;
	float* floatVec=new float[mV.size()];
	for (int i=0;i<mV.size();i++)
		*(floatVec+i)=mV[i];
	//////////////////去拟合曲线////////////
	vector<double> mXV;
	for (int i=0;i<mV.size();i++)
		mXV.push_back(i);
	xsc::Fit mFit;
	mFit.polyfit(mXV,mV,2,true);
	////////求拟合曲线的最大值////////////
	//CString strFun(_T("y=")),strTemp(_T(""));
	//for (int i=0;i<mFit.getFactorSize();++i)
	//{
	//	if (0 == i)
	//		strTemp.Format(_T("%g"),mFit.getFactor(i));
	//	else
	//	{
	//		double fac = mFit.getFactor(i);
	//		if (fac<0)
	//			strTemp.Format(_T("%gx^%d"),fac,i);
	//		else
	//			strTemp.Format(_T("+%gx^%d"),fac,i);
	//	}
	//	strFun += strTemp;
	//}
	if(mAutoFocusParas->bDebug)
	{
		IplImage* graphImage =drawFloatGraph(floatVec, mV.size(), NULL, 0, 0, 500, 500); 
		float* floatVec2=new float[mV.size()];
		for (int i=0;i<mV.size();i++)
			*(floatVec2+i)=mFit.getFactor(2)*i*i+mFit.getFactor(1)*i+mFit.getFactor(0);
		//然后再多绘制图像
		drawFloatGraph(floatVec2, mV.size(), graphImage, 0, 0, 500, 500); 
		cvNamedWindow("Peak Lines");
		cvShowImage("Peak Lines",graphImage);
		//AfxMessageBox(strFun);
		delete floatVec2;
	}	
	delete floatVec;
	double mR=-mFit.getFactor(1)/(2*mFit.getFactor(2));
	return mR;
}

void CImgQuality::GetMax(vector<double> mValues,double *Max,int *Idx,int iStart)
{
	*Max=-1;
	for (int i=iStart;i<mValues.size();i++)
	{
		if (mValues[i]>*Max)
		{
			*Max=mValues[i];
			*Idx=i;
		}
	}
}

double CImgQuality::GetPeak(vector<double> mV,int iStart,int iEnd)
{
	if (mV.size()<=0)
		return 0.0;
	//////////////////去拟合曲线////////////
	vector<double> mXV;
	vector<double> mYV;
	for (int i=iStart;i<iEnd;i++)
	{
		mXV.push_back(i-iStart);
		mYV.push_back(mV[i]);
	}
	xsc::Fit mFit;
	mFit.polyfit(mXV,mYV,2,true);
	mXV.clear();mYV.clear();
	if(mAutoFocusParas->bDebug)
	{
		float* floatVec=new float[iEnd-iStart];
		for (int j=iStart;j<iEnd;j++)
		{
			int i=j-iStart;
			*(floatVec+i)=mV[j];
			mXV.push_back(*(floatVec+i));
		}	
		IplImage* graphImage =drawFloatGraph(floatVec,iEnd-iStart, NULL, 0, 0, 500, 500); 

		float* floatVec2=new float[iEnd-iStart];
		for (int j=iStart;j<iEnd;j++)
		{
			int i=j-iStart;
			*(floatVec2+i)=mFit.getFactor(2)*i*i+mFit.getFactor(1)*i+mFit.getFactor(0);
			mYV.push_back(*(floatVec2+i));
		}			
		drawFloatGraph(floatVec2, iEnd-iStart, graphImage, 0, 0, 500, 500); 

		cvNamedWindow("Peak Lines");
		cvShowImage("Peak Lines",graphImage);
		delete floatVec2;
		delete floatVec;
		mXV.clear();mYV.clear();
	}	
	double mR=-mFit.getFactor(1)/(2*mFit.getFactor(2))+iStart;
	return mR;
}
