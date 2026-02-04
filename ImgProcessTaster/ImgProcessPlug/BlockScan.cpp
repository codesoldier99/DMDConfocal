#include "StdAfx.h"
#include "BlockScan.h"


CBlockScan::CBlockScan(void)
{
	bDebug=false;
	InitializeCriticalSection(&m_crtSec);
}

CBlockScan::~CBlockScan(void)
{
}

bool CBlockScan::InitProcess(LPVOID p_param)
{
	mProcessV=(ProcessParas*)p_param;
	if (mProcessV)
	{
		return true;
	}

	return false;
}

long CBlockScan::ProcessImg(HVideoHeader* pOriginHeader,HVideoHeader* pEndHeader,float fPiexlSize,void* OutPutPara)
{
	EnterCriticalSection(&m_crtSec);
	int iTh=mProcessV->iParas[0];
	int iMaxArea=mProcessV->iParas[1];
	int iMimArea=mProcessV->iParas[2];
	bool bInvert=mProcessV->bParas[0];
	LeaveCriticalSection(&m_crtSec);

	if (iTh<=0)
		iTh=otsu2(pOriginHeader->Vbuffer,pOriginHeader->Vwidth*pOriginHeader->VwBit/8,
				pOriginHeader->Vwidth,pOriginHeader->Vheight);

	int wBit=pOriginHeader->VwBit/8;
	if (wBit!=1)
		return -1;
	int width=pOriginHeader->Vwidth;
	int height=pOriginHeader->Vheight;	
	cv::Mat mOldImg=cv::Mat(height,width,CV_8U,pOriginHeader->Vbuffer);
	cv::Mat mBinaryImg=cv::Mat(height,width,CV_8U);

	cv::Mat mNewImg;
	if (!pEndHeader->Vbuffer)
	{
		pEndHeader->VwBit=pOriginHeader->VwBit;
		pEndHeader->Vheight=pOriginHeader->Vheight;
		pEndHeader->Vwidth=pOriginHeader->Vwidth;
		pEndHeader->Vsize=pOriginHeader->Vsize;
		pEndHeader->Vbuffer=(LPBYTE)malloc(pEndHeader->Vsize);
		pEndHeader->fPixelSize=pOriginHeader->fPixelSize;
	}
	if (pEndHeader->VwBit==24)
		mNewImg=cv::Mat(height,width,CV_8UC3,pEndHeader->Vbuffer);
	else
		mNewImg=cv::Mat(height,width,CV_8U,pEndHeader->Vbuffer);

	if (bDebug)
	{
		cv::namedWindow("Before ProcessImg");
		cv::imshow("Before ProcessImg",mOldImg);
	}

	if (bInvert)
		cv::threshold(mOldImg,mBinaryImg,iTh,255,CV_THRESH_BINARY_INV);
	else
		cv::threshold(mOldImg,mBinaryImg,iTh,255,CV_THRESH_BINARY);

	if (bDebug)
	{
		cv::namedWindow("ProcessImg");
		cv::imshow("ProcessImg",mBinaryImg);
	}
	DrawBinaryImg(mBinaryImg,mNewImg);

	if (bDebug)
	{
		cv::namedWindow("After ProcessImg");
		cv::imshow("After ProcessImg",mNewImg);
	}
	if (iMaxArea>-1&&iMimArea>-1)
	{
		double mMax=-1,mMin=100000;
		GetObjectArea(mBinaryImg,&mMax,&mMin);
		mMax=mMax*pOriginHeader->fPixelSize;
		mMin=mMin*pOriginHeader->fPixelSize;

		if (mMax>iMimArea&&mMax<iMaxArea)
			return 33;
		else if (mMax>iMimArea)
			return 31;
		else if (mMax<iMaxArea)
			return 32;
		return -1;
	}	
	return 33;
}

