#pragma once

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "HCoreProcess.h"
#include "RenderChain.h"
using namespace cv;

#ifndef BaseProcess_H
#define BaseProcess_H

void GetAbsGradient(Mat& src, Mat& des);
bool SaveImage( HVideoHeader* pHeader,CString pName,FLIP_TYPE pType );
CString HGetTimeString();
int otsu2 (LPBYTE Vbuffer,int iWidthStep,int iWidth,int iHeght);

void HERODE_XSC(Mat& img,int WS);
#endif
