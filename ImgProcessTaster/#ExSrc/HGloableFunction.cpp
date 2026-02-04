
#include "stdafx.h"
#include "HGloableFunction.h"

bool HLoadBmp( LPBYTE* pBuffer,int& IWidth, int& IHeight, int& wBitsPerPixel, LPCTSTR lpszFileName )
{
	BITMAPINFOHEADER bmpinfoHeader={0};
	BITMAPFILEHEADER bfh={0};
	HANDLE hFile=CreateFile(lpszFileName,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
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

bool HSaveBmp( LPBYTE pBuffer,int IWidth, int IHeight, int wBitsPerPixel, LPCTSTR lpszFileName )
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

void ChangeFormate( unsigned char* scr,unsigned char* dest )
{
	int m,i,j,k;
	int tt;

	for(m=0;m<768;m++)
		for(j=0;j<16;j++)
			for(i=0;i<8;i++)
			{
				tt = i;
				for(k=0;k<8;k++)
					dest[128*m + 8*j + i] = (dest[128*m + 8*j + i] << 1) | (scr[1024*m - j + 128*i + (127-16*k)] & 1);
			}
}


