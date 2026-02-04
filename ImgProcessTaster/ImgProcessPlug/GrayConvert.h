#pragma once
#include "HOpenCVBase.h"
#include "HCoreProcess.h"

class CGrayConvert:public HImgPreProcess
{
public:
	CGrayConvert(void);
	~CGrayConvert(void);
public:
	virtual bool InitProcess(LPVOID p_param=0);//初始化库
	virtual bool UnInitProcess() {return false;};
	//virtual bool PreProcessImg(HVideoHeader* pHeader);//处理图像，并且返回
	virtual bool PreProcessImg(HVideoHeader* pOriginHeader,HVideoHeader* pEndHeader);
	virtual void SavePic( HVideoHeader* pHeader,CString pName,int pType=0);
	virtual void SetDebug(bool bV);

	ProcessParas* mProcessV;
	bool bDebug;
	CRITICAL_SECTION m_crtSec;
};

