#pragma once

#include "PatternGeneratorImpl.h"

using namespace WlpDMDControl;

class PatternLine:public PatternGeneratorImpl
{
public:
	PatternLine(void);
	virtual ~PatternLine(void);
	//产生数据 parame1宽度，parame2代表图像个数
	virtual void Gernerate(void* Paras);
	unsigned char* GetData(int index,int iGray);//得到当前数据
	int GetFrameCount();//得到这个模式下的总帧数
	int GetMode();//得到这个模式类型
	bool SetSave(bool bSave,CString strPath);
protected:
  void Binary2Gray(int width, int height, unsigned char* src, unsigned char* dst);
protected:
	unsigned char* m_pData;//数据指针
  unsigned char* m_pData2;//图像指针
	CString strSavePath;
	bool bS;

	LinePara* mParas;
};

