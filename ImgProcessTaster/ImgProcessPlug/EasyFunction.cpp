#include "StdAfx.h"
#include "EasyFunction.h"
#include "HOpenCVBase.h"


CEasyFunction::CEasyFunction(void)
{
}


CEasyFunction::~CEasyFunction(void)
{
}

bool CEasyFunction::OpenFile(CString strPath,HVideoHeader* mHeader,int iColor,int iW,int iH)
{
	//if (mHeader->Vbuffer)
	//	delete(mHeader->Vbuffer);
	std::string tempName=(LPCSTR)CStringA(strPath);
	const char *tmp=tempName.c_str();
	if (iColor==-2)
		iColor=CV_LOAD_IMAGE_ANYCOLOR|CV_LOAD_IMAGE_ANYDEPTH;
	Mat mImg = cvLoadImage(tmp,iColor);
	if (!mImg.data)
		return false;
	if (iW!=0&&iH!=0)
	{
		Size mSize=cv::Size(iW,iH);
		cv::resize(mImg,mImg,mSize);
	}
	if (mHeader->Vbuffer)
	{
		delete  mHeader->Vbuffer;
		mHeader->Vbuffer=0; 
	}
		
	mHeader->Vwidth=mImg.cols;
	mHeader->Vheight=mImg.rows;
	mHeader->VwBit=mImg.channels()*8;
	mHeader->Vsize=mHeader->Vwidth*mHeader->Vheight*mImg.channels();
	mHeader->Vbuffer=(LPBYTE)malloc(mHeader->Vsize);
	memcpy(mHeader->Vbuffer,mImg.data,mHeader->Vsize);
	return true;
}

bool CEasyFunction::SavePic( HVideoHeader* pHeader,CString pName)
{
	return SaveImage( pHeader,pName,FLIP_VERTICAL);
}

bool CEasyFunction::ShowImage(HVideoHeader* header, CString title, double scale)
{
  if (!header)
  {
    return false;
  }
  Mat img(header->Vheight, header->Vwidth, header->VwBit == 24 ? CV_8UC3 : CV_8UC1, header->Vbuffer);
  USES_CONVERSION;
  if (scale != 0)
  {
    Mat img2;
    resize(img, img2, Size((int)(img.cols * scale), (int)(img.rows * scale)));
    imshow(W2A(title), img2);
  }
  else
  {
    imshow(W2A(title), img);
  }
  return true;
}