#include "StdAfx.h"
#include "PIAxis.h"


CPIAxis::CPIAxis(void)
{
	m_iControllerId=-1;
	m_AxisPositon=0;
}


CPIAxis::~CPIAxis(void)
{
}

bool CPIAxis::Init(AxisInfo p_Axis)
{
	_info=p_Axis;
	m_AxisMm2Count=p_Axis.sMm2Count;
	m_AxisCount2Mm=1.0f/m_AxisMm2Count;
	m_AxisMm2Encorder=p_Axis.sMm2Encoder;
	m_AxisEncorder2Mm=1.0f/m_AxisMm2Encorder;

	if(m_iControllerId>=0)
		return false;
	char* mName=new char[100];
	PI_EnumerateUSB(mName,100,NULL);
	m_iControllerId = -1;
	m_iControllerId = PI_ConnectUSB(mName);
	if(m_iControllerId<0)	
		return false;
	if(!PI_qSAI(m_iControllerId, mName, 100))
		return false;
	// Switch on the Servo
	BOOL bFlag = TRUE;
	if(!PI_SVO(m_iControllerId, mName,&bFlag))
		return false;
	m_AxisPositon=0;
	std::string mNam3="z";
	if(!PI_MOV(m_iControllerId,mNam3.c_str(),&m_AxisPositon))
		return false;
	return true;
}

bool CPIAxis::UnInit()
{
	PI_CloseConnection(m_iControllerId);
	return true;
}

void CPIAxis::Rest(void)
{
	m_AxisPositon=0;
	std::string mNam3="z";
	PI_MOV(m_iControllerId,mNam3.c_str(),&m_AxisPositon);
}

void CPIAxis::MoveTo(float p_Pos,float p_Speed)
{
	double mCount=p_Pos*m_AxisMm2Count;

	if (mCount>400)
		mCount=400;
	if (mCount<0)
		mCount=0;

	std::string mNam3="z";
	PI_MOV(m_iControllerId,mNam3.c_str(),&mCount);
	// Wait until the closed loop move is done.
	//BOOL bIsMoving = TRUE;
	//double mVPos;
	//while(bIsMoving == TRUE)
	//{
	//	//if(!PI_qPOS(m_iControllerId, mNam3.c_str(), &m_AxisPositon))
	//	//	return;
	//	//if(!PI_IsMoving(m_iControllerId, mNam3.c_str(), &bIsMoving))
	//	//	return;
	//	//TRACE(L"Pos: %g\n",m_AxisPositon);
	//	if(!PI_qPOS(m_iControllerId, mNam3.c_str(), &mVPos))
	//		return;
	//	TRACE(L"Pos: %g\n",mVPos);
	//	if (mVPos==m_AxisPositon)
	//		break;	
	//}
	TRACE(L"Finish Moving\n");
	Sleep(100);
	if(PI_qPOS(m_iControllerId, mNam3.c_str(), &mCount))
		TRACE(L"PI Pos: %g um\n",mCount);
	m_AxisPositon=mCount*m_AxisCount2Mm;
}

float CPIAxis::GetPosition()
{
	double mCount;
	std::string mNam3="z";
	if(PI_qPOS(m_iControllerId, mNam3.c_str(), &mCount))
		TRACE(L"PI Pos: %g um\n",mCount);
	m_AxisPositon=mCount*m_AxisCount2Mm;
	return m_AxisPositon;
}

void CPIAxis::MoveRef(float p_Offset,float p_Speed)
{
	if (p_Offset==0)
	{
		GetPosition();
		return;
	}		
	MoveTo(m_AxisPositon+p_Offset,p_Speed);
}

bool CPIAxis::IsConnect()
{
	BOOL bV=PI_IsConnected(m_iControllerId);
	return bV;
}
