#include "StdAfx.h"
#include "WAxis.h"


CWAxis::CWAxis(void)
{
	InitializeCriticalSection(&_cs);
	_stopcount = 0;
	_hCompensateEvent = CreateEvent(0,TRUE,FALSE,0);//补偿信号
	m_AxisHandel=INVALID_HANDLE_VALUE;
	m_MotorIndex=0;
	m_MotorAddress=0;
	m_bThredExit=TRUE;
	m_MotorIns=0;
	m_MotoWait=false;
	m_bSetting=false;//进入设置模式
}


CWAxis::~CWAxis(void)
{
	m_AxisHandel=INVALID_HANDLE_VALUE;
	m_bThredExit=false;//退出线程
}

bool CWAxis::Init( AxisInfo p_Axis )
{
	switch(p_Axis.ID)//初始化中
	{
		case AXIS_X://x轴
			 break;
		default://没有这个轴
			break;
	}
	_info=p_Axis;//存入轴
	m_AxisPositon=0;
	m_AxisCount=0;
	m_AxisSpeed=0;
	m_AxisEncorder=0;
	m_AxisStatus=0;//还未初始化
	m_AxisMove=false;
	m_AxisDirection=false;//方向
	Calibration(p_Axis.sMm2Count,p_Axis.sMm2Encoder);
	m_AxisCount2Mm=1.0/m_AxisMm2Count;//转换因子	
	return false;
}

void CWAxis::Calibration(float Mm2Count,float Mm2Encoder)
{
	m_AxisMm2Count=Mm2Count;
	m_AxisCount2Mm=1.0f/m_AxisMm2Count;
	m_AxisMm2Encorder=Mm2Encoder;
	m_AxisEncorder2Mm=1.0f/Mm2Encoder;
}

bool CWAxis::InitAxis(HANDLE p_Hander,BYTE p_MotorInd,AxisInfo p_Axis)
{
	Init(p_Axis);
	//设参数
	m_MotorAddress=p_MotorInd;
	m_MotorIndex=0;
	m_AxisHandel=p_Hander;
	//建线程。。。

	//m_ReadProcess.ChangeMode(false);
	//m_ReadProcess.SetTimer(this,529,500);

	//SendCmd(138,0,0);//这个使能输出
	//下面设置参数
	/*
	m_bSetting=true;//进入设置模式

	//SendCmd(TMCL_SAP,5,500000);//设置加速度

	while(m_MotoWait)
		Sleep(1);

	SendCmd(TMCL_SAP,4,500000);//设置加速度

	while(m_MotoWait)
		Sleep(1);

	m_bSetting=false;//退出
	*/
	return true;
}


bool CWAxis::UnInit()
{
	//退出线程
	m_bThredExit=false;
	//if(m_AxisHandel)
	//	CloseHandle(m_AxisHandel);
	m_AxisHandel=0;
	return true;
}


void CWAxis::Rest( void )
{
	m_AxisPositon=0;
	m_AxisSpeed=0;
	m_AxisEncorder=0;
	m_AxisCount=0;
	//m_AxisStatus=0;//还未初始化
	m_AxisMove=false;
	m_AxisDirection=false;//方向

	m_MotorIns|=CMD_STOP;//先停后复位，这个非常重要！！！
	m_MotorIns|=CMD_REST;
	//MoveTo(0);
	this->Notify(this,NOTIFY_AXIS_MOVED);
}


void CWAxis::SetSpeed( float p_Speed )
{
	m_MotorIns|=CMD_SET_SPEED;//设置命令
	m_AxisSpeed= p_Speed;
}


