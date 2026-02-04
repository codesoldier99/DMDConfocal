#pragma once
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "HCoreProcess.h"
using namespace cv;

class CEasyFunction:public HEasyFunction
{
public:
	CEasyFunction(void);
	~CEasyFunction(void);

	bool OpenFile(CString strPath,HVideoHeader* mHeader,int iColor=-2,int iW=0,int iH=0);
	//iColor=-2的使用，就使用CV_LOAD_IMAGE_ANYCOLOR|CV_LOAD_IMAGE_ANYDEPTH
	//iColor=CV_LOAD_IMAGE_UNCHANGED;        -1	
	//iColor=CV_LOAD_IMAGE_GRAYSCALE;         0
	//iColor=CV_LOAD_IMAGE_COLOR;             1
	//iColor=CV_LOAD_IMAGE_ANYDEPTH;          2
	//iColor=CV_LOAD_IMAGE_ANYCOLOR;          4
	//由于要做标志位的组合，所以可以看到标识位是 0(0000) 1(0001) 2(0010) 4(0100)，没有3
	//读取图像原图用 CV_LOAD_IMAGE_ANYCOLOR|CV_LOAD_IMAGE_ANYDEPTH，而iColor=CV_LOAD_IMAGE_UNCHANGED没用了
	bool SavePic( HVideoHeader* pHeader,CString pName);

  virtual bool ShowImage(HVideoHeader* header, CString title, double scale);
};

