#pragma once

#include "HGearBox.h"
#include "Serial.h"
#include "MoticGearProto.h"

class CMoticAxis:public HAxis,public IHsmSubject,public HAxisDispatcher
{

public:
	CMoticAxis(void);
	~CMoticAxis(void);

public://以下是实现
	IHsmSubject* GetSubject() {return this;};
	virtual bool Init(AxisInfo p_Axis);//初始化轴
	virtual bool UnInit(){return true;};
	virtual void Rest(void);//复位
	virtual AxisInfo* GetAxialInfo(){return &_info;};//得到参数
	virtual void Calibration(float Mm2Count,float Mm2Encoder);
	virtual float GetPosition();//{return m_AxisPositon;};//得到位置 mm单位
	virtual float GetCount(){return m_AxisCount;};//得到电机转动的步长
	virtual float GetEncorder(){return m_AxisEncorder;};//得到编码器位置
	virtual void SetSpeed(float p_Speed);//设置速度 mm/s
	virtual float GetSpeed(){return m_AxisSpeed;};//得到速度 mm/s
	virtual void MoveRef(float p_Offset,float p_Speed=0);//相对位置移动 mm& mm/s
	virtual void MoveTo(float p_Pos,float p_Speed=0);//绝对位置移动 mm& mm/s
	virtual void Move(float p_Speed=-1);//持续运动 mm/s
	virtual bool IsMoveing(){return m_AxisMove;};//是否运动
	virtual void Stop();//停止轴运动
	virtual bool IsConnect();//是否在线的测试
	virtual void Event_CommTest();
	virtual void Event_GetPosition(int pos);
	virtual void Event_Moved(int pos);
	virtual void Event_Error(){};
	virtual void Event_Reset();
	virtual void Event_IsMoving(bool moving);
	virtual void Event_LightChanged(int val);

	//virtual void Event_GetEncorder(int pos);
public://以下是不同轴的自有功能
	#define BUFSIZE 64
	bool InitAxis(Serial* p_Serial,MoticGearProto* p_MoticPro,AxisInfo p_Axis);//通过控制盒初始化
	
private:
	AxisInfo _info;
	Serial* _serial;
	MoticGearProto* _proto;
	bool _init;	
	int _resetting;// 是否正在复位
	bool m_bConnect;
	CRITICAL_SECTION _cs;	
	HANDLE _hCompensateEvent;
	HANDLE m_WMoveEvent;

protected://记录当前状态
	float m_AxisPositon;//当前位置 mm

	float m_AxisCount;//指示电机运动的步长
	float m_AxisMm2Count;//mm转步数
	float m_AxisCount2Mm;//步数转mm

	float m_AxisEncorder;//当前编码器位置 mm
	float m_AxisMm2Encorder;//mm转编码器
	float m_AxisEncorder2Mm;

	float m_AxisSpeed;//当前速度 mm/s	
	int  m_AxisStatus;//当前状态 -1未初始化 0 没有复位 1复位且初始化
	bool m_AxisMove;//移动
	bool m_AxisDirection;//方向 

//private:
	//int iLight;
private: 
	void MoveTAxis(int iTurn);
};

