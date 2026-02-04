#pragma once
//#include "HConfocalCore.h"
#include "HOpenCVBase.h"
#include <vector>
using std::vector;

class CImgDAC:public HCoreProcess
{
public:
	CImgDAC(void);
	~CImgDAC(void);
	virtual bool InitProcess(LPVOID p_param=0);//初始化库
	virtual bool UnInitProcess();
	//InputPara输入焦前焦后图以及差动参数;OutPutPara输出Z轴调整值;pHeader传入空值
	virtual bool ProcessImg(void* InputPara,void* OutPutPara,HVideoHeader* pHeader);
	virtual void SavePic( HVideoHeader* pHeader,CString pName,int pType=0);

private:
	vector<double> _height_img;
	int _img_w;
	int _img_h;
	void medianFilter(double* corrupted, double* smooth, int width, int height);
};

