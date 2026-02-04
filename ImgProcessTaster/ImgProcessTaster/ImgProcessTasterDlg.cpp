
// ImgProcessTasterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ImgProcessTaster.h"
#include "ImgProcessTasterDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CImgProcessTasterDlg 对话框




CImgProcessTasterDlg::CImgProcessTasterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CImgProcessTasterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CImgProcessTasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, mTab);
}

BEGIN_MESSAGE_MAP(CImgProcessTasterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CImgProcessTasterDlg::OnTcnSelchangeTab1)
END_MESSAGE_MAP()


// CImgProcessTasterDlg 消息处理程序

BOOL CImgProcessTasterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// 将“关于...”菜单项添加到系统菜单中。
	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	mTab.InsertItem(0,L"AutoFocus");
	mTab.InsertItem(1,L"MutiFocus");
	mTab.InsertItem(2,L"MapBuild");
	mTab.InsertItem(3,L"PreProcess");
	mTab.InsertItem(4,L"mTestDlg");

	CRect rs;
	mTab.GetClientRect(&rs);
	rs.top	+=20;
	rs.bottom-=1;
	rs.left+=1;
	rs.right-=2;

	mFocusDlg.Create(IDD_FocusDlg,&mTab);
	mMapDlg.Create(IDD_MutiFocusDlg,&mTab);
	mMutiDlg.Create(IDD_MutiBuild,&mTab);
	mPreDlg.Create(IDD_PreProcessDlg,&mTab);
	mTestDlg.Create(IDD_TestDlg,&mTab);

	mFocusDlg.MoveWindow(&rs);
	mMapDlg.MoveWindow(&rs);
	mMutiDlg.MoveWindow(&rs);
	mPreDlg.MoveWindow(&rs);
	mTestDlg.MoveWindow(&rs);

	mFocusDlg.ShowWindow(true);
	mMapDlg.ShowWindow(false);
	mMutiDlg.ShowWindow(false);
	mPreDlg.ShowWindow(false);
	mTestDlg.ShowWindow(false);
	InitDlg();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

bool CImgProcessTasterDlg::InitDlg()
{
	mProcessMg=(HCoreProcessMg*)LoadDlls(L"ImgProcessPlug.pro");
	mFocusDlg.SetProcess(mProcessMg->GetProcessCore(PROCESS_ImgQuality));//图像调焦特征量参数
	mFocusDlg.SetFunction(mProcessMg->GetFunction());//存储图像的函数

	mMapDlg.SetProcess(mProcessMg->GetProcessCore(PROCESS_3DReBuild));//图像三维重建的参数
	mMapDlg.SetFunction(mProcessMg->GetFunction());//存储图像的函数

	mPreDlg.SetCalProcess(mProcessMg->GetProcessCore(PROCESS_CalValue));
	mPreDlg.SetPreProcess(mProcessMg->GetImgPreProcess(L"GrayConvert"),L"GrayConvert");
	mPreDlg.SetPreProcess(mProcessMg->GetImgPreProcess(L"ScaleConvert"),L"ScaleConvert");
	mPreDlg.SetPreProcess(mProcessMg->GetImgPreProcess(L"GamaConvert"),L"GamaConvert");
	mPreDlg.SetPreProcess(mProcessMg->GetImgPreProcess(L"CutConvert"),L"CutConvert");
	mPreDlg.SetPreProcess(mProcessMg->GetImgPreProcess(L"FilterConvert"), L"FilterConvert");

	mPreDlg.SetProcess(mProcessMg->GetImgProcess(L"AutoWB"),L"AutoWB");
	mPreDlg.SetProcess(mProcessMg->GetImgProcess(L"BlockScan"),L"BlockScan");
	return true;
}

void CImgProcessTasterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CImgProcessTasterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CImgProcessTasterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CImgProcessTasterDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	mFocusDlg.ShowWindow(false);
	mMapDlg.ShowWindow(false);
	mMutiDlg.ShowWindow(false);
	mPreDlg.ShowWindow(false);
	mTestDlg.ShowWindow(false);
	switch(mTab.GetCurSel())
	{
		case 0:
			mFocusDlg.ShowWindow(true);
			break;
		case 1:
			mMapDlg.ShowWindow(true);
			break;
		case 2:
			mMutiDlg.ShowWindow(true);
			break;
		case 3:
			mPreDlg.ShowWindow(true);
			break;
		case 4:
			mTestDlg.ShowWindow(true);
			break;
	}
	*pResult = 0;
}

void* CImgProcessTasterDlg::LoadDlls(CString t_name)
{
	HMODULE t_mudule=GetModuleHandle(0);
	CString pfileName;
	GetModuleFileName(t_mudule,pfileName.GetBufferSetLength(MAX_PATH),MAX_PATH);
	pfileName.ReleaseBuffer();
	int nPos=pfileName.ReverseFind('\\');
	pfileName=pfileName.Left(nPos);
	pfileName+=L"\\";
	pfileName+=t_name;
	HMODULE t_module;
	t_module=::LoadLibrary(pfileName);
	if(t_module==NULL)
		return 0;
	PluginLib_GetInterface t_GetInterface;
	PluginLib_GetName		t_GetName;
	t_GetInterface=(PluginLib_GetInterface)::GetProcAddress(t_module,"HGetPluginInterface");
	t_GetName=(PluginLib_GetName)::GetProcAddress(t_module,"HGetPluginName");
	CString tname=t_GetName();
	TRACE(tname);
	void* t=t_GetInterface();
	return t;
}
