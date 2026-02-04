#include "StdAfx.h"
#include "TmcAxis.h"


CTmcAxis::CTmcAxis(void)
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
	m_end=false;

	_ow.hEvent = CreateEvent(0, FALSE, FALSE, 0);
	_ow.Internal = 0;
	_ow.InternalHigh = 0;
	_ow.Offset = 0;
	_ow.OffsetHigh = 0;
	_ow.Pointer = 0;

	_omask.hEvent = CreateEvent(0, FALSE, FALSE, 0);
	_omask.Internal = 0;
	_omask.InternalHigh = 0;
	_omask.Offset = 0;
	_omask.OffsetHigh = 0;
	_omask.Pointer = 0;

	_or.hEvent = CreateEvent(0, FALSE, FALSE, 0);
	_or.Internal = 0;
	_or.InternalHigh = 0;
	_or.Offset = 0;
	_or.OffsetHigh = 0;
	_or.Pointer = 0;
}

CTmcAxis::~CTmcAxis(void)
{
	m_AxisHandel=INVALID_HANDLE_VALUE;
	m_bThredExit=false;//退出线程
}

bool CTmcAxis::Init( AxisInfo p_Axis )
{
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

void CTmcAxis::Calibration(float Mm2Count,float Mm2Encoder)
{
	m_AxisMm2Count=Mm2Count;
	m_AxisCount2Mm=1.0f/m_AxisMm2Count;
	m_AxisMm2Encorder=Mm2Encoder;
	m_AxisEncorder2Mm=1.0f/Mm2Encoder;
}

bool CTmcAxis::InitAxis(HANDLE p_Hander,BYTE p_MotorInd,AxisInfo p_Axis)
{
	Init(p_Axis);
	//设参数
	m_MotorAddress=p_MotorInd;
	m_MotorIndex=0;
	m_AxisHandel=p_Hander;
	//建线程。。。
	m_ReadProcess.ChangeMode(false);
	m_ReadProcess.SetTimer(this,529,500);
	_proto.SetAxis(_info.AxisName,this);
	//下面设置参数
	//int count=0;
	//m_bSetting=true;//进入设置模式
	//SendCmd(TMCL_SAP,5,500000);//设置加速度
	//while(m_MotoWait)
	//{
	//	Sleep(1);
	//	if(count++>=5000)
	//		break;
	//}
	//SendCmd(TMCL_SAP,4,500000);//设置加速度
	//count=0;
	//while(m_MotoWait)
	//{
	//	Sleep(1);
	//	if(count++>=5000)
	//		break;
	//}
	//m_bSetting=false;//退出
	return false;
}

void CTmcAxis::RestProcess(void)
{
	//step1: 查开关
	//step2: 向上移动位置
	SendCmd(TMCL_SAP,5,500000);//设置加速度
	//step3: 往回移动
	//step4:  查开关
	//step5: 设置原点
	while(m_MotoWait)
	{
		Sleep(1);
		//if(count++>=5000)
			//break;
	}
}

bool CTmcAxis::UnInit( )
{
	//退出线程
	m_bThredExit=false;
	m_AxisHandel=0;
	return true;
}

void CTmcAxis::Rest( void )
{
	char buf[64];
	_proto.Reset(_info.AxisName, 0, buf,64);
	if(m_AxisHandel==INVALID_HANDLE_VALUE)
		return;
	DWORD BytesWritten;
	EnterCriticalSection(&_cs);
	WriteFile(m_AxisHandel, buf, strlen(buf),&BytesWritten,NULL);
	LeaveCriticalSection(&_cs);
	m_MotoWait=true;

	//m_AxisPositon=0;
	//m_AxisSpeed=0;
	//m_AxisEncorder=0;
	//m_AxisCount=0;
	//m_AxisMove=false;
	//m_AxisDirection=false;//方向
	//int count=0;
	//SendCmd(TMCL_ROL,0,100000);//向右转速度
	//while(m_MotoWait)
	//{
	//	Sleep(1);
	//	if(count++>=5000)
	//		break;
	//}	
	//m_end=true;
	//while(m_end)
	//{
	//	SendCmd(TMCL_GAP,11,0);//获取限位
	//	if(count++>=10)
	//		break;
	//	Sleep(1000);
	//}
	//m_end=false;
	//m_MotorIns|=CMD_STOP;//先停后复位，这个非常重要！！！
	//m_MotorIns|=CMD_REST;
}

void CTmcAxis::SetSpeed( float p_Speed )
{
	//m_MotorIns|=CMD_SET_SPEED;//设置命令
	//m_AxisSpeed= p_Speed;

	char buf[64];
	int t_speed=p_Speed*m_AxisMm2Count;
	if (t_speed<=0)
		t_speed=6400;
	_proto.SetSpeed(_info.AxisName, t_speed, buf,64);
	DWORD BytesWritten;
	EnterCriticalSection(&_cs);
	WriteFile(m_AxisHandel, buf, strlen(buf),&BytesWritten,NULL);
	LeaveCriticalSection(&_cs);
	m_AxisSpeed= p_Speed;
}


//这个函数最基本
void CTmcAxis::MoveRef( float p_Offset,float p_Speed/*=-1*/ )
{
	float offset;
	offset=p_Offset;
	//设置位置
	if(m_AxisPositon + p_Offset < _info.sMinPos)
		offset = _info.sMinPos - m_AxisPositon;
	if(m_AxisPositon + p_Offset > _info.sMaxPos)
		offset = _info.sMaxPos - m_AxisPositon;
	if(offset == 0)
		return ;
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
	char buf[64];
	int offcount=offset*m_AxisMm2Count;
	_proto.Move(_info.AxisName, offcount, buf,64);//相对值移动
	m_AxisMove=true;
	DWORD BytesWritten;
	WriteFile(m_AxisHandel, buf, strlen(buf),&BytesWritten,NULL);
	LeaveCriticalSection(&_cs);

	//运动
	//int offcount=offset*m_AxisMm2Count;
	//m_AxisCount=(m_AxisPositon + p_Offset)*m_AxisMm2Count;//更新电机要走到的步长
	//m_MotorIns|=CMD_SET_POSITION;//移动
	//m_AxisMove=true;
	//LeaveCriticalSection(&_cs);
}

void CTmcAxis::MoveTo( float p_Pos,float p_Speed/*=-1*/ )
{
	MoveRef(p_Pos-m_AxisPositon,p_Speed);
}

void CTmcAxis::Move( float p_Speed/*=-1*/ )
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

bool CTmcAxis::IsMoveing()
{
	return m_AxisMove;
}

void CTmcAxis::Stop()
{
	char buf[64];
	_proto.Stop(_info.AxisName, buf,64);
	DWORD BytesWritten;
	EnterCriticalSection(&_cs);
	WriteFile(m_AxisHandel, buf, strlen(buf),&BytesWritten,NULL);
	LeaveCriticalSection(&_cs);

	//EnterCriticalSection(&_cs);
	//if(m_AxisMove)
	//{
	//	m_MotorIns|=CMD_STOP;
	//	//SendCmd(TMCL_MST,0,0);//停止
	//	Sleep(40); // 保证 Stop 命令同步完成
	//	_stopcount++;
	//	if(_stopcount > 1)
	//	{
	//		_stopcount = 0;
	//		m_AxisMove = false;
	//	}
	//}
	//LeaveCriticalSection(&_cs);
}

#pragma region EventForRecieve


void CTmcAxis::Event_GetPosition( int pos )
{
	m_AxisEncorder=pos;
	m_AxisPositon=pos*m_AxisEncorder2Mm;
	this->Notify(this,NOTIFY_AXIS_POSITION,'f',&m_AxisPositon);
}

void CTmcAxis::Event_Moved( int pos )
{
	m_AxisEncorder=pos;
	m_AxisPositon=pos*m_AxisEncorder2Mm;//转成当前位置....
	m_AxisMove=false;
	TRACE(L"Axis is Moved\n");
	this->Notify(this,NOTIFY_AXIS_MOVED,'f',&m_AxisPositon);
}

void CTmcAxis::Event_Reset()
{
	m_AxisPositon=0;
	m_AxisStatus=1;
	m_AxisMove=false;
	m_AxisEncorder=0;
}

void CTmcAxis::Event_IsMoving( bool moving )
{
	m_AxisMove=true;
}

#pragma endregion EventForRecieve

//向电机发消息
void CTmcAxis::SendCmd(UCHAR Command, UCHAR Type,INT Value )
{
	if(m_AxisHandel==INVALID_HANDLE_VALUE)
		return;
	EnterCriticalSection(&_cs);
	BYTE TxBuffer[9];
	DWORD BytesWritten;
	int i;
	TxBuffer[0]=m_MotorAddress;
	TxBuffer[1]=Command;
	TxBuffer[2]=Type;
	TxBuffer[3]=m_MotorIndex;		//这里设置了！！
	TxBuffer[4]=Value >> 24;
	TxBuffer[5]=Value >> 16;
	TxBuffer[6]=Value >> 8;
	TxBuffer[7]=Value & 0xff;
	TxBuffer[8]=0;
	for(i=0; i<8; i++)
		TxBuffer[8]+=TxBuffer[i];	
	WriteFile(m_AxisHandel, TxBuffer, 9, &BytesWritten, NULL);//Send the datagram
	LeaveCriticalSection(&_cs);
	m_MotoWait=true;//等结果---------
}

BYTE CTmcAxis::GetResult( UCHAR *Address, UCHAR *Status,BYTE *Cmds, int *Value )
{
	BYTE RxBuffer[9], Checksum;
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
		return TMCL_RESULT_NOT_READY;
	return TMCL_RESULT_OK;
}

//void CTmcAxis::OnHTimer( int nID )
//{
//	//单线程模式
//	BYTE trec;
//	BYTE Addrs,Status,Cmds;
//	int Values;
//	int Failure=0;
//	bool t_Ack=false;//等回复消息
//	while(m_bThredExit)
//	{
//		trec=GetResult(&Addrs,&Status, &Cmds, &Values);		
//		if(trec==TMCL_RESULT_OK)
//		{
//			m_MotoWait=false;//解锁
//			if(m_bSetting)//下面是设置模式，不解码
//			{
//				Sleep(3);
//				continue;
//			}
//			switch(Cmds)
//			{
//				case TMCL_MVP:	
//					if(Status==TMC_STATUS_OK)//返回100是成功
//						m_AxisMove=true;//说明正在移动
//					break;
//				case TMCL_MST:	
//					if(Status==TMC_STATUS_OK)//返回100是成功
//						m_AxisMove=false;//说明停止了
//					break;
//				case TMCL_GAP:	
//					if(Status==TMC_STATUS_OK)//返回100是成功
//					{
//						if(m_AxisCount==Values)//因为电机就是以这个计数的，所以不怕死
//							Event_Moved(Values);
//						if(m_end)
//						{
//							if(Values==1)
//								m_end=false;
//						}
//						Event_GetPosition(Values);
//					}
//					break;	
//				case 138://这个是设置达位置返回功能
//					if (Status==128)//这个是位置到达指令100是发送成功消息
//						Event_Moved(m_AxisCount);//
//					break;
//				}
//			TRACE(L"Notify%d,%d,%d\n",Status,Cmds,Values);
//		}
//		if(m_MotoWait)//保护结果
//		{
//			Sleep(3);
//			if(Failure++>100)
//				m_MotoWait=false;
//			continue;
//		}
//		Sleep(3);
//		Failure=0;
//		if(m_MotorIns&CMD_STOP)//停止移动消息
//		{
//			m_MotorIns&=~CMD_STOP;//清除标志
//			SendCmd(TMCL_MST,0,0);//停止消息用！
//			continue;
//		}
//		if(m_MotorIns&CMD_REST)//复位消息
//		{
//			m_MotorIns&=~CMD_REST;//清除标志
//			SendCmd(TMCL_SAP,ACTUAL_POS,0);//设置为0
//			continue;
//		}		
//		if(m_MotorIns&CMD_SET_SPEED)//设置速度消息
//		{
//			m_MotorIns&=~CMD_SET_SPEED;//清除标志
//			int t_speed=m_AxisSpeed*m_AxisMm2Count;
//			SendCmd(TMCL_SAP,MAX_SPEED,t_speed);//设置速度
//			continue;
//		}
//		if(m_MotorIns&CMD_SET_POSITION)//移动消息
//		{		
//			if(t_Ack)
//			{
//				m_MotorIns&=~CMD_SET_POSITION;//清除标志
//				if (_info.sbInvert)
//					SendCmd(TMCL_MVP,MVP_ABS,-m_AxisCount);//使用绝对位置
//				else
//					SendCmd(TMCL_MVP,MVP_ABS,m_AxisCount);
//				t_Ack=false;//已经发完消息了
//			}
//			else
//			{
//				SendCmd(138,0,1);//到位消息
//				t_Ack=true;
//			}
//			continue;
//		}
//		if(m_MotorIns&CMD_REQ_POSITION)//获取位置
//		{
//			m_MotorIns&=~CMD_REQ_POSITION;//清除标志
//			SendCmd(TMCL_GAP,ACTUAL_POS,0);//位置
//			continue;
//		}		
//	}
//}

void CTmcAxis::OnHTimer( int nID )
{
	COMSTAT stat;
	DWORD err;
	DWORD len;
	char buf[1024];
	DWORD mask;
	while(1)
	{
		mask = 0;
		if(_omask.hEvent == INVALID_HANDLE_VALUE)
			break;
		GetOverlappedResult(m_AxisHandel, &_omask, &len, TRUE);
		ClearCommError(m_AxisHandel, &err, &stat);
		if(stat.cbInQue > 0)
		{
			ReadFile(m_AxisHandel, buf, stat.cbInQue, &len, &_or);
			GetOverlappedResult(m_AxisHandel, &_or, &len, TRUE);
			Sleep(1); // 依次 Reset 各轴会失败
			_proto.DataArrival(buf, len);
			//if(_feed)_feed->DataArrival(buf, len);
		}
		else
			Sleep(3);
	}
}



