#include "StdAfx.h"
#include "GaliaAxis.h"


CGaliaAxis::CGaliaAxis(void)
{
	InitializeCriticalSection(&_cs);
	_stopcount = 0;
	_hCompensateEvent = CreateEvent(0,TRUE,FALSE,0);//补偿信号
	m_bThredExit=TRUE;
	m_Galil=0;
	m_AxisName='A';
	m_AxisCount=0;
	m_AxisPositon=0;
}


CGaliaAxis::~CGaliaAxis(void)
{
	CloseAxis();
	m_Galil=0;
	m_bThredExit=false;//退出线程
}

bool CGaliaAxis::Init( AxisInfo p_Axis )
{
	switch(p_Axis.ID)//初始化中:
	{
	case AXIS_X://x轴
		m_AxisName='B';
		//m_AxisMg_Moved="AXISB_MOVED";//到位信息
		break;
	case  AXIS_Y://y轴
		m_AxisName='A';
		//m_AxisMg_Moved="AXISA_MOVED";
		break;
	case  AXIS_Z://y轴
		m_AxisName='C';
		//m_AxisMg_Moved="AXISA_MOVED";
		break;
	default://没有这个轴
		return false;
		break;
	}
	_info=p_Axis;//存入轴
	m_AxisPositon=0;
	m_AxisSpeed=0;
	m_AxisEncorder=0;
	m_AxisStatus=0;//还未初始化
	m_AxisMove=false;
	m_AxisDirection=false;//方向
	m_AxisCount=0;
	Calibration(p_Axis.sMm2Count,p_Axis.sMm2Encoder);
	return false;
}

bool CGaliaAxis::InitAxis(CGalilCom* p_Hander,AxisInfo p_Axis)
{
	//设置轴信息
	Init(p_Axis);
	m_Galil=p_Hander;
	m_Galil->SendAxisCmd(m_AxisName,L"SH");
	m_Galil->SetAxisHook(m_AxisName,this);
	SetSpeed(_info.sSpeed);//设置速度
	return false;
}


void CGaliaAxis::CloseAxis( void )
{
	m_bThredExit=false;
	m_Galil->SendAxisCmd(m_AxisName,L"MO");
}

void CGaliaAxis::Rest( void )
{
	FlushPosition();
	if (m_AxisPositon==0)
	{
		Sleep(30);
		Event_GetPosition(-1);
		Event_Moved(-1);
		return;
	}
	char buf[64];
	m_Galil->SendAxisCmd(m_AxisName,L"ST");	
	//m_Galil->SendAxisCmd(m_AxisName,L"DP",0);//重设置为0
	if(m_AxisName=='C')
	{
		//m_AxisEncorder=ReadEncoder();
		//m_AxisPositon=m_AxisEncorder*m_AxisEncorder2Mm;
		if (_info.sbInvert)
			m_Galil->AxisMove(m_AxisName,m_AxisPositon*m_AxisMm2Count);//相对运动
		else
			m_Galil->AxisMove(m_AxisName,-m_AxisEncorder*m_AxisMm2Count);
		//m_Galil->AxisMove()
		return;
	}	

	m_Galil->AxisMove(m_AxisName,0);//绝对位置
}

void CGaliaAxis::Calibration(float Mm2Count,float Mm2Encoder)
{
	m_AxisMm2Count=Mm2Count;
	m_AxisCount2Mm=1.0f/m_AxisMm2Count;
	m_AxisMm2Encorder=Mm2Encoder;
	m_AxisEncorder2Mm=1.0f/Mm2Encoder;
}

void CGaliaAxis::SetSpeed( float p_Speed )
{
	char buf[64];
	int t_speed=p_Speed*m_AxisMm2Count;
	//发送速度
	m_Galil->SendAxisCmd(m_AxisName,L"SP",t_speed);
	m_Galil->SendAxisCmd(m_AxisName,L"DC",t_speed*10);
	m_Galil->SendAxisCmd(m_AxisName,L"AC",t_speed*10);
	m_AxisSpeed= p_Speed;
}