//这个函数最基本
void CWAxis::MoveRef( float p_Offset,float p_Speed/*=-1*/ )
{
	float offset;
	offset=p_Offset;
	/*
	//设置位置
	if(m_AxisPositon + p_Offset < _info.sMinPos)
	{
		offset = _info.sMinPos - m_AxisPositon;
	}
	if(m_AxisPositon + p_Offset > _info.sMaxPos)
	{
		offset = _info.sMaxPos - m_AxisPositon;
	}
	if(offset == 0)
		return ;
*/
	EnterCriticalSection(&_cs);
	//设置方向
	if(offset > 0)
		m_AxisDirection=true;
	else
		m_AxisDirection=false;

	//运动
	//int offcount=offset*m_AxisMm2Count;
	m_AxisCount=p_Offset*m_AxisCount2Mm;

	SendCmd('M','O',p_Offset);
	m_AxisMove=false;
	//m_AxisPositon+=offset;//先记录位置--后面会修好改
	//this->Notify(this,NOTIFY_AXIS_MOVED);	
	LeaveCriticalSection(&_cs);
}

void CWAxis::MoveTo( float p_Pos,float p_Speed/*=-1*/ )
{
	MoveRef(p_Pos,p_Speed);
}

void CWAxis::Move( float p_Speed/*=-1*/ )
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

bool CWAxis::IsMoveing()
{
	return false;
}


