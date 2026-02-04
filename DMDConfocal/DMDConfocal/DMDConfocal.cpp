
// DMDConfocal.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "DMDConfocal.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "DMDConfocalDoc.h"
#include "DMDConfocalView.h"

#include "HGloableFunction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDMDConfocalApp

BEGIN_MESSAGE_MAP(CDMDConfocalApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CDMDConfocalApp::OnAppAbout)
	// 基于文件的标准文档命令
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// 标准打印设置命令
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CDMDConfocalApp 构造

CDMDConfocalApp::CDMDConfocalApp()
{
	m_bHiColorIcons = TRUE;

	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// 如果应用程序是利用公共语言运行时支持(/clr)构建的，则:
	//     1) 必须有此附加设置，“重新启动管理器”支持才能正常工作。
	//     2) 在您的项目中，您必须按照生成顺序向 System.Windows.Forms 添加引用。
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: 将以下应用程序 ID 字符串替换为唯一的 ID 字符串；建议的字符串格式
	//为 CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("DMDConfocal.AppID.NoVersion"));

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}

// 唯一的一个 CDMDConfocalApp 对象

CDMDConfocalApp theApp;


// CDMDConfocalApp 初始化

BOOL CDMDConfocalApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// 初始化 OLE 库
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();
	EnableTaskbarInteraction();
	// 使用 RichEdit 控件需要  AfxInitRichEdit2()	
	// AfxInitRichEdit2();

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名

	SetRegistryKey(_T("共聚焦程序20201020"));
	InitLayOut();
	LoadStdProfileSettings(4);  // 加载标准 INI 文件选项(包括 MRU)


	InitContextMenuManager();
	InitKeyboardManager();
	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// 注册应用程序的文档模板。文档模板
	// 将用作文档、框架窗口和视图之间的连接
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_DMDConfocalEng,//IDR_DMDConfocalTYPE,
		RUNTIME_CLASS(CDMDConfocalDoc),
		RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CDMDConfocalView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	//step 1.1 Core_Init
	if(!m_CofocalCoreLib.GetConfocalCore()->Init())
	{
		return FALSE;
	}
	
	// 创建主 MDI 框架窗口
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))//显示出界面
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	//pMainFrame->ShowWindow(SW_HIDE);
	SetupView();
	//pMainFrame->ShowWindow(m_nCmdShow);
	//pMainFrame->Invalidate();
	//pMainFrame->UpdateWindow();

	// 仅当具有后缀时才调用 DragAcceptFiles
	//  在 MDI 应用程序中，这应在设置 m_pMainWnd 之后立即发生

	// 分析标准 shell 命令、DDE、打开文件操作的命令行
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	cmdInfo.m_nShellCommand=CCommandLineInfo::FileNothing;//Xsc_不添加任何的文档文件

	// 调度在命令行中指定的命令。如果
	// 用 /RegServer、/Register、/Unregserver 或 /Unregister 启动应用程序，则返回 FALSE。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// 主窗口已初始化，因此显示它并对其进行更新
	//SetupView();

	//第二步
	int params = 1;
	if(!m_CofocalCoreLib.GetConfocalCore()->Init(&params))//初始化面板
	{
		return FALSE;
	}

	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

void CDMDConfocalApp::InitLayOut()
{
#ifdef _DEBUG
	
#else
	CString strKeyValue = L"NoValue";
	strKeyValue = GetProfileString(L"Workspace", L"ApplicationLook", L"NoValue");
	if (strKeyValue == L"NoValue")
	{
		ShellExecute(NULL, L"open", HGetStartPath() + L"\\Configure\\LayoutSetting.bat", NULL, NULL, SW_SHOW);
		Sleep(100);
		//下面的不生效
		/*CString strCmd = L"regdedit /s " +HGetStartPath() + L"\\Configure\\LayoutSetting.reg";
		std::string tempName = (LPCSTR)CStringA(strCmd);
		const char *tmp = tempName.c_str();
		system(tmp);
		Sleep(100);*/
		//下面的会报对话框
		//ShellExecute(NULL, L"open", HGetStartPath() + L"\\Configure\\LayoutSetting.bat", NULL, NULL, SW_SHOW);
	}
	else
	{
	}
#endif
}

