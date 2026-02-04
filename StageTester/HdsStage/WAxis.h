#pragma once

#include "HGearBox.h"
//#include "TmcProto.h"
#include "HTimer.h"
//#include "Serial.h"
//#include "MoticGearProto.h"

//这里暂时只支持一个电机，一个通信口，也可以改一下
//这个版本没有ac,dc,设置，只设速度。。亲。。。。。

class CWAxis:public HAxis,public IHsmSubject,public HTimerHook
{

public:
	CWAxis(void);
	~CWAxis(void);

public://以下是实现
	IHsmSubject* GetSubject() {return (IHsmSubject*)this;};
	virtual bool Init(AxisInfo p_Axis);//初始化轴
	virtual bool UnInit();
	virtual void Rest(void);//复位
	virtual AxisInfo* GetAxialInfo(){return &_info;};//得到参数
	virtual void Calibration(float Mm2Count,float Mm2Encoder);
	virtual float GetPosition(){return m_AxisPositon;};//得到位置 mm单位
	virtual float GetEncorder(){return m_AxisEncorder;};//得到编码器位置
	virtual float GetCount(){return m_AxisCount;};
	virtual void SetSpeed(float p_Speed);//设置速度 mm/s
	virtual float GetSpeed(){return m_AxisSpeed;};//得到速度 mm/s
	virtual void MoveRef(float p_Offset,float p_Speed=0);//相对位置移动 mm& mm/s
	virtual void MoveTo(float p_Pos,float p_Speed=0);//绝对位置移动 mm& mm/s
	virtual void Move(float p_Speed=-1);//持续运动 mm/s
	virtual bool IsMoveing();//是否运动
	virtual void Stop();//停止轴运动
	virtual bool IsConnect() {return true;};//是否在还线
	/////以下是通信接收
	virtual void Event_CommTest(){};
	virtual void Event_GetPosition(int pos);
	virtual void Event_Moved(int pos);
	virtual void Event_Error(){};
	virtual void Event_Reset();
	virtual void Event_IsMoving(bool moving);
	virtual void Event_GetEncorder(int pos);
	virtual void OnHTimer(int nID);
public://以下是不同轴的自有功能
	void SetSoftLimit();//设置软限位 
	bool InitAxis(HANDLE p_Hander,BYTE p_MotorInd,AxisInfo p_Axis);//通过控制盒初始化  其中motorind是电机地址
	void Event_Moving();	
private:
	//Send a binary TMCL command
	//e.g.  SendCmd(ComHandle, 1, TMCL_MVP, MVP_ABS, 1, 50000);   will be MVP ABS, 1, 50000 for a module with address 1
	//Parameters: Handle: Handle of the serial port (returned by OpenRS232).
	//            Address: address of the module (factory default is 1).
	//            Command: the TMCL command (see the constants at the begiining of this file)
	//            Type:    the "Type" parameter of the TMCL command (set to 0 if unused)
	//            Motor:   the motor number (set to 0 if unused)
	//            Value:   the "Value" parameter (depending on the command, set to 0 if unused)
	void SendCmd(UCHAR Command, UCHAR Type,INT Value);
	//Read the result that is returned by the module
	//Parameters: Handle: handle of the serial port, as returned by OpenRS232
	//            Address: pointer to variable to hold the reply address returned by the module
	//            Status: pointer to variable to hold the status returned by the module (100 means okay)
	//            Value: pointer to variable to hold the value returned by the module
	//Return value: TMCL_RESULT_OK: result has been read without errors
	//              TMCL_RESULT_NOT_READY: not enough bytes read so far (try again)
	//              TMCL_RESULT_CHECKSUM_ERROR: checksum of reply packet wrong
	BYTE GetResult(UCHAR *Address, UCHAR *Status,BYTE *Cmds ,int *Value);

#define CMD_SET_SPEED 0x01
#define CMD_SET_POSITION 0x02
#define CMD_STOP	0x04
#define CMD_REQ_POSITION 0x08
#define CMD_REST	0x10

protected:
	AxisInfo _info;
	CRITICAL_SECTION _cs;
	int _stopcount;// 计算执行停止的命令，用于丢失指令后恢复
	HANDLE _hCompensateEvent;

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

	//这下面是tmc电机特有的
	HANDLE m_AxisHandel;//控制句柄，如果在这做同接口不同电机貌似行的
	BYTE m_MotorIndex;
	BYTE m_MotorAddress;
	HTimer m_ReadProcess;
	bool m_bThredExit;
	BYTE m_MotorIns;//指令
	bool m_MotoWait;//是否等待
	bool m_bSetting;
};

