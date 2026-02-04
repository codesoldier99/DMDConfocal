#pragma once

#include "HGearBox.h"
#include "Serial.h"
#include "MoticGearProto.h"

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
	HAxis* m_AxisX[8];//支持8轴
	CString m_GearBoxName;//控制盒名字
	Serial m_MoticSerialCom;//motic的通信接口
	MoticGearProto m_MoticProto;//motic的协议

	Serial m_TurnSerialCom;//色轮转盘的通信接口
	MoticGearProto m_TurnProto;//motic的协议
};