//绘制轮廓
void CBlockScan::DrawBinaryImg(cv::Mat mBinaryImg,Mat mImg)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	cv::findContours(mBinaryImg,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE,cv::Point(0,0));
	/// Get the moments
	vector<Moments> mu(contours.size() );
	for( int i = 0; i < contours.size(); i++ )
	{ 
		mu[i] = moments( contours[i], false ); 
	}
	/// Get the mass centers:
	vector<Point2f> mc(contours.size());
	for( int i = 0; i < contours.size(); i++ )
	{ 
		mc[i] = Point2f(mu[i].m10/mu[i].m00,mu[i].m01/mu[i].m00); 
		TRACE(L"Contour[%d] 's Center:%.2f,%.2f\n",i,mc[i].x,mc[i].y);
	}
	RNG rng(12345);
	for (int i=0;i<contours.size();i++)
	{
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		cv::drawContours( mImg, contours, i, color, 2, 8, hierarchy, 0, Point() );
		circle( mImg, mc[i], 4, color, -1, 8, 0 );

		CString str;
		str.Format(L"Contour[%d] - Area (M_00) = %.2f - Area OpenCV: %.2f - Length: %.2f\n",i, mu[i].m00,contourArea(contours[i]),arcLength(contours[i],true));
		TRACE(str);
	}
	//cv::drawContours()
}

void CBlockScan::GetObjectArea(cv::Mat mBinaryImg,double *MaxArea,double *MinArea,double *dAllArea)
{ 	
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	cv::findContours(mBinaryImg,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE,cv::Point(0,0));
	double mV=0;
	for (int i=0;i<contours.size();i++)
	{
		double area=contourArea(contours[i]);
		if (area==0)
			continue;
		if (*MaxArea<area)
			*MaxArea=area;
		if (*MinArea>area)
			*MinArea=area;
		mV+=area;
	}
	if (dAllArea)
		*dAllArea=mV;
}

//int SegmentProcess::DeleteSmallObject(IplImage *src,float AL1,IplImage *dst)
//{ 	 
//	int hnum=0;//定义连通区域数
//	int hnum1=0;//定义连通区域数
//	int Sum=0;//输入图像中物体的个数
//	CvMemStorage *stor = cvCreateMemStorage(0);
//	CvSeq * cont = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvPoint), stor);
//	int Num = cvFindContours( src, stor, &cont, //存储轮廓
//		sizeof(CvContour), //序列尺寸
//		CV_RETR_CCOMP,//CV_RETR_EXTERNAL：只检索最外面的轮廓；CV_RETR_LIST：检索所有的轮廓，并将其放入list中；
//		//CV_RETR_CCOMP：检索所有的轮廓，并将他们组织为两层：顶层是各部分的外部边界，第二层是空洞的边界；
//		//CV_RETR_TREE：检索所有的轮廓，并重构嵌套轮廓的整个层次。
//		CV_CHAIN_APPROX_SIMPLE  , 
//		cvPoint(0,0)//偏移量，用于移动所有轮廓点。当轮廓是从图像的ROI提取的，并且需要在整个图像中分析时，这个参数将很有用。
//		);//定义目标对象数量
//	for(;cont;cont = cont->h_next)
//	{
//		double area= fabs(cvContourArea( cont,CV_WHOLE_SEQ)); //获取当前轮廓面积
//		if (area<AL1)
//		{
//			//cvDrawContours(img,contours,cvScalarAll(50),cvScalarAll(50),100);
//			//cvDrawContours( dst, cont, CV_RGB(255, 255, 0)//外轮廓的颜色。
//			// , CV_RGB(0, 255, 255)//内轮廓的颜色。
//			// ,0,//画轮廓的最大层数。如果是0，只绘制contour；
//			// //如果是1，将绘制contour后和contour同层的所有轮廓；
//			// //如果是2，绘制contour后所有同层和低一层的轮廓，以此类推；
//			// //如果值是负值，则函数并不绘制contour后的轮廓，但是将画出其子轮廓，一直到abs(max_level) - 1层。
//			// CV_FILLED//绘制轮廓线的宽度。如果为负值（例如，等于CV_FILLED），则contour内部将被绘制。
//			// ); //绘制白色CV_RGB(255, 255, 255)轮廓
//			//hnum1++;
//		}
//		else
//		{
//			cvDrawContours( dst, cont, CV_RGB(255, 255, 255), CV_RGB(0, 0, 255),0,CV_FILLED,8); //绘制黑色CV_RGB(0, 0, 0)轮廓
//			//cvDrawContours( dst, cont, CV_RGB(255, 255, 255), CV_RGB(0, 0, 255),10,2,8); //绘制黑色CV_RGB(0, 0, 0)轮廓
//			hnum++;
//		}
//	}
//	Sum=hnum+hnum1;
//	cvReleaseMemStorage(&stor);
//	//cvClearSeq(cont);
//	return hnum;//返回连通区域数
//}

