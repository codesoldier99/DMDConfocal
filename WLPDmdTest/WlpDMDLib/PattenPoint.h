#pragma once

#include "PatternGeneratorImpl.h"

using namespace WlpDMDControl;

class PattenPoint :public PatternGeneratorImpl
{
public:
	PattenPoint(void);
	virtual ~PattenPoint(void);
	//产生数据 parame1点大小，parame2点间隔
	//生成一个128*768的模板，以此来移动造图
	virtual void Gernerate(void* Paras);
	unsigned char* GetData(int index,int iGray);//得到当前数据
	int GetFrameCount();//得到这个模式下的总帧数
	int GetMode();//得到这个模式类型
	bool SetSave(bool bSave,CString strPath);
protected:
	int m_FrameCount;//总的数据

	unsigned char* m_pData;//数据指针
	unsigned char* m_pData2;//图像指针
	void ChangeFormate( unsigned char* scr,unsigned char* dest );
	CString strSavePath;
	bool bS;
	PointPara* mParas;
};

