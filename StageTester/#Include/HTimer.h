
#pragma once

class HTimerHook
{
public:
	virtual void OnHTimer(int nID)=0;
};

class HTimer
{

public:
	HTimer();
	~HTimer();
	void SetTimer(HTimerHook* pHook,int nID,int nElapse);//最大10个
	void KillTimer(int nID);
	void SetPricial(int nElapse);//设置精度，默认10ms
	void ChangeMode(bool p_Continue);//设置是否延续模式，默认为是
	void NewTask();//新开线程
public:
	typedef struct
	{
		int nID;
		int nElapse;
		bool bEnable;
		int nCountTick;
		int nNowTick;
	}TimerTable;

protected:

	HTimerHook* m_HTimer;
	void ThreadProcess(void);

private:

	friend DWORD WINAPI HTimerProThread(LPVOID lp);
	HANDLE m_hThread;
	HANDLE m_hUpdateEvent;
	int m_Elapse;
	bool m_bContinue;//是否重复运行的标志
	bool m_bTaskReady;//线程是否正运行
#define  MaxTimeTable 10
	TimerTable m_TimerTable[MaxTimeTable];//多个需要使用的，线程轮询它
};