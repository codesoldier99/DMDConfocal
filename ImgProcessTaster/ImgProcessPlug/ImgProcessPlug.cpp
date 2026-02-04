// ImgProcessPlug.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "ImgProcessPlug.h"
#include "ProcessMg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern "C" IMGPROCESSPLUG_API CProcessMg* mProcessMg=0;
class MyApp:CWinApp
{
public:
	virtual int ExitInstance()
	{
		TRACE(L"Hello ProcessLib Exit!\n");
		if(mProcessMg)
			delete(mProcessMg);
		mProcessMg=0;
		return CWinApp::ExitInstance();
	}
	virtual BOOL InitInstance()
	{
		TRACE(L"Hello This is ProcessLib!\n");
		mProcessMg=0;
		return CWinApp::InitInstance();
	}
};

// 唯一的应用程序对象

MyApp theApp;

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

extern "C" IMGPROCESSPLUG_API void* HGetPluginInterface( void )
{
	if(!mProcessMg)
		mProcessMg=new CProcessMg();
	return (CProcessMg*)mProcessMg;
}

extern "C" IMGPROCESSPLUG_API CString HGetPluginName( void )
{
	return L"ProcessLib";
}
