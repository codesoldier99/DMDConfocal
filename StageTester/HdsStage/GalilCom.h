#pragma once

#include "Galil.h"
#include "HTimer.h"
#include "HGearBox.h"

#define CTRACE 0

class AxisHook
{
public:
	virtual void Event_CommTest()=0;
	virtual void Event_GetPosition(int pos)=0;
	virtual void Event_Moved(int pos)=0;
	virtual void Event_Error()=0;
	virtual void Event_Reset()=0;
	virtual void Event_IsMoving(bool moving)=0;
	//virtual void Event_GetEncorder(int pos)=0;
};

class CGalilCom:public HTimerHook
{

public:
	CGalilCom(void);
	~CGalilCom(void);
	bool InitCom(CString pParam=L"");
	bool AxisMove(char pAxis,int pPos);
	bool SendAxisCmd(char pAxis,CString pCmdStr);
	bool SendAxisCmd(char pAxis,CString pCmdStr,int pCmdValue);
	bool GetAxisValue(char pAxis,CString pCmdStr,double& pValue);
	bool SendCmd(CString pCmdStr);
	virtual void OnHTimer(int nID);
	void SetAxisHook(char pAxisName,AxisHook* pAxis);
protected:
	Galil* m_Galil;
	CRITICAL_SECTION _cs;
	HTimer m_ReadProcess;
	bool m_bThredExit;
	AxisHook* m_AxisA,*m_AxisB,*m_AxisC;
};

