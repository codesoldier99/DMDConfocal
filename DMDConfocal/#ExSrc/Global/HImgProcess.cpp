// ImgProcess.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "HImgProcess.h"
#include <stdlib.h>
#include <math.h>


/*

	CFileDialog dlgFile(TRUE,NULL,NULL,OFN_HIDEREADONLY|OFN_LONGNAMES,
		_T("bmp位图|*.bmp|所有文件|*.*|"),NULL);
	if(dlgFile.DoModal())
	{
		//CString FileName=dlgFile.GetPathName();
		CString FilePath=dlgFile.GetFolderPath();
 
			LoadBmp(pImgBuffer,FilePath+"\\"+FileNames[i]);
*/


bool SaveBmp( LPBYTE pBuffer,long IWidth, long IHeight, int wBitsPerPixel, LPCTSTR lpszFileName )
{
	BITMAPINFOHEADER bmpinfoHeader={0};//没有考虑4的倍数哦。。
	bmpinfoHeader.biSize=sizeof(BITMAPINFOHEADER);
	bmpinfoHeader.biBitCount=wBitsPerPixel;
	bmpinfoHeader.biClrImportant=0;//use all colors
	bmpinfoHeader.biClrUsed=0;//use an many colors according to bits per pixel
	bmpinfoHeader.biCompression=BI_RGB;
	bmpinfoHeader.biHeight=IHeight;
	bmpinfoHeader.biWidth=IWidth;
	bmpinfoHeader.biPlanes=1;//使用默认调色
	bmpinfoHeader.biSizeImage=IWidth*IHeight*(wBitsPerPixel/8);

	BITMAPFILEHEADER bfh={0};
	bfh.bfType=0x4D42;//"BM"
	bfh.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);//图像数据偏移字节
	bfh.bfSize=bfh.bfOffBits+bmpinfoHeader.biSizeImage;//文件大小 

	RGBQUAD *pRgb;//调色板
	int	sizeQuad=sizeof(RGBQUAD)*256;
	if(wBitsPerPixel==8)//加调色板
	{
		pRgb=(RGBQUAD*)malloc(sizeQuad);
		for (int i=0;i<256;i++)
		{
			pRgb[i].rgbBlue=i;
			pRgb[i].rgbGreen=i;
			pRgb[i].rgbRed=i;
		}
		bfh.bfOffBits+=sizeQuad;
		bfh.bfSize+=sizeQuad;
	}

	HANDLE hFile=CreateFile(lpszFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (!hFile)
	{
		return FALSE;
	}

	DWORD dwWritten=0;
	WriteFile(hFile,&bfh,sizeof(bfh),&dwWritten,NULL);
	WriteFile(hFile,&bmpinfoHeader,sizeof(bmpinfoHeader),&dwWritten,NULL);
	if(wBitsPerPixel==8)//加调色板
	{
		WriteFile(hFile,pRgb,sizeQuad,&dwWritten,NULL);
		delete(pRgb);
	}
	WriteFile(hFile,pBuffer,bmpinfoHeader.biSizeImage,&dwWritten,NULL);

	CloseHandle(hFile);
	return TRUE;
}

 bool LoadBmp( LPBYTE* pBuffer,long& IWidth, long& IHeight, int& wBitsPerPixel, LPCTSTR lpszFileName )
 {
 	BITMAPINFOHEADER bmpinfoHeader={0};
 	BITMAPFILEHEADER bfh={0};
 	HANDLE hFile=CreateFile(lpszFileName,GENERIC_READ,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
 	if (!hFile)
 	{
 		return FALSE;
 	}
 	DWORD dwRead=0;
 	ReadFile(hFile,&bfh,sizeof(BITMAPFILEHEADER),&dwRead,NULL);
 
 	if (bfh.bfType==0x4d42)//bmp
 	{
 		ReadFile(hFile,&bmpinfoHeader,sizeof(BITMAPINFOHEADER),&dwRead,NULL);
 		IWidth=bmpinfoHeader.biWidth;
 		IHeight=bmpinfoHeader.biHeight;
 		wBitsPerPixel=bmpinfoHeader.biBitCount;
 
 		if ((IWidth%4)!=0) 
 		{
 			CloseHandle(hFile);
 			return FALSE;//暂不支持不是4的倍数
 		}
 		
 
 		//DWORD tsize=bmpinfoHeader.biSizeImage;//这个改改吧，常出问题
 		DWORD tsize=bmpinfoHeader.biBitCount/8*bmpinfoHeader.biWidth*bmpinfoHeader.biHeight;
 		
 		DWORD tsizeh=bfh.bfOffBits;
 		if (tsizeh>54)
 		{
 			RGBQUAD *pRgb;//调色板
 			tsizeh-=54;
 			pRgb=(RGBQUAD*)malloc(tsizeh);
 			ReadFile(hFile,pRgb,tsizeh,&dwRead,NULL);
 			delete(pRgb);
 		}
 		if (*pBuffer)
 		{
 			delete(*pBuffer);
 		}
 		*pBuffer=(LPBYTE)malloc(tsize);
 		OVERLAPPED tOver;
 		tOver.Offset=bfh.bfOffBits;
 		ReadFile(hFile,*pBuffer,tsize,&dwRead,NULL);//&tOver);
 	}
 	else
 	{
 		CloseHandle(hFile);
 		return FALSE;
 	}
 
 	CloseHandle(hFile);
 	return TRUE;
 }

 void Img8_Thresh2( LPBYTE pBuffer,long IWidth,long IHeight,int Thresh )
 {
	 long PixelCount=IWidth*IHeight;
	 int temp=Thresh;

	 LPBYTE Index=pBuffer;

	 for (long i=0;i<PixelCount;i++)
	 {
		 if(*Index<=temp)
			 *Index=0;
		 else
			 *Index=0xFF;

		 Index++;
	 }

 }

 void Img8_And(LPBYTE pDest,LPBYTE pTemplate,long IWidth,long IHeight)
 {
	 long PixelCount=IWidth*IHeight;


	 LPBYTE Index_Scr=pTemplate;
	 LPBYTE Index_Dest=pDest;



	 for (long i=0;i<PixelCount;i++)
	 {
		 if(*Index_Scr==0)
		 {
			 *Index_Dest=0;
		 }

		 Index_Scr++;
		 Index_Dest++;
	 }
 }

 void Img8_Add(LPBYTE pDest,LPBYTE pScr,long IWidth,long IHeight,int Thresh)
 {
	 long PixelCount=IWidth*IHeight;
	 int temp=Thresh;
	 LPBYTE Index_Scr=pScr;
	 LPBYTE Index_Dest=pDest;
	 for (long i=0;i<PixelCount;i++)
	 {

		 if(*Index_Scr>*Index_Dest)
			 *Index_Dest=*Index_Scr;

		 Index_Scr++;
		 Index_Dest++;
	 }
 }

 void Img8_Max(LPBYTE pDest,LPBYTE pScr,long IWidth,long IHeight,int Thresh)
 {
	 long PixelCount=IWidth*IHeight;
	 LPBYTE Index_Scr=pScr;
	 LPBYTE Index_Dest=pDest;
	 if (Thresh>0)
	 {
		 int iScr,iDest;
		 for (long i=0;i<PixelCount;i++)
		 {
			 iScr=*Index_Scr-Thresh;
			 iScr=iScr>0?iScr:0;
			 iDest=*Index_Dest-Thresh;
			 iDest=iDest>0?iDest:0;
			 if(iScr>iDest)
				 *Index_Dest=iScr;
			 Index_Scr++;
			 Index_Dest++;
		 }
	 }
	 else
	 {
		 for (long i=0;i<PixelCount;i++)
		 {

			 if(*Index_Scr>*Index_Dest)
				 *Index_Dest=*Index_Scr;

			 Index_Scr++;
			 Index_Dest++;
		 }
	 }
	
 }
 
  void Img24_Sharp( LPBYTE pSource,long IWidth,long IHeight )
 {
 	int templates[25]={-1,-4,-7,-4,-1,
 		-4,-16,-26,-16,-4,
 		-7,-26,505,-26,-7,
 		-4,-16,-26,-16,-4,
 		-1,-4,-7,-4,-4};
 	LPBYTE dst=(LPBYTE)malloc(IWidth*IHeight);
 	memset(dst,0,IWidth*IHeight);
 
 	for (int j=2;j<IHeight-2;j++)
 	{
 		for(int i=2;i<IWidth-2;i++)
 		{
 			int sum=0;
 			int index=0;
 			for(int m=j-2;m<j+3;m++)
 			{
 				for (int n=i-2;n<i+3;n++)
 				{
 					sum+=pSource[m*IWidth+n]*templates[index++];
 				}
 			}
 			sum/=273;
 			if (sum>255)
 				sum=255;
 			if(sum<0)
 				sum=0;
 		}
 	}	
 
 	delete dst;
 }
 
  void RGB2Gray( LPBYTE pBuffer,long IWidth,long IHeight )
 {
 	long PixelCount=IWidth*IHeight;
 	PixelRgb * pPixel=(PixelRgb*)pBuffer;
 
 	for (long i=0;i<PixelCount;i++)
 	{
 		pPixel->Red=(int)((299*(long)pPixel->Red+587*(long)pPixel->Green+114*(long)pPixel->Blue)/1000);
 		pPixel->Blue=pPixel->Red;
 		pPixel->Green=pPixel->Red;
 		pPixel++;
 	}
 }
 
  void RGB2Gray8( LPBYTE& pBuffer,long IWidth,long IHeight )
 {
 	long PixelCount=IWidth*IHeight;
 	PixelRgb * pPixel=(PixelRgb*)pBuffer;
 
 	LPBYTE pTemp=(LPBYTE)malloc(PixelCount);
 
 	for (long i=0;i<PixelCount;i++)
 	{
 		pTemp[i]=(int)((299*(long)pPixel->Red+587*(long)pPixel->Green+114*(long)pPixel->Blue)/1000);
 		pPixel++;
 	}
 
 	delete(pBuffer);
 
 	pBuffer=(LPBYTE)malloc(PixelCount);
 	memcpy(pBuffer,pTemp,PixelCount);
 	delete(pTemp);
 }
 
  void Img24_Sobel( LPBYTE pSource,long IWidth,long IHeight,float cof )
 {
 	int sobel_x[9]={-1,-1,-1,
 					 0, 0, 0,
 					 1, 1, 1};
 	int sobel_y[9]={-1, 0, 1,
 					-1, 0, 1,
 					-1, 0, 1};
 	int sobel_vx[9]={-1,-1, 0,
 					 -1, 0, 1,
 					  0, 1, 1};//45度
 	int sobel_vy[9]={ 0, 1, 1,
 					 -1, 0, 1,
 					 -1,-1, 0};//135度
 	LPBYTE pDest=(LPBYTE)malloc(IWidth*IHeight*3);
 	int Arry[9];
 	float gx,gy,gvx,gvy;
 	PixelRgb* Line1=(PixelRgb*)pSource;
 	PixelRgb* Line2=(PixelRgb*)pSource;
 	PixelRgb* Line3=(PixelRgb*)pSource;
 	
 	PixelRgb* Dest=(PixelRgb*)pDest;//目标
 
 	Line2+=IWidth;
 	Line3+=2*IWidth;
 
 	memset(pDest,0,IWidth*3);//首行
 	Dest+=IWidth;
 
 	int icount=IHeight-1;
 	int jcount=IWidth-1;
 	for (int i=1;i<icount;i++)
 	{
 		Line1++;
 		Line2++;
 		Line3++;
 		Dest[0].Red=0;
 		Dest[0].Green=0;
 		Dest[0].Blue=0;
 		Dest++;
 		for(int j=1;j<jcount;j++)
 		{
 			gy=0;gx=0;gvx=0,gvy=0;
 
 
 			Arry[0]=Line1[0].Red;
 			Arry[1]=Line1[1].Red;
 			Arry[2]=Line1[2].Red;
 
 			Arry[3]=Line2[0].Red;
 			Arry[4]=Line2[1].Red;
 			Arry[5]=Line2[2].Red;
 
 			Arry[6]=Line3[0].Red;
 			Arry[7]=Line3[1].Red;
 			Arry[8]=Line3[2].Red;
 
 			for(int k=0;k<9;k++)
 			{
 				gy+=Arry[k]*sobel_y[k];
 				gx+=Arry[k]*sobel_x[k];
 				//gvx+=Arry[k]*sobel_vx[k];
 				//gvy+=Arry[k]*sobel_vy[k];
 			}
 			//系数 
 			gx*=cof;
 			//gvx*=cof;
 			gy*=cof;
 			//gvy*=cof;
 			
 			//绝对
  			gx=(float)abs(gx);
  			//gvx=(float)abs(gvx);
  			gy=(float)abs(gy);
  			//gvy=(float)abs(gvy);
  			if(gx>255) gx=255;
 			if(gvx>255) gvx=255;
 			if(gy>255) gy=255;
 			if(gvy>255) gvy=255;
 // 			if(gx<0) gx=0;
 // 			if(gy<0) gy=0;
 // 			if(gvx<0) gvx=0;
 // 			if(gvy<0) gvy=0;
 
 			gx+=gy;//相加
 			//gx+=gvx;
 			//gx+=gvy;
 
 			Dest[0].Red=gx>255?255:gx;//取值
 			//if(Dest[0].Red>255) Dest[0].Red=255;
 			//if(Dest[0].Red<0) Dest[0].Red=0;
 
 			Dest[0].Green=Dest[0].Red;
 			Dest[0].Blue=Dest[0].Red;
 
 
 			Line1++;
 			Line2++;
 			Line3++;
 			Dest++;
 		}
 
 		Line1++;
 		Line2++;
 		Line3++;
 		Dest[0].Red=0;
 		Dest[0].Green=0;
 		Dest[0].Blue=0;
 		Dest++;
 	}
 	memset(Dest,0,IWidth*3);//末行
 	memcpy(pSource,pDest,IHeight*IWidth*3);
 	delete(pDest);
 }
 

