


#include "stdafx.h"

#include "ImgProcess.h"

#pragma region AWB


/////////返回进行白平衡的区域(x0，y0，xn，yn)，这个是从下而上的扫描方式///////////
/////////m_Block24为图像句柄，请修改/////////
bool Cam_AutoAWB(LPBYTE pBuffer,long IWidth, long IHeight,double& iRed,double& iGreen,double& iBlue)
{   
	RECT rImage;
	rImage.left = 0;
	rImage.top = 0;
	rImage.right = IWidth;
	rImage.bottom = IHeight;

	// 等分
	int cntX = 8, cntY = 6;

	cntX = rImage.right / 10;
	cntY = rImage.bottom / 10;

	RECT *pR = new RECT[cntX * cntY];
	int index = 0;
	int w = rImage.right / cntX;
	int h = rImage.bottom / cntY;

	for(int row = 0; row < cntY; row++)
	{
		for(int col = 0; col < cntX; col++)
		{
			pR[index].left = col * w;
			pR[index].top = row * h;
			pR[index].right = pR[index].left + w -1;
			pR[index].bottom = pR[index].top + h -1;
			index++;
		}
	}

	//找最大值
	float mval = -1;
	float curval = -1;
	float* pValue = new float[cntX*cntY];
	int pos = 0;
	for(int i = 0; i < cntX * cntY; i++)
	{
		pValue[i]= GetSmooth(rImage.right,rImage.bottom,pR[i],(unsigned char*)pBuffer);
	}  
	pos = SmoothMax(pValue, cntX, cntY, 3, &mval);
	if(mval>5)
	{
		//前面在自动找白色的区域。。下面是算出平衡的值！！
		selectWhiteBlance(pBuffer,pR[pos],IWidth,IHeight,iRed,iGreen,iBlue);
		//*maxval = mval;//这个个最大值不要了？
		delete[]pR;
		delete [] pValue;
		return true;
	}  
	delete []pR;
	delete [] pValue;
	return false;
}

//////计算RGB各自的白平衡比例////////
//////////////其中(x1,y1)和(x2,y2)中，是指从上而下的扫描方式/////////////////////
void selectWhiteBlance(LPBYTE pBuffer,RECT pRect,int iWidth, int iHeight,double &kR,double &kG,double &kB)
{
	LPBYTE p_data;//原图数据区指针
	int x1=pRect.left;
	int x2=pRect.right;
	int y1=pRect.top;
	int y2=pRect.bottom;


	int wide,height;
	int m_PixelBytes=3;//3位的

	p_data = pBuffer;
	wide =iWidth;
	height=iHeight;

	int count=0;
	int index=0;
	double Ymean=0;//计算平均亮度
	double Rmean=0;
	double Gmean=0;
	double Bmean=0;//记录白色点集的平均RGB值

	int x,y;
	if (x1>x2)
	{
		x=x1;
		x1=x2;
		x2=x;
	}
	if (y1>y2)
	{
		y=y1;
		y1=y2;y2=y;    //调整方框位置参数
	}

	for (int j=height-1;j>=0;j--)
	{
		for (int i=0;i<wide*m_PixelBytes;i+=m_PixelBytes)
		{
			BYTE b=*(p_data + wide*m_PixelBytes*j+i);
			BYTE g=*(p_data + wide*m_PixelBytes*j+i+1);
			BYTE r=*(p_data + wide*m_PixelBytes*j+i+2);

			double Y=0.299*r+0.587*g+0.114*b;

			if (j>height-y2&&j<height-y1&&i>x1*m_PixelBytes
				&&i<x2*m_PixelBytes)/////height-y2,height-y1 切换扫描方向/////////
			{
				Rmean+=*(p_data + wide*m_PixelBytes*j+i+2);
				Gmean+=*(p_data + wide*m_PixelBytes*j+i+1); 
				Bmean+=*(p_data + wide*m_PixelBytes*j+i); 
				count++;
			}
		}
	}

	Rmean=Rmean/count;
	Gmean=Gmean/count;
	Bmean=Bmean/count;
	Ymean=Rmean;
	if (Ymean<Gmean)
		Ymean=Gmean;
	else if (Ymean<Bmean)
		Ymean=Bmean;

	kB=Ymean/Bmean;
	kG=Ymean/Gmean;
	kR=Ymean/Rmean;
}



int SmoothMax( float* pValue, int w, int h, int matrix, float* max ) 
{
	int size = w*h;  
	int edge = (matrix - 1)/2;
	*max = 0; 
	int pos = 0;
	for (int i = edge; i < h - edge; i++)
	{
		for(int j = edge; j < w - edge; j++)
		{
			int idx = w*i + j;
			float p = (/*pValue[idx - w - 1] +*/ pValue[idx - w] /*+ pValue[idx - w + 1]*/
			+ pValue[idx - 1] + 4*pValue[idx] + pValue[idx + 1]
			/*+ pValue[idx + w - 1]*/ + pValue[idx + w] /*+ pValue[idx + w + 1]*/);
			if(*max < p)
			{
				*max = p;
				pos = idx;
			}
		}
	}
	return pos;
}

float GetSmooth(int w,int h,const RECT& r,unsigned char *pData)
{
	int bytesperline = w * 3;
	int sw = r.right - r.left + 1;
	int sh = r.bottom - r.top + 1;
	unsigned char *start = pData + r.top * bytesperline + r.left * 3;
	unsigned char *pTmp = start;
	long n = sw * sh;
	//求平均值
	long x = 0;   

	for(int row = 0; row < sh; row++)
	{
		for(int col = 0; col < sw; col++)
		{
			x += 11 * (*pTmp) + 59 * (*(pTmp + 1)) + 30 * (*(pTmp + 2));       
			pTmp += 3;
		}
		pTmp = start + bytesperline;
	} 
	float ave = (float)x / 100.0 / n; 
	return ave;
}




#pragma endregion AWB