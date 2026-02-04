
// MainFrm.cpp : CMainFrame 类的实现
//

#include "stdafx.h"
#include "DMDConfocal.h"
#include "HConfFile.h"
#include "MainFrm.h"
#include "HGloableFunction.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
	ON_WM_SETTINGCHANGE()
	ON_UPDATE_COMMAND_UI(IDS_STATUS_PANELX, &CMainFrame::OnUpdateIdsStatusPanelX)
	ON_UPDATE_COMMAND_UI(ID_View_Lock, &CMainFrame::OnUpdateLockDlg)
	ON_COMMAND(ID_View_Lock, &CMainFrame::OnViewLock)
	ON_UPDATE_COMMAND_UI(ID_ViewLargeTool, &CMainFrame::OnUpdateLargeIcon)
	ON_UPDATE_COMMAND_UI(IDS_STATUS_PANELY, &CMainFrame::OnUpdateIdsStatusPanelY)
	ON_UPDATE_COMMAND_UI(IDS_STATUS_PANELZ, &CMainFrame::OnUpdateIdsStatusPanelZ)
	ON_MESSAGE(WM_USER+529,OnAxisUpdata)	
	ON_COMMAND(ID_DefaultView, &CMainFrame::OnDefaultview)
	ON_COMMAND(ID_ViewLargeTool, &CMainFrame::OnViewlargetool)
	ON_COMMAND(ID_Capture, &CMainFrame::OnCapture)
	ON_COMMAND(ID_MeasureLine, &CMainFrame::OnMeasureline)
	ON_COMMAND(ID_MeasureRound, &CMainFrame::OnMeasureround)
	ON_COMMAND(ID_MeasureRec, &CMainFrame::OnMeasurerec)
	ON_COMMAND(ID_HelpMenuBtn, &CMainFrame::OnHelpmenubtn)
	ON_COMMAND(ID_MenuCVBtn, &CMainFrame::OnMenucvbtn)
	ON_COMMAND(ID_TOOLS_DEFAULTLAYOUT, &CMainFrame::OnToolsDefaultlayout)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 状态行指示器
	//ID_INDICATOR_CAPS,
	//ID_INDICATOR_NUM,
	//ID_INDICATOR_SCRL,
	IDS_STATUS_PANELX,
	IDS_STATUS_PANELY,
	IDS_STATUS_PANELZ,
};

// CMainFrame 构造/析构

