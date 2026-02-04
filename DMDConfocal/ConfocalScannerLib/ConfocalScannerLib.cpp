// ConfocalScannerLib.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "ConfocalScannerLib.h"
#include "MapScanner.h"
#include "ConfocalMerge.h"
#include "C3DScanner.h"
#include "ColorSacnner.h"
#include "AutoFocus.h"
#include "AutoFocusEx.h"
#include "ContinueScan.h"
#include "StructLightReconstruct.h"
#include "DiffMeasure.h"
#include "DiffMeasurePro.h"
#include "AutoFocusDAC.h"
#include "C3DCapture.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: 更改错误代码以符合您的需要
			_tprintf(_T("错误: MFC 初始化失败\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: 在此处为应用程序的行为编写代码。
		}
	}
	else
	{
		// TODO: 更改错误代码以符合您的需要
		_tprintf(_T("错误: GetModuleHandle 失败\n"));
		nRetCode = 1;
	}

	return nRetCode;
}

CConfocalScannerLib::CConfocalScannerLib( void )
{

}

HConfocalPlug* CConfocalScannerLib::GetPlugin(PLUGIN_TYPE pType)//int p_Index)
{
	HConfocalPlug* pPlugin;
	switch(pType)
	{
		case PLUGIN_MAP:
			pPlugin=new CMapScanner();	
			break;
		case PLUGIN_3DScanner:
			pPlugin=new C3DScanner();
			break;
		case PLUGIN_AutoFocus:
			pPlugin=new CAutoFocus();
			break;
		case PLUGIN_AutoFocusEx:
			pPlugin=new CAutoFocusEx();
			break;
		case PLUGIN_ColorScanner:
			pPlugin=new CColorSacnner();
			break;
		case PLUGIN_CONFOCALMODE:
			pPlugin=new CConfocalMerge();
			break;
		case PLUGIN_CONFOCALMODEEX:
			pPlugin = new CConfocalMerge();
			break;
		case PLUGIN_ContinueScan:
			pPlugin=new ContinueScan();
			break;
		case PLUGIN_SIMODE:
			pPlugin = new StructLightReconstruct();
			break;
		case PLUGIN_DIFFMEASURE:
			pPlugin = new DiffMeasure();
			break;
		case PLUGIN_DIFFMEASUREPRO:
			pPlugin = new DiffMeasurePro();
			break;
		case PLUGIN_AutoFocusDAC:
			pPlugin = new CAutoFocusDAC();
			break;
		case PLUGIN_3DCapture:
			pPlugin = new C3DCapture();
			break;
	}
	return pPlugin;

}

