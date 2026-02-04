// MoticStage.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "MoticStage.h"
#include "MotionBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern "C" MOTICSTAGE_API CMotionBox* m_GearBox=0;// 只导出这个变量！！！
class MyApp:CWinApp
{
	public:
		virtual int ExitInstance()
		{
			TRACE(L"Hello MoticVideo Exit!\n");
			if(m_GearBox)
				delete(m_GearBox);
			m_GearBox=0;
			return CWinApp::ExitInstance();
		}
		virtual BOOL InitInstance()
		{
			TRACE(L"Hello This is MoticVideo!\n");
			m_GearBox=0;
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

extern "C" MOTICSTAGE_API void* HGetPluginInterface( void )
{
	if(!m_GearBox)
		m_GearBox=new CMotionBox();
	return (HGearBox*)m_GearBox;
}

extern "C" MOTICSTAGE_API CString HGetPluginName( void )
{
	return L"MoticGearBox";
}
