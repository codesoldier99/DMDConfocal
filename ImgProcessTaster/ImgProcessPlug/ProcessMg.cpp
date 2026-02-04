#include "StdAfx.h"
#include "ProcessMg.h"
#include "ReBuild.h"
#include "ColorBuild.h"
#include "ImgQuality.h"
#include "MapBuild.h"
#include "GrayConvert.h"
#include "ScaleConvert.h"
#include "GamaConvert.h"
#include "BlockScan.h"
#include "EasyFunction.h"
#include "CutConvert.h"
#include "CalImgValue.h"
#include "AutoWB.h"
#include "FilterConvert.h"
#include "ImgDAC.h"

CProcessMg::CProcessMg(void)
{
}


CProcessMg::~CProcessMg(void)
{
}

HCoreProcess* CProcessMg::GetProcessCore(PROCESS_TYPE mType)
{
	HCoreProcess* p=0;
	switch(mType)
	{
	case PROCESS_3DReBuild:
		p=new CReBuild();	
		break;
	case PROCESS_ColorBuild:
		p=new CColorBuild();	
		break;
	case PROCESS_MapBuild:
		p=new CMapBuild();
		break;
	case PROCESS_ImgQuality:
		p=new CImgQuality();
		break;
	case PROCESS_CalValue:
		p=new CCalImgValue();
		break;
	case PROCESS_DACAF:
		p = new CImgDAC();
		break;
	default:
		break;
	}
	return p;
}

HImgPreProcess* CProcessMg::GetImgPreProcess(CString strType)
{
	HImgPreProcess* p=0;
	if (strType==L"GrayConvert")
	{
		p=new CGrayConvert();
		return p;
	}
	else if (strType==L"ScaleConvert")
	{
		p=new CScaleConvert();
		return p;
	}
	else if (strType==L"GamaConvert")
	{
		p=new CGamaConvert();
		return p;
	}
	else if (strType==L"CutConvert")
	{
		p=new CCutConvert();
		return p;
	}
	else if (strType == L"FilterConvert")
	{
		p = new CFilterConvert();
		return p;
	}
	return 0;
}

HImgProcess* CProcessMg::GetImgProcess(CString strType)
{
	HImgProcess* p=0;
	if (strType==L"BlockScan")
	{
		p=new CBlockScan();
		return p;
	}
	else if (strType==L"AutoWB")
	{
		p=new AutoWB();
		return p;
	}
	return 0;
}

HEasyFunction* CProcessMg::GetFunction()
{
	HEasyFunction* p=new CEasyFunction();
	if (p)
		return p;
	return 0;
}