#pragma once
#include <afx.h>
#include "ptnSingleton.h"
#include "HConfFile.h"

//using namespace marsCore;

class Confocal_Config
{

public:
	int m_deviceType;//-1:none; 0:PCI Controller; 1:BioDissection Machine
	CString m_uiResourceLibFileName;	

	int m_nLangId;
	int m_nToolbarStyle;

	CString m_strPlash;//启动画面初始化图像
	
	CString appicon_path;//应用程序图标的路径
	CString apptitle;//应用程序的标题
	CString _qsgPath;//QSG帮助路径

	HConfigure	m_ConfigureFileLib;//配置文件

	Confocal_Config(void);
	~Confocal_Config(void);
    void Load(CString pConfigFile);
	void Save(CString pConfigFile);
	void Clear();
};

class Confocal_ConfigSingleton:public ptnSingleton<Confocal_Config>
{

};