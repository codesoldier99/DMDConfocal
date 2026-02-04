#include "StdAfx.h"
#include "HOpenCVBase.h"

void GetAbsGradient(Mat& src, Mat& des)
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
}

bool SaveImage( HVideoHeader* pHeader,CString pName,FLIP_TYPE pType )
{
	int width=pHeader->Vwidth;
	int height=pHeader->Vheight;
	int index=int(pHeader->VcaptureTime);//这里暂用capture time代替编号
	int wBit=pHeader->VwBit/8;//这里图的位数
	cv::Mat mImg;
	if (wBit==3)
		//mImg=cv::Mat(height,width,CV_8UC3,pHeader->Vbuffer);	
		mImg=cv::Mat(height,width,CV_8UC3);
	else if(wBit==1)
		//mImg=cv::Mat(height,width,CV_8U,pHeader->Vbuffer);
		mImg=cv::Mat(height,width,CV_8U);
	else 
		return false;
	memcpy(mImg.data,pHeader->Vbuffer,pHeader->Vsize);
	switch(pType)
	{
		case FLIP_NONE:
			break;
		case FLIP_VERTICAL:
			cv::flip(mImg,mImg,0);
			break;
		case FLIP_HORIZONTAL:
			cv::flip(mImg,mImg,1);
			break;
		case FLIP_BOTH:
			cv::flip(mImg,mImg,-1);
			break;
		default:
			break;
	}
	std::string tempName=(LPCSTR)CStringA(pName);
	const char* mN=tempName.c_str();
	imwrite(mN,mImg);
	return true;
}

//获取时间字符串
CString HGetTimeString()
{
	struct tm *local;
	time_t t;
	t=time(NULL);
	local=localtime(&t);
	CString str;
	str.Format(_T("%d%02d%02d\\%d-%d-%d"),local->tm_year+1900,local->tm_mon+1,local->tm_mday,
		local->tm_hour,local->tm_min,local->tm_sec);
	//str.Format(_T("%d%02d%02d\\%dh\\%dm%ds"),local->tm_year+1900,local->tm_mon+1,local->tm_mday,
	//	local->tm_hour,local->tm_min,local->tm_sec);
	return str;
}

//int otsu ( IplImage*img, int x0, int y0, int dx, int dy, int vvv)
//	//IplImage*img
//{
//	uchar* np; // 图像指针
//	int thresholdValue=1; // 阈值
//	int ihist[256]; // 图像直方图，256个点
//	int step = img->widthStep/sizeof(uchar);
//	uchar *data = (uchar*)img->imageData;//源数据
//	int i, j, k; // various counters
//	int n, n1, n2, gmin, gmax;
//	double m1, m2, sum, csum, fmax, sb;
//	// 对直方图置零
//	memset(ihist, 0, sizeof(ihist));
//	gmin=255; gmax=0;
//	// 生成直方图
//	for (i = y0 +1; i < y0 + dy -1; i++) 
//	{
//		np = &data[i*step+x0+1];
//		for (j = x0 +1; j < x0 + dx -1; j++)
//		{
//			ihist[*np]++;
//			if(*np > gmax) gmax=*np;
//			if(*np < gmin) gmin=*np;
//			np++; /* next pixel */
//		}
//	}
//	// set up everything
//	sum = csum =0.0;
//	n =0;
//
//	for (k =0; k <=255; k++) 
//	{
//		sum += (double) k * (double) ihist[k]; /* x*f(x) 质量矩*/
//		n += ihist[k]; /* f(x) 质量 */
//	}
//
//	if (!n) 
//	{
//		// if n has no value, there is problems...
//		fprintf (stderr, "NOT NORMAL thresholdValue = 160\n");
//		return (160);
//	}
//
//	// do the otsu global thresholding method
//	fmax =-1.0;
//	n1 =0;
//	for (k =0; k <255; k++)
//	{
//		n1 += ihist[k];
//		if (!n1) 
//		{ 
//			continue; 
//		}
//		n2 = n - n1;
//		if (n2 ==0)
//		{ 
//			break; 
//		}
//		csum += (double) k *ihist[k];
//		m1 = csum / n1;
//		m2 = (sum - csum) / n2;
//		sb = (double) n1 *(double) n2 *(m1 - m2) * (m1 - m2);
//		/* bbg: note: can be optimized. */
//		if (sb > fmax) 
//		{
//			fmax = sb;
//			thresholdValue = k;
//		}
//	}
//
//	// at this point we have our thresholding value
//
//	// debug code to display thresholding values
//	if ( vvv &1 )
//		fprintf(stderr,"# OTSU: thresholdValue = %d gmin=%d gmax=%d\n",
//		thresholdValue, gmin, gmax);
//
//	return(thresholdValue);
//}

