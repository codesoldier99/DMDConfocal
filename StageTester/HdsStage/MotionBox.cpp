

#include "StdAfx.h"
#include "MotionBox.h"
#include "TmcAxis.h"
#include "GaliaAxis.h"
#include "WAxis.h"
//#include "MoticAxis.h"

CMotionBox::CMotionBox(void)
{
	m_TMC_SerialCom=INVALID_HANDLE_VALUE;
	m_WAxis_SerialCom=INVALID_HANDLE_VALUE;
}


CMotionBox::~CMotionBox(void)
{
	UnInit();//卸载
}

bool CMotionBox::InitGearBox( CString p_Name,void* wParam, AxisInfo* pAxisInfo,int nAxisCnt )
{
	//这是变量区
	std::string t_GalilName;
	CTmcAxis*	t_TmcAxis;
	CWAxis* t_WAxis;
	HdsStageParas mHdsParas;
	mHdsParas.iWCom=1;
	mHdsParas.iZCom=4;
	mHdsParas.strIP=L"10.0.0.100";
	m_GearBoxName=p_Name;
	for (int i=0;i<8;i++)
		m_AxisX[i]=0;

	if(wParam!=0)///空的时候用默认参数
		mHdsParas=*(HdsStageParas*)wParam;

	if(!m_GalilCom.InitCom(mHdsParas.strIP))
		return false;
	for(int i=0;i<nAxisCnt;i++)//step3 //初始化轴信息
	{	
		if(pAxisInfo[i].ID==AXIS_Z&&mHdsParas.iZCom>0)//设置TMC轴
		{
			if(m_TMC_SerialCom==INVALID_HANDLE_VALUE)
				m_TMC_SerialCom=OpenRS232(mHdsParas.iZCom,115200);//9600);//设置口和波特率
			t_TmcAxis=new CTmcAxis();
			if(m_TMC_SerialCom!=INVALID_HANDLE_VALUE)
			{		
				int motoraddr=1;//记得电机地址
				t_TmcAxis->InitAxis(m_TMC_SerialCom,motoraddr,pAxisInfo[i]);
				m_AxisX[i]=t_TmcAxis;//存入数组中
			}
		}
		else if(pAxisInfo[i].ID==AXIS_W)
		{
			if(m_WAxis_SerialCom==INVALID_HANDLE_VALUE)
			{
				m_WAxis_SerialCom=OpenRS232(mHdsParas.iWCom,9600);//设置口和波特率--2口
				TRACE("Sorry 232正在");
			}
			t_WAxis=new CWAxis();
			TRACE("准备打开com1");
			if(m_WAxis_SerialCom!=INVALID_HANDLE_VALUE)
			{		
				int motoraddr=1;//记得电机地址
				t_WAxis->InitAxis(m_WAxis_SerialCom,motoraddr,pAxisInfo[i]);
				m_AxisX[i]=t_WAxis;//存入数组中
				TRACE("打开成功");
			}
		}
		else
		{
			CGaliaAxis* t_Axis=new CGaliaAxis();
			t_Axis->InitAxis(&m_GalilCom,pAxisInfo[i]);
			m_AxisX[i]=t_Axis;//存入数组中
		}	
	}
	return true;
}

HAxis* CMotionBox::GetAxis( AxisID p_ID)
{
	for(int i=0;i<8;i++)
	{
		if(!m_AxisX[i])
			continue;
		if(m_AxisX[i]->GetAxialInfo()->ID==p_ID)
			return m_AxisX[i];
	}
	return 0;
}

HAxis* CMotionBox::GetAxis( int p_Index )
{
	for(int i=0;i<8;i++)
	{
		if(m_AxisX[i])
		{
			if(i==p_Index)
				return (m_AxisX[i]);
		}
	}
	return 0;
}

bool CMotionBox::UnInit()
{
	if(m_TMC_SerialCom!=INVALID_HANDLE_VALUE)	//  [2/13/2014 confocal]
		CloseHandle(m_TMC_SerialCom);//---------
	m_TMC_SerialCom=INVALID_HANDLE_VALUE;
	if(m_WAxis_SerialCom!=INVALID_HANDLE_VALUE)	//  [2/13/2014 confocal]
		CloseHandle(m_WAxis_SerialCom);//---------
	m_WAxis_SerialCom=INVALID_HANDLE_VALUE;	
	for (int i=0;i<8;i++)//删除轴
	{
		if(m_AxisX[i])
		{
			m_AxisX[i]->UnInit();
			delete(m_AxisX[i]);
		}		
	}
	return true;
}

int CMotionBox::IsConnect()
{
	int rec=0;
	for(int i=0;i<8;i++)
	{
		if(m_AxisX[i])
		{
			if(m_AxisX[i]->IsConnect())
			{
				rec+=1<<i;
			}
		}
	}
	return rec;
}

HANDLE CMotionBox::OpenRS232( int pPort, int BaudRate )
{
	HANDLE ComHandle;
	DCB CommDCB;
	COMMTIMEOUTS CommTimeouts;
	CString port;
	port.Format(L"\\\\.\\COM%d",pPort);
	ComHandle=CreateFile(port, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(GetLastError()!=ERROR_SUCCESS) 
		return INVALID_HANDLE_VALUE;
	else
	{
		GetCommState(ComHandle, &CommDCB);
		CommDCB.BaudRate=BaudRate;
		CommDCB.Parity=NOPARITY;
		CommDCB.StopBits=ONESTOPBIT;
		CommDCB.ByteSize=8;
		CommDCB.fBinary=1;  //Binary Mode only
		CommDCB.fParity=0;
		CommDCB.fOutxCtsFlow=0;
		CommDCB.fOutxDsrFlow=0;
		CommDCB.fDtrControl=0;
		CommDCB.fDsrSensitivity=0;
		CommDCB.fTXContinueOnXoff=0;
		CommDCB.fOutX=0;
		CommDCB.fInX=0;
		CommDCB.fErrorChar=0;
		CommDCB.fNull=0;
		CommDCB.fRtsControl=RTS_CONTROL_TOGGLE;
		CommDCB.fAbortOnError=0;
		SetCommState(ComHandle, &CommDCB);	
		SetupComm(ComHandle, 100, 100);//Set buffer size		
		GetCommTimeouts(ComHandle, &CommTimeouts);//Set up timeout values (very important, as otherwise the program will be very slow)
		CommTimeouts.ReadIntervalTimeout=MAXDWORD;
		CommTimeouts.ReadTotalTimeoutMultiplier=0;
		CommTimeouts.ReadTotalTimeoutConstant=0;
		SetCommTimeouts(ComHandle, &CommTimeouts);
		return ComHandle;
	}
}


