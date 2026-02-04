#include "StdAfx.h"
#include "ImgDAC.h"
#include "GraphUtils.h"

CImgDAC::CImgDAC(void)
{
}

CImgDAC::~CImgDAC(void)
{
}

bool CImgDAC::InitProcess(LPVOID p_param)
{
	return false;
}

bool CImgDAC::UnInitProcess()
{

	return true;
}

bool CImgDAC::ProcessImg(void* InputPara,void* OutPutPara,HVideoHeader* pHeader)
{
	DACAFPARAM mV=*(DACAFPARAM*)InputPara;
	double* dMoveV=(double*)OutPutPara;    //传出Z轴移动值

	int iW0 = mV.pCaptureVideo0->Vwidth;
	int iH0 = mV.pCaptureVideo0->Vheight;
	int iBit0 = mV.pCaptureVideo0->VwBit / 8;//这里图的位数
	int iW1 = mV.pCaptureVideo1->Vwidth;
	int iH1 = mV.pCaptureVideo1->Vheight;
	int iBit1 = mV.pCaptureVideo1->VwBit / 8;//这里图的位数
	/************************生成图像0和图像1的openCV对象************************/
	cv::Mat mImg0, mImg1;
	if (iBit0 == 3)
		mImg0 = cv::Mat(iH0, iW0, CV_8UC3, mV.pCaptureVideo0->Vbuffer);
	else if (iBit0 == 1)
		mImg0 = cv::Mat(iH0, iW0, CV_8U, mV.pCaptureVideo0->Vbuffer);

	if(iBit1 == 3)
		mImg1 = cv::Mat(iH1, iW1, CV_8UC3, mV.pCaptureVideo1->Vbuffer);
	else if (iBit1 == 1)
		mImg1 = cv::Mat(iH1, iW1, CV_8U, mV.pCaptureVideo1->Vbuffer);
	/************************旧的差动算法************************/
	_img_w = iW0 > iW1 ? iW1 : iW0;
	_img_h = iH0 > iH1 ? iH1 : iH0;
	_height_img.resize(_img_w * _img_h);
	unsigned char* a = mImg0.data;
	unsigned char* b = mImg1.data;
	double* h = _height_img.data();
	for (int i = 0; i < _img_h; i++)
	{
		for (int j = 0; j < _img_w; j++)
		{
			int p = i * _img_w + j;
			double zU = (a[p] - b[p]);// 255.0;
			double zD = (a[p] + b[p]);
			double z = zU / zD;
			if (a[p] + b[p] > 18)
			{
				if (fabs(mV.coeff_k) > 1e-5)
				{
					z = -(z + mV.coeff_b) / mV.coeff_k;
				}
				else
				{
					z = -(z + mV.coeff_b);
				}
			}
			else
				z = -(0.5 + mV.coeff_b) / mV.coeff_k;//-1
			h[p] = z;
		}
	}
	if (mV.bFilter)
	{
		medianFilter(h, h, 5, 5);
	}
	/************************得出Z轴移动距离************************/
	*dMoveV = 0;
	for (int i = 0; i < _img_h; i++)
	{
		for (int j = 0; j < _img_w; j++)
		{
			int p = i * _img_w + j;
			*dMoveV += h[p];
		}
	}
	*dMoveV /= _img_h* _img_w; //目前直接取高度的均值，传出去的单位是mm;
	return true;
}

void CImgDAC::SavePic( HVideoHeader* pHeader,CString pName,int pType)
{
	SaveImage(pHeader,pName,FLIP_VERTICAL);
}

void CImgDAC::medianFilter(double* corrupted, double* smooth, int width, int height)
{
	memcpy(smooth, corrupted, width*height * sizeof(unsigned char));
	for (int j = 1; j < height - 1; j++)
	{
		for (int i = 1; i < width - 1; i++)
		{
			int k = 0;
			double window[9];
			for (int jj = j - 1; jj < j + 2; ++jj)
				for (int ii = i - 1; ii < i + 2; ++ii)
					window[k++] = corrupted[jj * width + ii];
			//   Order elements (only half of them)  
			for (int m = 0; m < 5; ++m)
			{
				int min = m;
				for (int n = m + 1; n < 9; ++n)
					if (window[n] < window[min])
						min = n;
				//   Put found minimum element in its place  
				double temp = window[m];
				window[m] = window[min];
				window[min] = temp;
			}
			smooth[j*width + i] = window[4];
		}
	}
}

