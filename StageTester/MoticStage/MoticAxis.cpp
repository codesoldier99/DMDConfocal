
#include "StdAfx.h"
#include "MoticAxis.h"


CMoticAxis::CMoticAxis(void)
{
	InitializeCriticalSection(&_cs);
	_hCompensateEvent = CreateEvent(0,TRUE,FALSE,0);//补偿信号
	m_WMoveEvent = CreateEvent(0,TRUE,FALSE,0);
	m_bConnect=false;
}


CMoticAxis::~CMoticAxis(void)
{

}

bool CMoticAxis::InitAxis( Serial* p_Serial,MoticGearProto* p_MoticPro,AxisInfo p_Axis )
{
	Init(p_Axis);
	_serial=p_Serial;
	_proto=p_MoticPro;
	_proto->SetAxis(_info.AxisName,this);
	EnterCriticalSection(&_cs);
	char buf[64];
	_proto->SetSoftLimit(_info.AxisName, _info.sMaxPos*m_AxisMm2Count, buf,BUFSIZE);
	_serial->Send(buf);//设置软限位
	LeaveCriticalSection(&_cs);
	return false;
}

bool CMoticAxis::Init( AxisInfo p_Axis )
{
	_info=p_Axis;//存入轴
	if (_info.AxisName=='T')
		_info.AxisName='W';
	m_bConnect=false;
	m_AxisPositon=0;//具体位置
	m_AxisCount=0;
	m_AxisEncorder=0;//编码器位置
	Calibration(p_Axis.sMm2Count,p_Axis.sMm2Encoder);
	m_AxisSpeed=0;
	m_AxisStatus=0;//还未初始化
	m_AxisMove=false;
	m_AxisDirection=false;//方向
	m_AxisEncorder2Mm=1.0/m_AxisMm2Count;//转换因子
	return false;
}

void CMoticAxis::Rest( void )
{
	char buf[64];
	_proto->Reset(_info.AxisName, 0, buf,BUFSIZE);
	_serial->Send(buf);
	float ifValue=0;
	if(_info.ID ==AXIS_W)
		this->Notify(this,NOTIFY_AXISW_CHANGING,'f',&ifValue);	
	if (_info.ID ==AXIS_T)
		this->Notify(this,NOTIFY_AXIST_CHANGING,0);
}

bool CMoticAxis::IsConnect()
{
	int count=1000;
	char buf[64];
	_proto->Test(buf,BUFSIZE);
	_serial->Send(buf);
	m_bConnect=false;
	while(count--)//等待--1秒
	{
		if(m_bConnect)
			break;
		Sleep(1);
	}
	return m_bConnect;
}

void CMoticAxis::Calibration(float Mm2Count,float Mm2Encoder)
{
	m_AxisMm2Count=Mm2Count;
	m_AxisCount2Mm=1.0f/m_AxisMm2Count;
	m_AxisMm2Encorder=Mm2Encoder;
	m_AxisEncorder2Mm=1.0f/Mm2Encoder;
}

void CMoticAxis::SetSpeed( float p_Speed )
{
	char buf[64];
	int t_speed=p_Speed*m_AxisMm2Count;
	if (t_speed<=0)
		t_speed=6400;
	_proto->SetSpeed(_info.AxisName, t_speed, buf,BUFSIZE);
	_serial->Send(buf);
	m_AxisSpeed= p_Speed;
}


//这个函数最基本
void CMoticAxis::MoveRef( float p_Offset,float p_Speed)
{
	if(_info.ID ==AXIS_O)//灯光轴
	{
		EnterCriticalSection(&_cs);
		if(p_Offset < _info.sMinPos) p_Offset = _info.sMinPos;
		if(p_Offset > _info.sMaxPos) p_Offset = _info.sMaxPos;
		if(_info.sMaxPos == 99) p_Offset *= 2;
		char buf[64];
		int posCount=p_Offset*m_AxisMm2Count;//转成步数
		_proto->SetLight(posCount, buf,BUFSIZE);
		_serial->Send(buf);
		LeaveCriticalSection(&_cs);
		return ;
	}
	else//x,y,z轴
	{
		float offset;
		char buf[64];
		offset=p_Offset;	
		if(_info.ID !=AXIS_W)
		{
			if(m_AxisPositon + p_Offset < _info.sMinPos)//设置位置
				offset = _info.sMinPos - m_AxisPositon;
			if(m_AxisPositon + p_Offset > _info.sMaxPos)
				offset = _info.sMaxPos - m_AxisPositon;
			m_AxisCount=(m_AxisPositon + p_Offset)*m_AxisMm2Count;//更新电机要走到的步长
		}
		if(offset == 0)
		{
			m_AxisMove=false;
			return;
		}	
		EnterCriticalSection(&_cs);	
		if(offset > 0)//设置方向
			m_AxisDirection=true;
		else
			m_AxisDirection=false;	
		//设置速度
		if (p_Speed!=0)
			SetSpeed(p_Speed);
		else if (m_AxisSpeed!=_info.sSpeed)
			SetSpeed(_info.sSpeed);
		//运动
		int offcount=offset*m_AxisMm2Count;
		_proto->Move(_info.AxisName, offcount, buf,BUFSIZE);//相对值移动
		m_AxisMove=true;
		_serial->Send(buf);
		LeaveCriticalSection(&_cs);
	}
}

