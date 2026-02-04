#include "StdAfx.h"
#include "CalImgValue.h"
#include <vector>
using std::vector;
#include "HGloableFunction.h"

CCalImgValue::CCalImgValue(void)
{
	//mCVCalMat=0;
#ifndef _DEBUG
	ReadExistCVMat();
#endif
	mBKBinaryImg=0;
}

CCalImgValue::~CCalImgValue(void)
{
}

bool CCalImgValue::InitProcess(LPVOID p_param)
{
	mCVParas=0;
	if (p_param!=0)
	{
		mCVParas=(CalCVParas*)p_param;
		//读取背景图像
		if (mCVParas->strBKImg!=L"")
		{
			std::string tempName=(LPCSTR)CStringA(mCVParas->strBKImg);
			const char *tmp=tempName.c_str();
			cv::Mat mBKImg = cvLoadImage(tmp,CV_LOAD_IMAGE_GRAYSCALE);
			cv::flip(mBKImg,mBKImg,0);//翻转，方向相反！
			mBKBinaryImg=cv::Mat(mBKImg.rows,mBKImg.cols,CV_8U);
			cv::threshold(mBKImg,mBKBinaryImg,150,255,CV_THRESH_BINARY);
			HERODE_XSC(mBKBinaryImg,mCVParas->iBKErode);
		}
		return true;
	}		
	return false;
}

bool CCalImgValue::ProcessImg(void* InputPara,void* OutPutPara,HVideoHeader* pHeader)
{
	if (!mCVParas)
		return false;

	Cal_TYPE mV=*(Cal_TYPE*)InputPara;
	//CalValueParas* dReV=(CalValueParas*)OutPutPara;
	int iCur=0;
	switch(mV)
	{
		case Cal_CV:
			CalCVValue(pHeader,pHeader->Vbuffer,(CalValueParas*)OutPutPara);
			break;
		case Cal_LightAvg://判断要几副图像来做 计算CV均值
			iCur=*(int*)OutPutPara;
			GetMask(pHeader,3,5,iCur);
			break;
		case Set_LightAvg:		
			UseMask(pHeader);
			break;
		default:
			break;
	}
	return true;
}

bool CCalImgValue::CalCVValue(HVideoHeader* pHeader,LPBYTE pBuffer,CalValueParas* Paras)
{
	int width=pHeader->Vwidth;
	int height=pHeader->Vheight;
	int wBit=pHeader->VwBit/8;//这里图的位数
	for (int i=0;i<256;i++)
		Paras->GrayHist[i]=0;
	cv::Mat mImg;
	if (wBit==3)
	{
		cv::Mat mOldImg=cv::Mat(height,width,CV_8UC3);
		memcpy(mOldImg.data,pBuffer,pHeader->Vsize);
		cvtColor(mOldImg,mImg,CV_RGB2GRAY);
	}		
	else if(wBit==1)
	{
		mImg=cv::Mat(height,width,CV_8U);
		memcpy(mImg.data,pBuffer,pHeader->Vsize);
	}
		
	//////////////先分割//////////////
	if (Paras->iTh>0)
	{
		cv::Mat mBinaryImg=cv::Mat(height,width,CV_8U);
		cv::threshold(mImg,mBinaryImg,Paras->iTh,255,CV_THRESH_BINARY);
		cv::bitwise_and(mImg,mBinaryImg,mBinaryImg);
		if (pHeader->Vbuffer)
			delete pHeader->Vbuffer;			
		pHeader->Vheight=height;
		pHeader->Vwidth=width;
		pHeader->VwBit=8;
		pHeader->Vsize=pHeader->Vheight*mBinaryImg.step;
		pHeader->Vbuffer=(LPBYTE)malloc(pHeader->Vsize);
		memcpy(pHeader->Vbuffer,mBinaryImg.data,pHeader->Vsize);
	}

	if (Paras->mROIRec.Width()>0&&Paras->mROIRec.Height()>0)
	{
		int iTotalWidth=Paras->mROIRec.Width()+Paras->mROIRec.left;
		int iTotalHeight=Paras->mROIRec.Height()+Paras->mROIRec.top;
		if (iTotalWidth>mImg.cols||iTotalHeight>mImg.rows)
		{
			AfxMessageBox(L"The selected ROI is bigger than the Picture！Cal the whold Pic!");
		}
		else
		{
			//////////切取一定的ROI，直接的ROI的数据源还是原本的，所以要重新复制一份！否则用指针访问像素数据的时候指向原本的大图！！！//////////////
			Mat mROIPic=mImg(Rect(Paras->mROIRec.left,Paras->mROIRec.top,Paras->mROIRec.Width(),Paras->mROIRec.Height()));
			Mat mROIPic2=cv::Mat(mROIPic.rows,mROIPic.cols,mROIPic.type());
			mROIPic.copyTo(mROIPic2);
			CalImgValue(Paras,mROIPic2);
			return true;
		}
	}	
	CalImgValue(Paras,mImg);
	return true;
}

