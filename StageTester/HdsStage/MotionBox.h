#pragma once

#include "HGearBox.h"
#include "Galil.h"
#include "GalilCom.h"

#include <sstream>   //ostringstream istringstream
using namespace std; //cout ostringstream vector string

class CMotionBox:public HGearBox
{

public:
	CMotionBox(void);
	~CMotionBox(void);
	virtual bool InitGearBox(CString p_Name,void* wParam,
		AxisInfo* pAxisInfo,int nAxisCnt);//初始化__控制盒__param自由控制
	virtual HAxis* GetAxis(AxisID p_ID);//获取轴
	virtual HAxis* GetAxis(int p_Index);
	virtual bool UnInit();//卸载
	virtual int IsConnect();//哪个bit位置1，哪轴在线
protected:
	CString m_GearBoxName;//控制盒名字

	HAxis* m_AxisX[8];//支持8轴
	CGalilCom m_GalilCom;//XY轴

	HANDLE m_TMC_SerialCom;//TCM的通信接口，Z轴
	HANDLE m_WAxis_SerialCom;//TCM的通信接口，W轴
protected:
	HANDLE OpenRS232(int pPort, int BaudRate);//通信接口
};

