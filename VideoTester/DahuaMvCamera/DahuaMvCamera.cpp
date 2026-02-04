// DahuaMvCamera.cpp: 定义 DLL 的初始化例程。
//


#include "stdafx.h"
#include "DahuaMvCamera.h"
#include "VideoManager.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern "C" DAHUAMVCAMERA_API HVideoManager* m_pVieoManager = 0;// 只导出这个变量！！！

class MyApp :CWinApp
{

public:
	virtual int ExitInstance()
	{
		TRACE(L"Hello DahuaMvCamera Exit!\n");

		return CWinApp::ExitInstance();
	}
	virtual BOOL InitInstance()
	{
		TRACE(L"Hello This is DahuaMvCamera!\n");
		//实例化类型
		CVideoManager* mManger = new CVideoManager();
		m_pVieoManager = (HVideoManager*)mManger;
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