void CCalImgValue::CalImgValue(CalValueParas* Paras,Mat mImg)
{
	if (Paras->iTh>0)
	{		
		vector<uchar> chValue;//做统计 
		uchar* pData = (uchar*)mImg.data; 
		GetValue(Paras,mImg);
		for (int i=0; i<mImg.rows; i++)  
		{  
			for (int j=0; j<mImg.cols; j++)  
			{  
				int iK=*(pData);
				if (iK>=Paras->iTh)
				{
					Paras->GrayHist[iK]++; 
					chValue.push_back(iK);
				}	
				else
				{
					iK+=0;
				}
				pData++;  
			}  
		}  
		Mat mVMid=Mat(chValue,true);
		GetValue(Paras,mVMid);
		chValue.clear();
	}
	else 
	{
		uchar* pData = (uchar*)mImg.data; 
		for (int i=0; i<mImg.rows; i++)  
		{  
			for (int j=0; j<mImg.cols; j++)  
			{  
				int iK=*(pData);
				Paras->GrayHist[iK]++; 			
				pData++;  
			}  
		}  
		pData=0;
		GetValue(Paras,mImg);
	}
}

void CCalImgValue::GetValue(CalValueParas* Paras,Mat mCalV)
{
	double dMax=0,dMin=0;
	cv::Point minPt=Point(0,0),maxPt=Point(0,0);
	cv::minMaxLoc(mCalV,&dMin,&dMax,&minPt,&maxPt);
	cv::Scalar mVmean,mVstd;
	cv::meanStdDev(mCalV,mVmean,mVstd);
	double mXiCalue=mVmean[0];
	Paras->MaxValue=dMax;
	Paras->MinValue=dMin;
	Paras->MeanValue=mXiCalue;
	Paras->SDValue=mVstd[0];
	Paras->CVValue=Paras->SDValue/Paras->MeanValue;
}

void CCalImgValue::GetMask(HVideoHeader* pHeader,int iAreaPer,int iScale,int iCur)
{
	int width=pHeader->Vwidth;
	int height=pHeader->Vheight;
	int wBit=pHeader->VwBit/8;//这里图的位数
	cv::Mat mImg;
	if (wBit==3)
	{
		cv::Mat mOldImg=cv::Mat(height,width,CV_8UC3);
		memcpy(mOldImg.data,pHeader->Vbuffer,pHeader->Vsize);
		cvtColor(mOldImg,mImg,CV_RGB2GRAY);
	}		
	else if(wBit==1)
	{
		mImg=cv::Mat(height,width,CV_8U);	
		memcpy(mImg.data,pHeader->Vbuffer,pHeader->Vsize);
	}

	if (mCurImg.cols!=mImg.cols&&mCurImg.rows!=mImg.rows&&mCurImg.type()!=mImg.type())
		mCurImg=cv::Mat(mImg.rows,mImg.cols,mImg.type());	

	if (iCur==0)//如果是第一副图
		mImg.copyTo(mCurImg);
	else
		cv::max(mImg,mCurImg,mCurImg);
	if (iCur==mCVParas->iUseImg-1)
		CalMask(mCurImg,iAreaPer,iScale);
}

void CCalImgValue::ReadExistCVMat()
{
	ReadCVMats.clear();
	int iSize[]={512,1024,2048};
	for (int i=0;i<3;i++)
	{
		CString strWrite;
		strWrite.Format(L".\\EpiResult\\CVAvgValue%d.xml",iSize[i]);
		std::string tempName=(LPCSTR)CStringA(strWrite);

		Mat mCVAvgImg=Mat(iSize[i],iSize[i],CV_32F);
		FileStorage fs(tempName, FileStorage::READ);
		Mat mat_vocabulary;
		fs["CVReBuildValue"] >> mCVAvgImg;

		/*FILE * fp=fopen((const char*)tempName.c_str(),"rb");
		if (!fp)
		continue;
		int iWSize=mCVAvgImg.rows*mCVAvgImg.cols*mCVAvgImg.channels()*sizeof(float);
		fread(mCVAvgImg.data,1,iWSize,fp);
		fclose(fp);*/
		ReadCVMats.push_back(mCVAvgImg);
	}
}