void CDMDConfocalApp::ResetLayOut()
{
#ifdef _DEBUG
	AfxMessageBox(L"Can't be used in debug mode!");
#else
	SetRegistryKey(_T("共聚焦程序20201020-A"));//防止退出之后又要去覆盖保存
	ShellExecute(NULL, L"open", HGetStartPath() + L"\\Configure\\LayoutSetting.bat", NULL, NULL, SW_SHOW);
	AfxMessageBox(L"Reset finished! Please restart soft!");
#endif
}

void CDMDConfocalApp::SetupView()
{
	//先申请Doc，然后获取每个Doc的View，再通过给每个View来设置包含的hWnd
	int iViewCnt = m_CofocalCoreLib.GetConfocalCore()->GetViewCfg().Count;
	int Viewlist[9] = { 0 };
	int j = 0;
	for (int i=0;i<9;i++)
	{
		if (m_CofocalCoreLib.GetConfocalCore()->GetViewCfg().ViewShow[i])
		{
			Viewlist[j] = i;
			j++;
		}
	}
	if (j != iViewCnt)
	{
		AfxMessageBox(L"View Cfg is err!");
		return;
	}

	
	for (int i=0;i< iViewCnt;i++)
	{
		m_pDocManager->OnFileNew();//先直接创建出几个View
	}

	POSITION		t_DocTemp_Pos	= m_pDocManager->GetFirstDocTemplatePosition();
	CDocTemplate*	t_DocTemp		= m_pDocManager->GetNextDocTemplate(t_DocTemp_Pos);
	POSITION		t_Doc_Pos		= t_DocTemp->GetFirstDocPosition();
	CString t_Title;

	while (iViewCnt--)
	{
		CWnd* t_viewWnd = ((m_CofocalCoreLib.GetConfocalCore())->
			GetViewPanel((VIEWPANEL_TYPE)(Viewlist[iViewCnt]))->GetCWnd());

		t_viewWnd->GetWindowTextW(t_Title);
		CDocument*			t_Doc		= t_DocTemp->GetNextDoc(t_Doc_Pos);//用于设置视图名称
		POSITION			t_View_Pos	= t_Doc->GetFirstViewPosition();
		CDMDConfocalView*	t_View		= (CDMDConfocalView*)t_Doc->GetNextView(t_View_Pos);//用于设置视图内容

		t_Doc->SetTitle(t_Title);
		t_View->SetShowWnd(t_viewWnd);
		t_View->SendMessage(WM_SIZE);//发消息防变形
	}
}

int CDMDConfocalApp::ExitInstance()
{
	if (m_CofocalCoreLib.GetConfocalCore())
	{
		m_CofocalCoreLib.GetConfocalCore()->Uninit();
	}
	//if(!m_CofocalCoreLib.GetConfocalCore()->Uninit())
	//{
	//	return FALSE;
	//}
	AfxOleTerm(FALSE);
	return CWinAppEx::ExitInstance();
}

// CDMDConfocalApp 消息处理程序


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// 用于运行对话框的应用程序命令
void CDMDConfocalApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CDMDConfocalApp 自定义加载/保存方法

void CDMDConfocalApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CDMDConfocalApp::LoadCustomState()
{
}

void CDMDConfocalApp::SaveCustomState()
{
}

void CDMDConfocalApp::SetLangue()
{
	//LCID lcid;
	//if(strlangid == -1) // 没有语言字符串使用语言 ID
	//{
	//	switch(langid)
	//	{
	//		case MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED):
	//			lcid = MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED);
	//			break;
	//		default:
	//			lcid = MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US);
	//	}
	//}
	//else
	//{
	//	lcid = strlangid;
	//}
	//OSVERSIONINFOEX vi;
	//vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	//::GetVersionEx((LPOSVERSIONINFO)&vi);
	//if(vi.dwMajorVersion<=5)
	//{
	//	//xp, 2003.etc
	//	SetThreadLocale(MAKELCID(lcid,SORT_DEFAULT));			
	//}
	//else
	//{
	//	//vista or later
	//	LANGID id = SetThreadUILanguage(MAKELCID(lcid,SORT_DEFAULT));			
	//}

}



void CAboutDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}


BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// TODO:  在此添加额外的初始化
	CString str = L"DMDConfocal,2021.11.13A";
#ifdef _WIN64
	str += L" (x64";
#else
	str += L" (win32";
#endif

#ifdef _DEBUG
	str += L"_debug版)";
#else
	str += L"_release版)";
#endif
	SetDlgItemText(IDC_VERSION, str);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