//这个函数最基本
void CGaliaAxis::MoveRef( float p_Offset,float p_Speed/*=-1*/ )
{
	FlushPosition();
	//x,y,z轴
	float offset;
	char buf[64];
	offset=p_Offset;
	//设置位置
	if(m_AxisPositon + p_Offset < _info.sMinPos)
		offset = _info.sMinPos - m_AxisPositon;
	if(m_AxisPositon + p_Offset > _info.sMaxPos)
		offset = _info.sMaxPos - m_AxisPositon;
	if(offset == 0)
		return ;

	EnterCriticalSection(&_cs);
	//设置方向
	if(offset > 0)
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
	//m_AxisCount+=offcount;//这里用 count数来计量
	m_AxisCount=(m_AxisPositon + p_Offset)*m_AxisMm2Count;//更新电机要走到的步长

	if(m_AxisName=='C')//2014-0521-换
	{
		if (_info.sbInvert)
			m_Galil->AxisMove(m_AxisName,-offcount);
		else
			m_Galil->AxisMove(m_AxisName,offcount);//相对值移动
	}
	else
	{
		if(_info.sbInvert)
			m_Galil->AxisMove(m_AxisName,-m_AxisCount);//绝对值移动
		else
			m_Galil->AxisMove(m_AxisName,m_AxisCount);
	}
	TRACE(L"Axis Moving,Count:%d\n",m_AxisCount);
	m_AxisMove=true;	
	LeaveCriticalSection(&_cs);
}

void CGaliaAxis::MoveTo( float p_Pos,float p_Speed/*=-1*/ )
{
	FlushPosition();
	if (p_Pos-m_AxisPositon==0)
	{
		Sleep(30);
		Event_GetPosition(-1);
		Event_Moved(-1);
		return;
	}
	MoveRef(p_Pos-m_AxisPositon,p_Speed);
}

void CGaliaAxis::Move( float p_Speed/*=-1*/ )
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
	{
		MoveTo(_info.sMaxPos,p_Speed);
	}
	else
	{
		MoveTo(_info.sMinPos,p_Speed);
	}
}

bool CGaliaAxis::IsMoveing()
{
	return m_AxisMove;
}

void CGaliaAxis::Stop()
{
	EnterCriticalSection(&_cs);
	if(m_AxisMove)
	{
		m_Galil->SendAxisCmd(m_AxisName,L"ST");//停止命令
		Sleep(40); // 保证 Stop 命令同步完成
		_stopcount++;
		if(_stopcount > 1)
		{
			_stopcount = 0;
			m_AxisMove = false;
		}
	}
	LeaveCriticalSection(&_cs);
}

void CGaliaAxis::Event_GetPosition( int pos )
{
	if (pos<0)
		m_AxisEncorder=ReadEncoder();
	else
		m_AxisEncorder=pos;
	m_AxisPositon=m_AxisEncorder*m_AxisEncorder2Mm;
	this->Notify(this,NOTIFY_AXIS_POSITION,'f',&m_AxisPositon);
}

void CGaliaAxis::FlushPosition()
{
	m_AxisEncorder=ReadEncoder();
	m_AxisPositon=m_AxisEncorder*m_AxisEncorder2Mm;
}

void CGaliaAxis::Event_Moved( int pos )
{
	if (pos<0)
		m_AxisEncorder=ReadEncoder();
	else
		m_AxisEncorder=pos;
	m_AxisPositon=m_AxisEncorder*m_AxisEncorder2Mm;
	m_AxisMove=false;
	this->Notify(this,NOTIFY_AXIS_MOVED,'f',&m_AxisPositon,m_AxisCount+0.0f);
}


void CGaliaAxis::Event_Reset()
{
	m_AxisPositon=0;
	m_AxisStatus=1;
	m_AxisMove=false;
	m_AxisEncorder=0;
}

void CGaliaAxis::Event_IsMoving( bool moving )
{
	m_AxisMove=true;
}

//void CGaliaAxis::Event_GetEncorder( int pos )
//{
//	m_AxisEncorder=pos*m_AxisCount2Mm;
//	this->Notify(this,NOTIFY_AXIS_ENCORDER,'f',&m_AxisEncorder);
//}

float CGaliaAxis::ReadEncoder()
{	
	double pos=0;
	float iDirect=_info.sbEncoderInvert?-1.0f:1.0f;
	if(m_Galil->GetAxisValue(m_AxisName,L"TP",pos))
		//return  abs(pos);
		return  iDirect*pos;
	else
		return 0;
}