CMainFrame::CMainFrame()
{
	// TODO: 在此添加成员初始化代码
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2008);
	m_ConfocalCore=((CDMDConfocalApp*)AfxGetApp())->GetConfocalCore();
  AxisX = 0;
  AxisY = 0;
  AxisZ = 0;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;
	// 基于持久值设置视觉管理器和样式
	OnApplicationLook(theApp.m_nAppLook);

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // 其他可用样式...
	mdiTabParams.m_bActiveTabCloseButton = FALSE;//TRUE;      // 设置为 FALSE 会将关闭按钮放置在选项卡区域的右侧
	mdiTabParams.m_bTabCloseButton = FALSE;//Tab窗口的右边打叉键
	mdiTabParams.m_bEnableTabSwap = FALSE;
	mdiTabParams.m_bTabIcons = FALSE;    // 设置为 TRUE 将在 MDI 选项卡上启用文档图标
	mdiTabParams.m_bAutoColor = TRUE;    // 设置为 FALSE 将禁用 MDI 选项卡的自动着色
	mdiTabParams.m_bDocumentMenu = TRUE; // 在选项卡区域的右边缘启用文档菜单
	EnableMDITabbedGroups(TRUE, mdiTabParams);

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("未能创建菜单栏\n");
		return -1;      // 未能创建
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// 防止菜单栏在激活时获得焦点
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_TOOLBAR1, 0, 0, TRUE, 0, 0, IDB_PNG1)
		||!m_wndMeasureBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
		new CRect(1,1,1,1),uiLastUserToolBarId+1) ||
		!m_wndMeasureBar.LoadToolBar(IDR_TOOLBAR2, 0, 0, TRUE, 0, 0, IDB_PNG2))
	{
		TRACE0("未能创建工具栏\n");
		return -1;      // 未能创建
	}

	//int w = 32;
	//m_wndToolBar.SetSizes(CSize(w + 7,w + 6),CSize(w,w));
	//m_wndToolBar.EnableTextLabels(TRUE);
	//m_wndMeasureBar.EnableTextLabels(TRUE); //显示文本到图标下面
	//m_wndToolBar.SetShowTooltips(false);

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	strToolBarName=L"ToolBar";
	m_wndToolBar.SetWindowText(strToolBarName);
	bNameValid = strToolBarName.LoadString(IDS_MEASUREBAR);
	strToolBarName=L"MeasureBar";
	ASSERT(bNameValid);
	m_wndMeasureBar.SetWindowText(strToolBarName);

	//CString strCustomize;
	//bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	//ASSERT(bNameValid);
	//strCustomize=L"非标准";
	//m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize); //工具栏的自定义

	// 允许用户定义的工具栏操作:
	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("未能创建状态栏\n");
		return -1;      // 未能创建
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
	PosX=0;PosY=0;PosZ=0;
	m_wndStatusBar.SetPaneText(1,L"X:0.000mm");
	m_wndStatusBar.SetPaneWidth(1,60);
	m_wndStatusBar.SetPaneText(2,L"Y:0.000mm");
	m_wndStatusBar.SetPaneWidth(2,60);
	m_wndStatusBar.SetPaneText(3,L"Z:0.000mm");
	m_wndStatusBar.SetPaneWidth(3,60);

	// TODO: 如果您不希望工具栏和菜单栏可停靠，请删除这五行
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndMeasureBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);
	DockPane(&m_wndMeasureBar);


	// 启用 Visual Studio 2005 样式停靠窗口行为
	CDockingManager::SetDockingMode(DT_SMART);
	// 启用 Visual Studio 2005 样式停靠窗口自动隐藏行为
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// 加载菜单项图像(不在任何标准工具栏上):
	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

	// 创建停靠窗口
	if (!CreateDockingWindows())
	{
		TRACE0("未能创建停靠窗口\n");
		return -1;
	}

	//接收三个坐标轴的对象
  if (m_ConfocalCore->GetGearBox())
  {
    AxisX = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_X);
    AxisY = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Y);
    AxisZ = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Z);
  }
	if(AxisX)
		AxisX->GetSubject()->Attach(this);
	if(AxisY)
		AxisY->GetSubject()->Attach(this);
	if(AxisZ)
		AxisZ->GetSubject()->Attach(this);

	// 启用增强的窗口管理对话框
	EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);
	// 启用工具栏和停靠窗口菜单替换
	//EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);
	EnablePaneMenu(TRUE, NULL, NULL, ID_VIEW_TOOLBAR);
	//EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, NULL, ID_VIEW_TOOLBAR);
	//ID_VIEW_TOOLBAR 占位符
	// 启用快速(按住 Alt 拖动)工具栏自定义
	CMFCToolBar::EnableQuickCustomization();
	if (CMFCToolBar::GetUserImages() == NULL)
	{
		// 加载用户定义的工具栏图像
		if (m_UserImages.Load(_T(".\\UserImages.bmp")))
		{
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}
	// 启用菜单个性化(最近使用的命令)
	// TODO: 定义您自己的基本命令，确保每个下拉菜单至少有一个基本命令。
	// 将文档名和应用程序名称在窗口标题栏上的顺序进行交换。这
	// 将改进任务栏的可用性，因为显示的文档名带有缩略图。
	ModifyStyle(0, FWS_PREFIXTITLE);
	bLockDlg=false;
	bToolIcon=false;
	m_wndToolBar.SetLargeIcons(bToolIcon);
	m_wndMeasureBar.SetLargeIcons(bToolIcon);

	// 设置应用程序的图标
	HConfigure mConfig;
	CString mStrIco=mConfig.GetString(L"Common",L"AppIconPath",L"Confocal_Config");
	HICON icon = (HICON)LoadImage(0, mStrIco,
		IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);
	if(icon)SetIcon(icon, TRUE);
	return 0;
}

//bool CMainFrame::InitAllDlg()
//{
//	// 创建停靠窗口
//	if (!CreateDockingWindows())
//	{
//		TRACE0("未能创建停靠窗口\n");
//		return false;
//	}
//
//	//接收三个坐标轴的对象
//	AxisX=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_X);
//	AxisY=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Y);
//	AxisZ=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Z);
//	if(AxisX)
//		AxisX->GetSubject()->Attach(this);
//	if(AxisY)
//		AxisY->GetSubject()->Attach(this);
//	if(AxisZ)
//		AxisZ->GetSubject()->Attach(this);
//
//	return true;
//}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return TRUE;
}

