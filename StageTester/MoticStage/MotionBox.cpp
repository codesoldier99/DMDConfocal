

#include "StdAfx.h"
#include "MotionBox.h"
#include "MoticAxis.h"
#include "PIAxis.h"

CMotionBox::CMotionBox(void)
{

}

CMotionBox::~CMotionBox(void)
{
	UnInit();//卸载
}

bool CMotionBox::InitGearBox( CString p_Name,void* wParam, AxisInfo* pAxisInfo,int nAxisCnt )
{
	//这是变量区
	int comport=4;
	int iTurnPort=-1;
	//step1 初始化此模块
	m_GearBoxName=p_Name;	
	for (int i=0;i<8;i++)
		m_AxisX[i]=0;//轴信息
	//step2 开底层接口通信
	if(wParam!=0)///空的时候用默认参数
	{
		MoticStageParas mParas=*(MoticStageParas*)wParam;
		comport=mParas.iAxisCom;
		iTurnPort=mParas.iTurnCom;
		//comport=*(int*)wParam;
	}
	for(int i=0;i<nAxisCnt;i++)//step3 //初始化轴信息
	{	
		if(!m_MoticSerialCom.IsOpened())//这里要看控制盒有没有打开
		{
			m_MoticSerialCom.Open(comport,115200);//这里先默认
			Sleep(10);
			m_MoticSerialCom.SetFeed(&m_MoticProto);//反馈接口
		}	
		if (!m_TurnSerialCom.IsOpened()&&iTurnPort>0)
		{
			m_TurnSerialCom.Open(iTurnPort,115200);
			Sleep(10);
			m_TurnSerialCom.SetFeed(&m_TurnProto);
		}

		if (pAxisInfo[i].ID==AXIS_PIZ)
		{
			CPIAxis* mPIAxis=new CPIAxis();
			if (mPIAxis->Init(pAxisInfo[i]))
				m_AxisX[i]=mPIAxis;
			else
				m_AxisX[i]=0;
			continue;
		}

		if (pAxisInfo[i].ID==AXIS_T)
		{
			CMoticAxis* mMoticAxis=new CMoticAxis();
			if (m_TurnSerialCom.IsOpened())
			{
				mMoticAxis->InitAxis(&m_TurnSerialCom,&m_TurnProto,pAxisInfo[i]);	
				m_AxisX[i]=mMoticAxis;	
			}
			else
				delete mMoticAxis;
			continue;
		}

		CMoticAxis* mMoticAxis=new CMoticAxis();//新建轴
		if(m_MoticSerialCom.IsOpened()&&pAxisInfo[i].ID!=AXIS_PIZ)//初始化轴
		{
			mMoticAxis->InitAxis(&m_MoticSerialCom,&m_MoticProto,pAxisInfo[i]);	
			m_AxisX[i]=mMoticAxis;	
		}
		else
		{
			delete mMoticAxis;
			if (pAxisInfo[i].ID!=AXIS_PIZ)
				return false;	
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
	//关接口
	if(m_MoticSerialCom.IsOpened())
		m_MoticSerialCom.Close();
	//删除轴
	for (int i=0;i<8;i++)
	{
		if(m_AxisX[i])
		{
			m_AxisX[i]->UnInit();
			delete(m_AxisX[i]);
			m_AxisX[i] = 0;
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

