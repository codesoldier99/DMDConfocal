
#pragma once

#include "IHsmObserver.h"

#define NOTIFY_AXIS_MOVED 4001
#define NOTIFY_AXIS_RESTED  4002
#define NOTIFY_AXIS_POSITION 4003
#define NOTIFY_AXIS_ENCORDER 4004
#define NOTIFY_AXISW_CHANGING 4005//物镜转换，
#define NOTIFY_LIGHT_CHANGING 4006
#define NOTIFY_AXIST_CHANGING 4007//滤色块转换，发送转换完的滤色块位置

enum AxisID
{
	AXIS_X,
	AXIS_Y,
	AXIS_Z,
	AXIS_W,
	AXIS_PIZ,
	AXIS_T,//色盘
	AXIS_O
};

struct HdsStageParas
{
	CString strIP;
	int iZCom;
	int iWCom;
};

struct MoticStageParas
{
	int iAxisCom;
	int iTurnCom;
};

struct AxisInfo
{
	AxisID ID;//轴ID
	char AxisName;//轴名  要大写
	float sMinPos;//最小位置
	float sMaxPos;//最大位置
	float sMm2Count;//步数转换值 mm转成count
	float sMm2Encoder;//位置反馈编码转化，
	float sSpeed;//速度参考值，与特定设备有关。实际的基本速度=fBaseSpeedRate*nSpeed;
	float sResetPos;//复位的绝对位置		
	long sCompensation;//空回补偿值       //对于灯光轴来说，代表灯光控制是99级[6]还是999级[8]
	bool sbInvert; //轴是否倒过来
	bool sbEncoderInvert;//返回位置值的反向与否
	bool sbReset; //是否复位
};


class HAxis
{
	public:
		virtual ~HAxis(void){};
		virtual IHsmSubject* GetSubject()=0;//得到观察者接口
		virtual bool Init(AxisInfo p_Axis)=0;//初始化轴
		virtual bool UnInit()=0;//{return true};//初始化轴
		virtual void Rest(void)=0;//复位轴，复位到初始化参数及原点
		virtual AxisInfo* GetAxialInfo()=0;//得到参数
		virtual void Calibration(float Mm2Count,float Mm2Encoder)=0;//较准步数 mm/div
		virtual float GetPosition()=0;//得到位置 mm单位
		virtual float GetCount()=0;//得到移动步长
		virtual float GetEncorder()=0;//得到编码器
		virtual void SetSpeed(float p_Speed)=0;//设置速度 mm/s
		virtual float GetSpeed()=0;//得到速度 mm/s
		virtual void MoveRef(float p_Offset,float p_Speed=0)=0;//相对位置移动 mm& mm/s
		virtual void MoveTo(float p_Pos,float p_Speed=0)=0;//绝对位置移动 mm& mm/s
		virtual void Move(float p_Speed=0)=0;//持续运动 mm/s
		virtual bool IsMoveing()=0;//是否移动
		virtual void Stop()=0;//停止轴运动
		virtual bool IsConnect()=0;//是否在还线
		//轴控制响应
		virtual void Event_CommTest()=0;
		virtual void Event_GetPosition(int pos)=0;
		virtual void Event_Moved(int pos)=0;
		virtual void Event_Error()=0;
		virtual void Event_Reset()=0;
		virtual void Event_IsMoving(bool moving)=0;
		//virtual void Event_GetEncorder(int pos)=0;
};

class HGearBox
{
	public:
		virtual ~HGearBox(void){};
		virtual bool InitGearBox(CString p_Name,void* wParam,
			AxisInfo* pAxisInfo,int nAxisCnt)=0;//初始化__控制盒__param自由控制
		virtual HAxis* GetAxis(AxisID p_ID)=0;//获取轴
		virtual HAxis* GetAxis(int p_Index)=0;//编号模式
		virtual int IsConnect()=0;//是否在还线--这个有点重要。。,哪个bit位置1，哪轴在线
		virtual bool UnInit()=0;//卸载
};