BOOL CMainFrame::CreateDockingWindows()
{
	CDockDlg* pPanel=&m_wndPane;
	CString name;
	int i=0;
	int PanelNum=7;//CONFOCALDOCKPANS; 停靠窗口个数
	while(true)
	{
		name.Format(L"TestPanal%d",i);
		pPanel->Create(name, this, CRect(0, 0, 200, 200), TRUE, 2000+i, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI);
		pPanel->EnableDocking(CBRS_ALIGN_ANY);
		DockPane(pPanel);
		//这里直接向核心申请资源
		pPanel->SetWnd(m_ConfocalCore->GetDockablePanel((DOCKPANEL_TYPE)(i))->GetCWnd());
		if(++i<PanelNum)
		{
			pPanel->NewWnd();
			pPanel=pPanel->GetNextWnd();
		}
		else
			break;
	}

	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	/*HICON hFileViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_FILE_VIEW_HC : IDI_FILE_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndFileView.SetIcon(hFileViewIcon, FALSE);*/

	UpdateMDITabbedBarsIcons();
}

// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame 消息处理程序

void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

void CMainFrame::OnViewCustomize()
{
	//CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* 扫描菜单*/);
	//pDlgCust->EnableUserDefinedToolbars();
	//pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

//exe每次启动会调用LoadFrame
BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// 基类将执行真正的工作
	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}
	// 为所有用户工具栏启用自定义按钮
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != NULL)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}
	return TRUE;
}


void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CMDIFrameWndEx::OnSettingChange(uFlags, lpszSection);
	//m_wndOutput.UpdateFonts();
}

void CMainFrame::OnUpdateIdsStatusPanelX(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable();
}

void CMainFrame::OnUpdateLockDlg(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(bLockDlg);
}

void CMainFrame::OnUpdateLargeIcon(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(bToolIcon);
}

void CMainFrame::OnUpdateIdsStatusPanelY(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable();
}

void CMainFrame::OnUpdateIdsStatusPanelZ(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable();
}

int CMainFrame::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if(pSubject==AxisX->GetSubject())
	{
		if(ID==NOTIFY_AXIS_MOVED||ID==NOTIFY_AXIS_POSITION)
		{
			PosX=*((float*)pParam);
			SendMessage(WM_USER+529);
		}
		//Updata_Axis();
	}
	else if (pSubject==AxisY->GetSubject())
	{
		if(ID==NOTIFY_AXIS_MOVED||ID==NOTIFY_AXIS_POSITION)
		{
			PosY=*((float*)pParam);
			SendMessage(WM_USER+529);
		}
	}else if (pSubject==AxisZ->GetSubject())
	{
		if(ID==NOTIFY_AXIS_MOVED||ID==NOTIFY_AXIS_POSITION)
		{

			PosZ=*((float*)pParam);
			SendMessage(WM_USER+529);
		}
	}
	return 0;
}

LRESULT CMainFrame::OnAxisUpdata(WPARAM wParam,LPARAM lParam)
{
	CString pos;

	pos.Format(L"%.3fmm",PosX);
	m_wndStatusBar.SetPaneText(1,pos);

	pos.Format(L"%.3fmm",PosY);
	m_wndStatusBar.SetPaneText(2,pos);

	pos.Format(L"%.3fmm",PosZ);
	m_wndStatusBar.SetPaneText(3,pos);
	return 0;
}

void CMainFrame::OnViewLock()
{
	// TODO: 在此添加命令处理程序代码
	bLockDlg=!bLockDlg;
	CDockDlg* pPanel=&m_wndPane;
	while (pPanel)
	{
		pPanel->LockPane(!bLockDlg);
		pPanel->EnableDocking(0);
		pPanel=pPanel->GetNextWnd();
		LockAllSliders(bLockDlg);
		LockToolBar(!bLockDlg);
		//m_wndToolBar.EnableDocking(!bLockDlg);
		//m_wndToolBar.EnableGripper(!bLockDlg);
	}
}


void CMainFrame::OnDefaultview()
{
	// TODO: 在此添加命令处理程序代码
	//SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
	//LoadFrame(IDR_MAINFRAME);
}


