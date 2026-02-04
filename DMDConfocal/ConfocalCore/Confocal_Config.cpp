#include "stdafx.h"
#include "Confocal_Config.h"
#include <shlwapi.h>

Confocal_Config::Confocal_Config(void)
{
	Clear();
}

Confocal_Config::~Confocal_Config(void)
{

}

void Confocal_Config::Clear()
{
	m_deviceType = -1;
	m_uiResourceLibFileName = "";
	m_nLangId = -1;
	m_nToolbarStyle = 0;
	m_strPlash.Empty();
}

void Confocal_Config::Load(CString pConfigFile)
{
	Clear();
	m_strPlash=m_ConfigureFileLib.GetModulePath();
	m_strPlash+=L"\\"+m_ConfigureFileLib.GetString(L"Common",L"Splash",pConfigFile);

	//m_deviceType = GetPrivateProfileInt("Common","device",m_deviceType,pConfigPath);
	//m_nToolbarStyle = GetPrivateProfileInt("Common", "ToolbarStyle", m_nToolbarStyle, pConfigPath);
	//char buf[128];
	//memset(buf,0,128);
	//GetPrivateProfileString("Common","Splash","",buf,128,pConfigPath);
	//m_strPlash = pConfigPath;
	//m_strPlash = m_strPlash.Left(m_strPlash.ReverseFind('\\') + 1);
	//m_strPlash += buf;
	//memset(buf,0,128);
	//GetPrivateProfileString("Common","UIResourceLib","",buf,128,pConfigPath);
	//m_uiResourceLibFileName = buf;
	//m_nLangId = GetPrivateProfileInt("Language","ID",m_nLangId,pConfigPath);
	//
	//// 读取应用程序的图标路径和标题
	//strcpy_s(buf, _countof(buf), "");
	//GetPrivateProfileString("Common", "AppIconPath", "", buf, _countof(buf), pConfigPath);
	//appicon_path = buf;
	//strcpy_s(buf, _countof(buf), "");
	//GetPrivateProfileString("Common", "AppTitle", "", buf, _countof(buf), pConfigPath);
	//apptitle = buf;
	//apptitle.Trim();
	//GetPrivateProfileString("Common", "QSGPath", "", buf, _countof(buf), pConfigPath);
	//_qsgPath = buf;
}

void Confocal_Config::Save(CString pConfigPath)
{
	//CString str;
	//str.Format("%d",m_deviceType);

	//WritePrivateProfileString("Common","device",str,pConfigPath);
	//WritePrivateProfileString("Common","UIResourceLib",m_uiResourceLibFileName,pConfigPath);
	//str.Format("%d", m_nToolbarStyle);
	//WritePrivateProfileString("Common", "ToolbarStyle", str, pConfigPath);
	//m_strPlash = m_strPlash.Right(m_strPlash.GetLength() - m_strPlash.ReverseFind('\\') - 1);
	//WritePrivateProfileString("Common", "Splash", m_strPlash, pConfigPath);
	//str.Format("%d",m_nLangId);
	//WritePrivateProfileString("Language","ID",str,pConfigPath);

	//char buf[128];
	//strcpy_s(buf, _countof(buf), "");
	//int len = GetPrivateProfileString("Common", "AppIconPath", "", buf, _countof(buf), pConfigPath);
	//if(len == 0)
	//{
	//	WritePrivateProfileString("Common", "AppIconPath", "", pConfigPath);
	//}
	//strcpy_s(buf, _countof(buf), "");
	//len = GetPrivateProfileString("Common", "AppTitle", "", buf, _countof(buf), pConfigPath);
	//if(len == 0)
	//{
	//	WritePrivateProfileString("Common", "AppTitle", "", pConfigPath);
	//}
	//strcpy_s(buf, _countof(buf), "");
	//len = GetPrivateProfileString("Common", "QSGPath", "", buf, _countof(buf), pConfigPath);
	//if(len == 0)
	//{
	//	WritePrivateProfileString("Common", "QSGPath", "", pConfigPath);
	//}
}