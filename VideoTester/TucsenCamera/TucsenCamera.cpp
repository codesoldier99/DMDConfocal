// TucsenCamera.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "TucsenCamera.h"
#include "VideoManager.h"

extern "C" TUCSENCAMERA_API HVideoManager* m_pVieoManager=0;// 只导出这个变量！！！
// 唯一的应用程序对象

class MyApp:CWinApp
{

public:
	virtual int ExitInstance()
	{
		TRACE(L"TucsenVideo Exit!\n");

		return CWinApp::ExitInstance();
	}
	virtual BOOL InitInstance()
	{
		TRACE(L"TucsenVideo Init!\n");
		//实例化类型
		CVideoManager* t_manager=new CVideoManager();
		m_pVieoManager=(HVideoManager*)t_manager;
		return CWinApp::InitInstance();
	}
};

MyApp theApp;

//using namespace std;

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