void CMainFrame::OnViewlargetool()
{
	// TODO: 在此添加命令处理程序代码
	bToolIcon=!bToolIcon;
	m_wndToolBar.SetLargeIcons(bToolIcon);
	m_wndMeasureBar.SetLargeIcons(bToolIcon);

	//m_wndMeasureBar.SetWindowPos()
}


void CMainFrame::OnCapture()
{
	// TODO: 在此添加命令处理程序代码
}


void CMainFrame::OnMeasureline()
{
	// TODO: 在此添加命令处理程序代码
	TRACE("绘制直线!\n");
}


void CMainFrame::OnMeasureround()
{
	// TODO: 在此添加命令处理程序代码
}


void CMainFrame::OnMeasurerec()
{
	// TODO: 在此添加命令处理程序代码
}

void CMainFrame::LockAllSliders(bool bLock)
{	
	CWnd* container = GetWindow(GW_HWNDFIRST);
	DWORD pid;
	::GetWindowThreadProcessId(m_hWnd,&pid);
	while(container)
	{
		DWORD pid2 = 0;
		::GetWindowThreadProcessId(container->m_hWnd,&pid2);		
		if((DYNAMIC_DOWNCAST(CPaneFrameWnd, container)||DYNAMIC_DOWNCAST(CMDIFrameWndEx, container))&&pid==pid2)
		{
			CWnd* pChild = container->GetWindow(GW_CHILD);
			while(pChild)
			{
				CPaneDivider* pSlider = DYNAMIC_DOWNCAST(CPaneDivider, pChild);
				if(pSlider != 0)
				{
					pSlider->EnableWindow(!bLock);
				}
				pChild = pChild->GetNextWindow(GW_HWNDNEXT);
			}
		}
		container = container->GetWindow(GW_HWNDNEXT);
	}	
}

void CMainFrame::LockToolBar(bool bLock)
{
	m_wndToolBar.EnableGripper(bLock);
	m_wndToolBar.EnableDocking(bLock?0:CBRS_ALIGN_ANY);
	m_wndMeasureBar.EnableGripper(bLock);
	m_wndMeasureBar.EnableDocking(bLock?0:CBRS_ALIGN_ANY);
	m_wndMenuBar.EnableGripper(bLock);
	m_wndMenuBar.EnableDocking(bLock?0:CBRS_ALIGN_ANY);
	//DWORD m_dwControlBarStyle = bLock?0:AFX_CBRS_FLOAT; CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC
	DWORD m_dwStyle=m_wndToolBar.GetPaneStyle();
	if(bLock)
		m_dwStyle = m_dwStyle&(~CBRS_GRIPPER)&(~CBRS_FLYBY)&(~CBRS_SIZE_DYNAMIC);
	else
		m_dwStyle = m_dwStyle|CBRS_GRIPPER|CBRS_FLYBY|CBRS_SIZE_DYNAMIC; 
	m_wndToolBar.SetPaneStyle(m_dwStyle);
	m_wndMeasureBar.SetPaneStyle(m_dwStyle);
}


void CMainFrame::OnHelpmenubtn()
{
	// TODO: 在此添加命令处理程序代码
	//HINSTANCE dll_hInstance = GetModuleHandle(L"ConfocalUILib.dll");
	//AfxSetResourceHandle(dll_hInstance); //切换状态

	//((CDialogEx*)mDlg->GetCWnd())->DoModal();
	
}


void CMainFrame::OnMenucvbtn()
{
	// TODO: 在此添加命令处理程序代码
	HCorePanel* mDlg=m_ConfocalCore->GetMsgPanel(MSG_CALCV,true);
	mDlg->InitPanel(m_ConfocalCore);
	mDlg->GetCWnd()->ShowWindow(SW_SHOW);
}


void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// TODO: 在此添加专用代码和/或调用基类

	//HConfigure mConfig;
	//CString mStrTitle=mConfig.GetString(L"Common",L"AppTitle",L"Confocal_Config");
	//HINSTANCE h = ::AfxGetResourceHandle();
	//::AfxSetResourceHandle(::AfxGetInstanceHandle());
	//SetWindowText(mStrTitle);
	//::AfxSetResourceHandle(h);
	__super::OnUpdateFrameTitle(bAddToTitle);
}


void CMainFrame::OnToolsDefaultlayout()
{
	// TODO: 在此添加命令处理程序代码
	CWinApp* pApp = AfxGetApp();
	((CDMDConfocalApp*)pApp)->ResetLayOut();
}