/*======================================================================*/
/* OTSU global thresholding routine */
/*======================================================================*/
int otsu2 (LPBYTE Vbuffer,int iWidthStep,int iWidth,int iHeght)//lImage *image)
{
	int w = iWidth;
	int h = iHeght;
	unsigned char*np; // 图像指针
	unsigned char pixel;
	int thresholdValue=1; // 阈值
	int ihist[256]; // 图像直方图，256个点
	int i, j, k; // various counters
	int n, n1, n2, gmin, gmax;
	double m1, m2, sum, csum, fmax, sb;
	// 对直方图置零...
	memset(ihist, 0, sizeof(ihist));
	gmin=255; gmax=0;
	// 生成直方图
	for (i =0; i < h; i++) 
	{
		np = (unsigned char*)(Vbuffer + iWidthStep*i);
		for (j =0; j < w; j++) 
		{
			pixel = np[j];
			ihist[ pixel]++;
			if(pixel > gmax) gmax= pixel;
			if(pixel < gmin) gmin= pixel;
		}
	}
	// set up everything
	sum = csum =0.0;
	n =0;
	for (k =0; k <=255; k++) 
	{
		sum += (double)k * (double)ihist[k]; /* x*f(x) 质量矩*/
		n += ihist[k]; /* f(x) 质量 */
	}

	if (!n) 
	{
		thresholdValue =160;
		goto L;
	}
	// do the otsu global thresholding method
	fmax =-1.0;
	n1 =0;
	for (k =0; k <255; k++) 
	{
		n1 += ihist[k];
		if (!n1) { continue; }
		n2 = n - n1;
		if (n2 ==0) { break; }
		csum += (double)k *ihist[k];
		m1 = csum / n1;
		m2 = (sum - csum) / n2;
		sb = (double)n1 * (double)n2 *(m1 - m2) * (m1 - m2);
		/* bbg: note: can be optimized. */
		if (sb > fmax)
		{
			fmax = sb;
			thresholdValue = k;
		}
	}
L:
	return(thresholdValue);
}


void HERODE_XSC(Mat& img,int WS)//要输出二值图像
{
	Mat imgDist=Mat(img.rows,img.cols,CV_32F);
	cv::distanceTransform(img,imgDist,CV_DIST_L2,3);
	cv::threshold(imgDist,img,WS,255,CV_THRESH_BINARY);
	img.convertTo(img,CV_8U);
}
//
//void SegmentProcess::DILATE_XSC(IplImage* img,int WS)//要输出二值图像
//{
//	IplImage* imgDist=cvCreateImage(cvGetSize(img),IPL_DEPTH_32F,1);
//	IplImage* img2=cvCreateImage(cvGetSize(img),IPL_DEPTH_8U,1);
//	cvCopyImage(img,img2);
//
//	cvThreshold(img2,img2,254,255,CV_THRESH_BINARY_INV);//先取反，再变换
//
//	cvDistTransform(img2,imgDist,CV_DIST_L2);//马氏距离变换
//	cvThreshold(imgDist,img2,WS,255,CV_THRESH_BINARY);//阈值分割，留下大于该距离的范围
//
//	cvThreshold(img2,img,254,255,CV_THRESH_BINARY_INV);//再取反
//	cvReleaseImage(&imgDist);
//	cvReleaseImage(&img2);
//
//}