void CWAxis::Stop()
{
	EnterCriticalSection(&_cs);
	if(m_AxisMove)
	{
		m_MotorIns|=CMD_STOP;
		//SendCmd(TMCL_MST,0,0);//停止
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

void CWAxis::Event_GetPosition( int pos )
{
	m_AxisEncorder=pos;
	m_AxisPositon=pos*m_AxisEncorder2Mm;
	this->Notify(this,NOTIFY_AXIS_POSITION,'f',&m_AxisPositon);
}

void CWAxis::Event_Moved( int pos )
{
	m_AxisEncorder=pos;
	m_AxisPositon=pos*m_AxisEncorder2Mm;//转成当前位置....
	m_AxisMove=false;
	TRACE(L"Axis is Moved\n");
	this->Notify(this,NOTIFY_AXIS_MOVED,'f',&m_AxisPositon);
}

void CWAxis::Event_Reset()
{
	m_AxisPositon=0;
	m_AxisStatus=1;
	m_AxisMove=false;
	m_AxisEncorder=0;
}

void CWAxis::Event_IsMoving( bool moving )
{
	m_AxisMove=moving;
}

void CWAxis::Event_GetEncorder( int pos )
{
	m_AxisEncorder=pos*m_AxisCount2Mm;
	this->Notify(this,NOTIFY_AXIS_ENCORDER,'f',&m_AxisEncorder);
}


//向电机发消息
void CWAxis::SendCmd(UCHAR Command, UCHAR Type,INT Value )
{
	if(m_AxisHandel==INVALID_HANDLE_VALUE)
		return;
	EnterCriticalSection(&_cs);
	BYTE TxBuffer[9];
	DWORD BytesWritten;
	int i;
	TxBuffer[0]='*';
	TxBuffer[1]='0';
	TxBuffer[2]='1';
	TxBuffer[3]='E';		//这里设置了！！
	TxBuffer[4]='0'+Value;
	TxBuffer[5]='$';
	//Send the datagram
	WriteFile(m_AxisHandel, TxBuffer, 6, &BytesWritten, NULL);
	LeaveCriticalSection(&_cs);
	//m_MotoWait=true;//等结果---------
}

BYTE CWAxis::GetResult( UCHAR *Address, UCHAR *Status,BYTE *Cmds, int *Value )
{
	/*BYTE RxBuffer[9], Checksum;
	DWORD Errors, BytesRead;
	COMSTAT ComStat;
	int i;

	//Check if enough bytes can be read
	ClearCommError(m_AxisHandel, &Errors, &ComStat);
	if(ComStat.cbInQue>8)
	{
		//Receive
		ReadFile(m_AxisHandel, RxBuffer, 9, &BytesRead, NULL);

		Checksum=0;
		for(i=0; i<8; i++)
			Checksum+=RxBuffer[i];

		if(Checksum!=RxBuffer[8])
		{
			ClearCommError(m_AxisHandel, &Errors, &ComStat);
			if(ComStat.cbInQue>0&&ComStat.cbInQue<8)
				ReadFile(m_AxisHandel, RxBuffer, 9, &BytesRead, NULL);//清空他！！！

			return TMCL_RESULT_CHECKSUM_ERROR;
		}

		*Address=RxBuffer[0];
		*Status=RxBuffer[2];
		*Cmds=RxBuffer[3];//
		*Value=(RxBuffer[4] << 24) | (RxBuffer[5] << 16) | (RxBuffer[6] << 8) | RxBuffer[7];
	} 
	else
	{
		return TMCL_RESULT_NOT_READY;
	}

	return TMCL_RESULT_OK;
	*/
	return 0;
}

void CWAxis::OnHTimer( int nID )
{
	//单线程模式
/*
	BYTE trec;
	BYTE Addrs,Status,Cmds;
	int Values;
	int Failure=0;
	bool t_Ack=false;//等回复消息

	while(m_bThredExit)
	{
		trec=GetResult(&Addrs,&Status, &Cmds, &Values);
			
		if(trec==TMCL_RESULT_OK)
		{
			m_MotoWait=false;//解锁

			if(m_bSetting)//下面是设置模式，不解码
			{
				Sleep(3);
				continue;
			}

			switch(Cmds)
			{
			case TMCL_MVP:	
				if(Status==TMC_STATUS_OK)//返回100是成功
					m_AxisMove=true;//说明正在移动
				break;
			case TMCL_MST:	
				if(Status==TMC_STATUS_OK)//返回100是成功
					m_AxisMove=false;//说明停止了
				break;
			case TMCL_GAP:	
				if(Status==TMC_STATUS_OK)//返回100是成功
				{

					if(m_AxisCount==Values)//因为电机就是以这个计数的，所以不怕死
						Event_Moved(Values);

					Event_GetPosition(Values);
				}
				break;	
			case 138://这个是设置达位置返回功能

				if (Status==128)//这个是位置到达指令100是发送成功消息
					Event_Moved(m_AxisCount);//

				break;
			}
			TRACE(L"Notify%d,%d,%d\n",Status,Cmds,Values);
		}

		if(m_MotoWait)//保护结果
		{
			Sleep(3);
			if(Failure++>1000)
				m_MotoWait=false;
			continue;
		}
		Sleep(3);
		Failure=0;

		if(m_MotorIns&CMD_STOP)//停止移动消息
		{
			m_MotorIns&=~CMD_STOP;//清除标志

			SendCmd(TMCL_MST,0,0);//停止消息用！

			continue;
		}

		if(m_MotorIns&CMD_REST)//复位消息
		{
			m_MotorIns&=~CMD_REST;//清除标志

			SendCmd(TMCL_SAP,ACTUAL_POS,0);//设置为0
			continue;
		}		

		if(m_MotorIns&CMD_SET_SPEED)//设置速度消息
		{
			m_MotorIns&=~CMD_SET_SPEED;//清除标志

			int t_speed=m_AxisSpeed*m_AxisMm2Count;

			SendCmd(TMCL_SAP,MAX_SPEED,t_speed);//设置速度

			continue;
		}

		if(m_MotorIns&CMD_SET_POSITION)//移动消息
		{
			

			//int t_Pos=m_AxisPositon*m_AxisMm2Count;
			if(t_Ack)
			{
				m_MotorIns&=~CMD_SET_POSITION;//清除标志
				SendCmd(TMCL_MVP,MVP_ABS,m_AxisCount);//用绝对位置哦！
				t_Ack=false;//已经发完消息了
			}
			else
			{
				SendCmd(138,0,1);//到位消息
				t_Ack=true;
			}
			

			continue;
		}

		if(m_MotorIns&CMD_REQ_POSITION)//获取位置
		{
			m_MotorIns&=~CMD_REQ_POSITION;//清除标志

			SendCmd(TMCL_GAP,ACTUAL_POS,0);//位置

			continue;
		}

		//step2://查询是否到达位置
		//if(m_AxisMove)//正在移动
		//{
		//	m_MotorIns|=CMD_REQ_POSITION;//查位置
		//}


		//Sleep(30);//延时
		
	}
	*/
}