void CMoticAxis::MoveTAxis(int iTurn)
{
	float offset;
	char buf[64];
	EnterCriticalSection(&_cs);	
	for (int i=0;i<iTurn;i++)
	{
		//AfxMessageBox(L"转动一个色轮！");
		_proto->Move(_info.AxisName, 1, buf,BUFSIZE);//相对值移动
		m_AxisMove=true;
		_serial->Send(buf);
	}
	LeaveCriticalSection(&_cs);
}

void CMoticAxis::MoveTo( float p_Pos,float p_Speed)
{
	if(_info.ID ==AXIS_O)//灯光轴
	{
		EnterCriticalSection(&_cs);
		if(p_Pos < _info.sMinPos) p_Pos = _info.sMinPos;
		if(p_Pos > _info.sMaxPos) p_Pos = _info.sMaxPos;
		char buf[10];
		_proto->SetLight(p_Pos, buf,_info.sCompensation);
		_serial->Send(buf);
		LeaveCriticalSection(&_cs);
		return ;
	}

	if(_info.ID ==AXIS_W)
	{
		int iMov=p_Pos-m_AxisPositon;
		if (iMov==0)
			return;
		else
		{
			if (abs(iMov)<3)
			{
				MoveRef(iMov,p_Speed);
			}
			else
			{
				//AfxMessageBox(L"请分次移动！");
				int iK=iMov/abs(iMov)*2;
				MoveRef(iK,p_Speed);
				ResetEvent(m_WMoveEvent);
				WaitForSingleObject(m_WMoveEvent,10000);
				m_AxisPositon+=iK;
				MoveTo( p_Pos,p_Speed);
				return;
			}
		}	
		m_AxisPositon=p_Pos;
		this->Notify(this,NOTIFY_AXISW_CHANGING,'f',&m_AxisPositon);
	}
	else if (_info.ID ==AXIS_T)
	{
		if (p_Pos==0)
		{
			Rest();//复位
		}		
		else 
		{
			int iMov=p_Pos-m_AxisPositon;
			//CString str;
			//str.Format(L"NowPos:%.0f,IMov:%.0f,DstPos:%.0f,",m_AxisPositon,iMov,p_Pos);
			//AfxMessageBox(str);
			if (iMov<0)
			{
				//AfxMessageBox(L"Reset Firstly！");
				Rest();
				MoveTAxis(p_Pos);
			}
			else if (iMov==0)
				return;
			else if (iMov>0)
				MoveTAxis(iMov);
			this->Notify(this,NOTIFY_AXIST_CHANGING,p_Pos);
		}
		m_AxisPositon=p_Pos;
	}
	else 	
		MoveRef(p_Pos-m_AxisPositon,p_Speed);
}

float CMoticAxis::GetPosition()
{
	if(_info.ID ==AXIS_O)//灯光轴
	{
		EnterCriticalSection(&_cs);
		char buf[10];
		_proto->GetLight(buf,_info.sCompensation);
		_serial->Send(buf);
		LeaveCriticalSection(&_cs);
	}
	return m_AxisPositon;
}

void CMoticAxis::Move( float p_Speed )
{
	if(p_Speed==0)
	{
		if(m_AxisDirection)
			MoveTo(_info.sMaxPos,p_Speed);
		else
			MoveTo(_info.sMinPos,p_Speed);

		return;
	}
	if(p_Speed>0)
		MoveTo(_info.sMaxPos,p_Speed);
	else
		MoveTo(_info.sMinPos,p_Speed);
}

void CMoticAxis::Stop()
{
	EnterCriticalSection(&_cs);
	char buf[64];
	_proto->Stop(_info.AxisName, buf,BUFSIZE);
	_serial->Send(buf);
	LeaveCriticalSection(&_cs);
}

#pragma region EventForRecieve

void CMoticAxis::Event_CommTest()
{
	m_bConnect=true;
}

void CMoticAxis::Event_GetPosition( int pos )
{
	if(_info.ID !=AXIS_W&&_info.ID !=AXIS_T)
	{
		m_AxisEncorder=pos;
		m_AxisPositon=pos*m_AxisEncorder2Mm;
	}
	else
		SetEvent(m_WMoveEvent);
	this->Notify(this,NOTIFY_AXIS_POSITION,'f',&m_AxisPositon);
}

void CMoticAxis::Event_Moved( int pos )
{
	if(_info.ID !=AXIS_W&&_info.ID !=AXIS_T)
	{	
		m_AxisEncorder=pos;
		m_AxisPositon=pos*m_AxisEncorder2Mm;
		m_AxisMove=false;		
	}
	else
		SetEvent(m_WMoveEvent);
	this->Notify(this,NOTIFY_AXIS_MOVED,'f',&m_AxisPositon,pos+0.0f);
}

void CMoticAxis::Event_Reset()
{
	m_AxisPositon=0;
	m_AxisStatus=1;
	m_AxisMove=false;
	m_AxisEncorder=0;
}

void CMoticAxis::Event_IsMoving( bool moving )
{
	m_AxisMove=true;
}

void CMoticAxis::Event_LightChanged(int val)
{
	m_AxisPositon=val;
	this->Notify(this,NOTIFY_LIGHT_CHANGING,val);
}

//void CMoticAxis::Event_GetEncorder( int pos )
//{
//	m_AxisEncorder=pos*m_AxisEncorder2Mm;
//	this->Notify(this,NOTIFY_AXIS_ENCORDER,'f',&m_AxisEncorder);
//}

#pragma endregion EventForRecieve