void CCalImgValue::CalMask(cv::Mat mImg,int iAreaPer,int iScale)
{
	double dMax=0,dMin=0;
	cv::Point minPt=Point(0,0),maxPt=Point(0,0);
	cv::minMaxLoc(mImg,&dMin,&dMax,&minPt,&maxPt);
	cv::Scalar mVmean,mVstd;
	cv::meanStdDev(mImg,mVmean,mVstd);
	int iTh=0;
	int width=mImg.cols;
	int height=mImg.rows;
	cv::Mat mBinaryImg=cv::Mat(mImg.rows,mImg.cols,CV_8U);
	cv::Mat mBinaryImgNew=cv::Mat(mImg.rows,mImg.cols,CV_8U);
	double mAreaMax=-1,mAreaMin=height*width,mAllArea=0;
	for (int i=8;i<10;i++)
	{
		iTh=dMax-mVmean[0]/i;
		cv::threshold(mImg,mBinaryImg,iTh,255,CV_THRESH_BINARY);
		mBinaryImg.copyTo(mBinaryImgNew);
		mAreaMax=-1,mAreaMin=height*width;
		mBolockScan.GetObjectArea(mBinaryImg,&mAreaMax,&mAreaMin,&mAllArea);//里面会更改掉二值图像
		if (mAreaMax<height*width/iAreaPer)
			break;
	}
	//还没有去掉大面积

	//然后进行统计、中值滤波
	cv::Mat mNewImg=cv::Mat(height,width,CV_8U);
	cv::bitwise_and(mBinaryImgNew,mImg,mNewImg);

	//cv::namedWindow("ProcessImg1");
	//cv::imshow("ProcessImg1",mImg);

	Scalar mV=cv::sum(mNewImg);
	float mAveValue=dMax-mVstd[0];//mV[0]/mAllArea;//mAreaMax;
	int iAveValue=mAveValue;

	//cv::Mat mCVMidV=cv::Mat(height,width, CV_8U,Scalar(iAveValue));	
	cv::Mat mCVMidV2=cv::Mat(height,width, CV_32F,Scalar(mAveValue));
	cv::Mat mCVCalMat=cv::Mat(height,width, CV_32F,Scalar(mAveValue));	

	for (int i=0;i<height;i++)
	{
		for (int j=0;j<width;j++)
		{
			float fV=mAveValue/mImg.at<byte>(i,j);
			//if (fV<1)
			//	fV=1;
			mCVMidV2.at<float>(i,j)=fV;
		}
	}

	//cv::divide(mCVMidV,mImg,mCVMidV2);  //会改变了mCVMidV2的数据类型，变成与前两个图像的数据类型一样， CV_32F的图像不能被divide函数使用！
	cv::medianBlur(mCVMidV2,mCVCalMat,iScale);//当iScale为3或者5时，图片的深度只能是CV_8U,，CV_16U，或者 CV_32F，对于其他大孔径尺寸只支持深度为CV_8U。
	//mCVCalMat.at<float>(550,100);

	//存储一下模板图像
	cv::Mat mSaveImg=cv::Mat(height,width,CV_8U,Scalar(255));
	cv::minMaxLoc(mCVCalMat,&dMin,&dMax,&minPt,&maxPt);
	for (int i=0;i<height;i++)
	{
		for (int j=0;j<width;j++)
		{
			float fV=mCVCalMat.at<float>(i,j)/dMax;//mAveValue/mImg.at<byte>(i,j);
			if (fV>1)
				fV=1;
			mSaveImg.at<byte>(i,j)=fV*255;
		}
	}
	//cv::flip(mSaveImg,mSaveImg,0);
	CString str;
	str.Format(L".\\EpiResult\\CVAvgImg%d.bmp",width);
	HSaveBmp(mSaveImg.data,width,height,8,str);


	//记录数据：
	CString strWrite;
	strWrite.Format(L".\\EpiResult\\CVAvgValue%d.xml",width);
	std::string tempName=(LPCSTR)CStringA(strWrite);
	FileStorage fs(tempName, FileStorage::WRITE);
	fs<<"CVReBuildValue"<<mCVCalMat;
	fs.release();

	//使用下面的文件流方式速度比较快，但是有时候的浮点数据不能还原！
	//strWrite.Format(L".\\EpiResult\\CVAvgValue%d.cvmat",width);
	//std::string tempName=(LPCSTR)CStringA(strWrite);
	//FILE * fp=fopen((const char*)tempName.c_str(),"w");
	//if (!fp)
	//	return;
	////int iWSize=mCVCalMat.rows*mCVCalMat.cols*mCVCalMat.channels()*sizeof(float);
	////fwrite(mCVCalMat.data,1,iWSize,fp);
	//int iWSize=mCVCalMat.rows*mCVCalMat.cols*mCVCalMat.channels();//*sizeof(float);
	//fwrite(mCVCalMat.data,sizeof(float),iWSize,fp);
	//fclose(fp);

	ReadExistCVMat();

	//直接计算试试
	
	////读取，重新计算试试
	//{
	//	Mat mCVAvgImg=Mat(mImg.rows,mImg.cols,CV_32F);
	//	FileStorage fs(tempName, FileStorage::READ);
	//	Mat mat_vocabulary;
	//	fs["CVReBuildValue"] >> mCVAvgImg;
	//	for (int i=0;i<height;i++)
	//	{
	//		for (int j=0;j<width;j++)
	//		{
	//			float fV=mCVAvgImg.at<float>(i,j);//mAveValue/mImg.at<byte>(i,j);
	//			fV=fV*mImg.at<byte>(i,j);
	//			if (fV>255)
	//				mImg.at<byte>(i,j)=255;
	//			else 
	//				mImg.at<byte>(i,j)=fV;
	//		}
	//	}
	//	cv::namedWindow("DstImg2");
	//	cv::imshow("DstImg2",mImg);
	//}

	//strValHeader=".\\EpiResult\\ImgCVValue.txt";
	//fp=fopen((const char*)strValHeader.c_str(),"w");
	//CString strWrite=L"";
	//strWrite.Format(L"最大值：%.5f，最小值：%.5f； 均值：%.5f，方差：%.5f\r\n",dMax,dMin,mVmean[0],mVstd[0]);
	//std::string tempName=(LPCSTR)CStringA(strWrite);
	//const char* mN=tempName.c_str();
	//fwrite(mN,1,strlen(mN),fp);
	//for (int i=0;i<height;i++)
	//{
	//	for (int j=0;j<width;j++)
	//	{
	//		float fV=mCVCalMat.at<float>(i,j);
	//		strWrite.Format(L"%.3f，",fV);
	//		if (j==width-1)
	//			strWrite += L"  \r\n下一行：";
	//		tempName=(LPCSTR)CStringA(strWrite);
	//		mN=tempName.c_str();
	//		fwrite(mN,1,strlen(mN),fp);
	//	}	
	//}
	//fclose(fp);
}

