
#include "StdAfx.h"
#include "HTimer.h"

HTimer::HTimer()
{
	m_Elapse=10;//，默认10ms

	m_bContinue=true;//延续模式

	m_bTaskReady=false;//线程状态
	
	m_hUpdateEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

// 	ResetEvent(m_hUpdateEvent);//这时暂不动作
// 	m_hThread = CreateThread(NULL, 0, HTimerProThread, this, 0, 0);

	for (int i=0;i<MaxTimeTable;i++)
	{
		m_TimerTable[i].nID=0;
		m_TimerTable[i].bEnable=false;
		m_TimerTable[i].nCountTick=0;
	}
}

HTimer::~HTimer()
{

	for (int i=0;i<MaxTimeTable;i++)
	{
		m_TimerTable[i].bEnable=false;
	}
	if(m_bTaskReady)
	{
		SetEvent(m_hUpdateEvent);// 退出线程
		WaitForSingleObject(m_hThread, INFINITE);
	}

}

void HTimer::ChangeMode(bool p_Continue)
{
	m_bContinue=p_Continue;

	if(!p_Continue)// 一次性模式
	{
		
		if(m_bTaskReady)
		{
			//退了线程先。。。
			SetEvent(m_hUpdateEvent);
			WaitForSingleObject(m_hThread, INFINITE);
		}

	}
	else//重复模式
	{
		NewTask();//建线程
	}
}

void HTimer::SetTimer(HTimerHook* pHook, int nID,int nElapse )
{
	m_HTimer=pHook;
	
	for(int i=0;i<MaxTimeTable;i++)//查询
	{
		if(m_TimerTable[i].nID==nID)
		{
			m_TimerTable[i].nElapse=nElapse;
			m_TimerTable[i].nCountTick=nElapse/m_Elapse;
			m_TimerTable[i].nNowTick=0;
			m_TimerTable[i].bEnable=true;

			NewTask();//新建线程

			return;
		}
		
	}

	for(int i=0;i<MaxTimeTable;i++)//新建
	{
		if(m_TimerTable[i].bEnable==false)
		{
			m_TimerTable[i].nID=nID;
			m_TimerTable[i].nElapse=nElapse;
			m_TimerTable[i].nCountTick=nElapse/m_Elapse;
			m_TimerTable[i].nNowTick=0;
			m_TimerTable[i].bEnable=true;

			NewTask();//新建线程

			return;
		}
	}

}

void HTimer::NewTask()
{
	if(m_bTaskReady)
		return;//已经有线程了

	ResetEvent(m_hUpdateEvent);
	m_hThread = CreateThread(NULL, 0, HTimerProThread, this, 0, 0);

	m_bTaskReady=true;
}

void HTimer::KillTimer( int nID )
{
	for(int i=0;i<MaxTimeTable;i++)
	{
		if(m_TimerTable[i].nID==nID)
		{
			m_TimerTable[i].bEnable=false;
			return;
		}
	}
}

void HTimer::ThreadProcess( void )
{
	while(WaitForSingleObject(m_hUpdateEvent,m_Elapse)!= WAIT_OBJECT_0)
	{

		for(int i=0;i<MaxTimeTable;i++)
		{
			if(m_TimerTable[i].bEnable==true)
			{
				if(m_TimerTable[i].nCountTick<m_TimerTable[i].nNowTick++)
				{
					m_TimerTable[i].nNowTick=0;

					m_HTimer->OnHTimer(m_TimerTable[i].nID);//输出

					if(!m_bContinue)//退出线程吧
					{
						SetEvent(m_hUpdateEvent);//退出事件
						break;
					}
				}
			
			}
		}

	}


	m_bTaskReady=false;//退出线程标志
	return;
}

DWORD WINAPI HTimerProThread( LPVOID lp )
{
	((HTimer*)lp)->ThreadProcess();
	
	return 0;
}
