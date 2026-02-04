#pragma once
#include "HCoreProcess.h"

class CProcessMg:public HCoreProcessMg
{
public:
	CProcessMg(void);
	~CProcessMg(void);
	HCoreProcess* GetProcessCore(PROCESS_TYPE mType);

	HImgPreProcess* GetImgPreProcess(CString strType);
	HImgProcess* GetImgProcess(CString strType);

	HEasyFunction* GetFunction();
};