void CCalImgValue::UseMask(HVideoHeader* pHeader)
{
	int width=pHeader->Vwidth;
	int height=pHeader->Vheight;
	//cv::Mat mCVCalMat;
	bool bUse=false;
	int idex=0;
	for (int i=0;i<ReadCVMats.size();i++)
	{
		//mCVCalMat=ReadCVMats[i];
		if (ReadCVMats[i].rows!=height||ReadCVMats[i].cols!=width)
			continue;
		else
		{
			idex=i;
			bUse=true;
			break;
		}
	}
	if (!bUse)
	{
		return;
	}

	cv::Mat mCVCalMat=ReadCVMats[idex];

	int wBit=pHeader->VwBit/8;//这里图的位数
	cv::Mat mImg;
	if (wBit==3)
	{
		cv::Mat mOldImg=cv::Mat(height,width,CV_8UC3);
		memcpy(mOldImg.data,pHeader->Vbuffer,pHeader->Vsize);
		cvtColor(mOldImg,mImg,CV_RGB2GRAY);
		delete(pHeader->Vbuffer);
		pHeader->Vsize=width*height;
		pHeader->VwBit=8;
		pHeader->Vbuffer=(LPBYTE)malloc(pHeader->Vsize);
	}		
	else if(wBit==1)
	{
		mImg=cv::Mat(height,width,CV_8U);	
		memcpy(mImg.data,pHeader->Vbuffer,pHeader->Vsize);
	}
	//cv::namedWindow("PreImg");
	//cv::imshow("PreImg",mImg);
	for (int i=0;i<height;i++)
	{
		for (int j=0;j<width;j++)
		{
			float fV=mCVCalMat.at<float>(i,j);//mAveValue/mImg.at<byte>(i,j);
			fV=fV*mImg.at<byte>(i,j);
			if (fV>255)
				mImg.at<byte>(i,j)=255;
			else 
				mImg.at<byte>(i,j)=fV;
		}
	}
	//cv::namedWindow("After PreImg");
	//cv::imshow("After PreImg",mImg);
	if (mBKBinaryImg.rows==mImg.rows&&mBKBinaryImg.cols==mImg.cols)//如果有与背景图一样大小，则合并去除背景！
	{
		cv::bitwise_and(mBKBinaryImg,mImg,mImg);
	}
	memcpy(pHeader->Vbuffer,mImg.data,pHeader->Vsize);
}
