#pragma once
//#include "HConfocalCore.h"
#include "HOpenCVBase.h"
#include <vector>
using std::vector;

class CFindCell :public HCoreProcess
{
public:
	CFindCell(void);
	~CFindCell(void);
	virtual bool InitProcess(LPVOID p_param=0);//初始化库
	virtual bool UnInitProcess();
	//InputPara输入，为了获取某个图像质量评估值;OutPutPara输出返回结果，可能结构图或者数据
	virtual bool ProcessImg(void* InputPara,void* OutPutPara,HVideoHeader* pHeader);
	virtual void SavePic( HVideoHeader* pHeader,CString pName,int pType=0);
};

