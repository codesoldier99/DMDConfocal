

#pragma once

#include "stdafx.h"

class HConfigure
{
public:
	HConfigure(void)
	{
		m_FileName=GetModulePath()+L"\\Configure\\";//得到当前路径
	}

	CString GetModulePath()
	{
		HMODULE t_mudule=GetModuleHandle(0);
		CString pfileName;
		GetModuleFileName(t_mudule,pfileName.GetBufferSetLength(MAX_PATH),MAX_PATH);
		pfileName.ReleaseBuffer();

		int nPos=pfileName.ReverseFind('\\');
		if(nPos<0)
			return L"";
		else
			return pfileName.Left(nPos);
	}

	CString GetWorkPath()
	{
		CString pFileName;
		int nPos=GetCurrentDirectory(MAX_PATH,pFileName.GetBufferSetLength(MAX_PATH));
		if(nPos<0)
			return L"";
		else
			return pFileName;
	}

	CString GetString(CString p_Section,CString p_Name,CString pModule)
	{
		CString t_Value;
		CString t_FileName=m_FileName;
		pModule+=L".ini";
		t_FileName+=pModule;
		//::GetPrivateProfileInt(p_Section,p_Name,30,_T("./Confocal.ini"));
		::GetPrivateProfileString(p_Section,p_Name,L"",t_Value.GetBufferSetLength(200),200,t_FileName);
		t_Value.ReleaseBuffer();//记得释放
		return t_Value;
	}

	int GetInt(CString p_Section,CString p_Name,CString pModule)
	{
		CString t_Value;
		CString t_FileName=m_FileName;//L".\\Configure\\";

		pModule+=L".ini";
		t_FileName+=pModule;

		int t_value;
		t_value=::GetPrivateProfileIntW(p_Section,p_Name,-1,t_FileName);

		return t_value;
	}

	bool GetBool(CString p_Section,CString p_Name,CString pModule)
	{
		int mV=GetInt(p_Section,p_Name,pModule);
		bool mbV=mV>0?true:false;
		return mbV;
	}

	double GetDouble(CString p_Section,CString p_Name,CString pModule)
	{
		CString t_Value;
		CString t_FileName=m_FileName;//L".\\Configure\\";

		pModule+=L".ini";
		t_FileName+=pModule;

		::GetPrivateProfileString(p_Section,p_Name,L"-1",t_Value.GetBufferSetLength(20),20,t_FileName);
		t_Value.ReleaseBuffer();//记得释放

		double t_return=_wtof(t_Value);

		return t_return;
	}

	void RecValue(CString p_Section,CString p_Name,int p_Value,CString pModule)
	{
		CString t_FileName=m_FileName;//L".\\Configure\\";		

		pModule+=L".ini";
		t_FileName+=pModule;
		CString t_Value;
		t_Value.Format(L"%d",p_Value);
		::WritePrivateProfileString(p_Section,p_Name,t_Value,t_FileName);
	}


	void RecValue(CString p_Section,CString p_Name,CString p_Value,CString pModule)
	{
		CString t_FileName=m_FileName;//L".\\Configure\\";

		pModule+=L".ini";
		t_FileName+=pModule;

		::WritePrivateProfileString(p_Section,p_Name,p_Value,t_FileName);

	}


	void RecValue(CString p_Section,CString p_Name,double p_Value,CString pModule)
	{
		CString t_FileName=m_FileName;//L".\\Configure\\";CString t_FileName=L".\\Configure\\";

		pModule+=L".ini";
		t_FileName+=pModule;


		CString t_Value;
		t_Value.Format(L"%.4f",p_Value);
		::WritePrivateProfileString(p_Section,p_Name,t_Value,t_FileName);
	}

private:

	CString m_FileName;

protected:


};



