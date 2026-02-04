#include "StdAfx.h"
#include "GalilCom.h"
using namespace std;

CGalilCom::CGalilCom(void)
{
	m_Galil=0;	
	m_bThredExit=true;
	m_AxisA=0;
	m_AxisB=0;
	m_AxisC=0;
	InitializeCriticalSection(&_cs);
}


CGalilCom::~CGalilCom(void)
{
	m_bThredExit=false;//退出线程
	if(m_Galil)
	{
		delete(m_Galil);
		m_Galil=0;
	}
}

bool CGalilCom::InitCom( CString pParam )
{
	string cst_GalilName="";
	if (pParam==L"")
		cst_GalilName="10.0.0.100";
	else 
		cst_GalilName=(LPCSTR)CStringA(pParam);
	try
	{
		m_Galil=new Galil("10.0.0.100");
	}
	catch (string e)
	{
		m_Galil=0;	
		TRACE(L"Sorry Can't Connect to Galil\n");
		return false;
	}

#if CTRACE


#else
	//下载程序
	try
	{
		//m_Galil->programDownloadFile(".\\GALIL.DMC");
	}
	catch (string e)
	{
		TRACE(L"Sorry Can't DownLoad the Programs to Galil\n");
	}
#endif	
	//建线程。。。
	m_ReadProcess.ChangeMode(false);
	m_ReadProcess.SetTimer(this,529,500);
	return true;
}

bool CGalilCom::SendAxisCmd( char pAxis,CString pCmdStr )
{
	EnterCriticalSection(&_cs);
	std::string	t_Cmd=(LPCSTR)CStringA(pCmdStr);
	t_Cmd+=pAxis;
	try
	{
		
#if CTRACE
		TRACE(t_Cmd.c_str());
		TRACE(" \n");
#else
		m_Galil->command(t_Cmd);
#endif
	}
	catch(string e)
	{
		LeaveCriticalSection(&_cs);
		return false;
	}
	LeaveCriticalSection(&_cs);
	return TRUE;
}

bool CGalilCom::SendAxisCmd(char pAxis, CString pCmdStr,int pCmdValue )
{
	EnterCriticalSection(&_cs);
	CString t_value;
	if(pCmdValue>=0)
		t_value.Format(L"=%d",pCmdValue);//值  140218调试后改进的
	else
	{
		pCmdValue=-pCmdValue;
		t_value.Format(L"=-%d",pCmdValue);
	}
	std::string	t_Cmd=(LPCSTR)CStringA(pCmdStr);
	std::string t_STValue=(LPCSTR)CStringA(t_value);
	t_Cmd+=pAxis;//加轴信息
	t_Cmd+=t_STValue;//加值

	try
	{
#if CTRACE
		TRACE(t_Cmd.c_str());
		TRACE(" \n");
#else
		m_Galil->command(t_Cmd);
		//TRACE(t_Cmd.c_str());
		//TRACE(" \n");
#endif
	}
	catch(string e)
	{
		TRACE(L"Command Erro\n");
		LeaveCriticalSection(&_cs);
		return FALSE;
	}
	LeaveCriticalSection(&_cs);
	return TRUE;
}

bool CGalilCom::SendCmd( CString pCmdStr )
{
	EnterCriticalSection(&_cs);
	std::string	t_Cmd=(LPCSTR)CStringA(pCmdStr);
	try
	{
#if CTRACE
		TRACE(t_Cmd.c_str());
		TRACE(" \n");
#else
		m_Galil->command(t_Cmd);
#endif
	}
	catch(string e)
	{
		TRACE(L"Command Erro\n");
		LeaveCriticalSection(&_cs);
		return FALSE;
	}
	LeaveCriticalSection(&_cs);
	return TRUE;
}

bool CGalilCom::GetAxisValue( char pAxis,CString pCmdStr,double& pValue )
{
	EnterCriticalSection(&_cs);
	double tRec;
	std::string tempName=(LPCSTR)CStringA(pCmdStr+pAxis);
	try
	{
#if CTRACE
		TRACE(tempName.c_str());
		TRACE(" \n");
#else
		pValue=m_Galil->commandValue(tempName);
#endif
	}
	catch (string e)
	{
		TRACE(L"Command Erro\n");
		LeaveCriticalSection(&_cs);
		return false;
	}
	LeaveCriticalSection(&_cs);
	return true;
}

bool CGalilCom::AxisMove( char pAxis,int pPos )
{
	if(pAxis=='A')
	{
		SendAxisCmd('3',L"HX");
		SendAxisCmd('A',L"ST");
		SendAxisCmd('A',L"POSIT",pPos);
		SendCmd(L"XQ#MOVEA,3");
	}
	else if(pAxis=='B')
	{
		SendAxisCmd('4',L"HX");
		SendAxisCmd('B',L"ST");
		SendAxisCmd('B',L"POSIT",pPos);
		SendCmd(L"XQ#MOVEB,4");//step 2 运行程序
	}
	else if (pAxis=='C')
	{
		SendAxisCmd('5',L"HX");
		SendAxisCmd('C',L"ST");
		SendAxisCmd('C',L"POSIT",pPos);
		SendCmd(L"XQ#MOVEC,5");//step 2 运行程序
	}
	return true;
}

void CGalilCom::OnHTimer( int nID )
{
	//int iCount=0;
	while(m_bThredExit)
	{
		string t_Msg;
#if CTRACE
		
#else
		try
		{
			t_Msg=m_Galil->message(1000);
			//iCount=0;
		}
		catch(string e)
		{
			/*iCount++;
			if (iCount>10)
			{
			AfxMessageBox(L"请点击这个窗口，如果连续出现，请联系我！");
			}*/
		}
		
#endif
		CString t_MsgCS(t_Msg.data());//转成csting 
		if(t_MsgCS==L"AMOVD\r\n")
		{
			if(m_AxisA)
				m_AxisA->Event_Moved(-1);
			//TRACE(L"AMoved\n");
		}
		if(t_MsgCS==L"BMOVD\r\n")
		{
			if(m_AxisB)
				m_AxisB->Event_Moved(-1);
			//TRACE(L"BMoved\n");
		}
		if(t_MsgCS==L"CMOVD\r\n")
		{
			if(m_AxisC)
				m_AxisC->Event_Moved(-1);
			//TRACE(L"BMoved\n");
		}
	}
}

void CGalilCom::SetAxisHook( char pAxisName,AxisHook* pAxis )
{
	if(pAxisName=='A')
		m_AxisA=pAxis;
	if(pAxisName=='B')
		m_AxisB=pAxis;
	if(pAxisName=='C')
		m_AxisC=pAxis;
}
