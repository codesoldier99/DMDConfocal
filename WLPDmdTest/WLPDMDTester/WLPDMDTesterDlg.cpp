
// WLPDMDTesterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "WLPDMDTester.h"
#include "WLPDMDTesterDlg.h"
#include "afxdialogex.h"
#include "HGloableFunction.h"
#include "BrowseFolderDialog.h"

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


// CWLPDMDTesterDlg 对话框




CWLPDMDTesterDlg::CWLPDMDTesterDlg(CWnd* pParent /*=NULL*/)
  : CDialogEx(CWLPDMDTesterDlg::IDD, pParent)
  , mPointSize(1)
  , mPtSpace(10)
  , mLineW(4)
  , mLineCount(16)
  , m_FrameMargin(20000)//(3800)
  , m_FrameInterval(0)//(150)
  , delay(0)
  , plusewidth(20)
  , m_StrPath(_T(""))
  , miResolution(0)
  , miLineType(0)
  , m_Vexposure(0)
  , mPtExtraW(1)
  , mLExtraW(4)
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWLPDMDTesterDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_EDIT1, mPointSize);
  DDX_Text(pDX, IDC_EDIT2, mPtSpace);
  DDX_Text(pDX, IDC_EDIT3, mLineW);
  DDX_Text(pDX, IDC_EDIT4, mLineCount);
  DDX_Text(pDX, IDC_EDIT5, m_FrameMargin);
  DDX_Text(pDX, IDC_EDIT6, m_FrameInterval);
  DDX_Text(pDX, IDC_EDIT7, delay);
  DDX_Text(pDX, IDC_EDIT8, plusewidth);
  DDX_Control(pDX, IDC_COMBO2, m_trigger_mode);
  DDX_Control(pDX, IDC_COMBO1, mGray);
  DDX_Text(pDX, IDC_SavePath, m_StrPath);
  DDX_Radio(pDX, IDC_RADIO1, miResolution);
  DDX_Radio(pDX, IDC_RADIO4, miLineType);
  DDX_Control(pDX, IDC_SLIDER1, m_ExSlider);
  DDX_Text(pDX, IDC_EDIT10, m_Vexposure);
  DDX_Text(pDX, IDC_EDIT9, mPtExtraW);
  DDX_Text(pDX, IDC_EDIT11, mLExtraW);
  DDX_Control(pDX, IDC_COMBO3, m_dev_type);
}

BEGIN_MESSAGE_MAP(CWLPDMDTesterDlg, CDialogEx)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_BN_CLICKED(IDC_SavePointBtn, &CWLPDMDTesterDlg::OnBnClickedSavepointbtn)
  ON_BN_CLICKED(IDC_SaveLineBtn, &CWLPDMDTesterDlg::OnBnClickedSavelinebtn)
  ON_BN_CLICKED(IDC_InitBtn, &CWLPDMDTesterDlg::OnBnClickedInitbtn)
  ON_BN_CLICKED(IDC_OpenImgBtn, &CWLPDMDTesterDlg::OnBnClickedOpenimgbtn)
  ON_BN_CLICKED(IDC_DownPointBtn, &CWLPDMDTesterDlg::OnBnClickedDownpointbtn)
  ON_BN_CLICKED(IDC_DownLinetBtn, &CWLPDMDTesterDlg::OnBnClickedDownlinetbtn)
  ON_BN_CLICKED(IDC_StartBtn, &CWLPDMDTesterDlg::OnBnClickedStartbtn)
  ON_BN_CLICKED(IDC_TriggerBtn, &CWLPDMDTesterDlg::OnBnClickedTriggerbtn)
  ON_BN_CLICKED(IDC_StopBtn, &CWLPDMDTesterDlg::OnBnClickedStopbtn)
  ON_BN_CLICKED(IDC_BrightBtn, &CWLPDMDTesterDlg::OnBnClickedBrightbtn)
  ON_BN_CLICKED(IDC_BlackBtn, &CWLPDMDTesterDlg::OnBnClickedBlackbtn)
  ON_BN_CLICKED(IDC_UpdateBtn, &CWLPDMDTesterDlg::OnBnClickedUpdatebtn)
  ON_EN_CHANGE(IDC_EDIT5, &CWLPDMDTesterDlg::OnEnChangeEdit5)
  ON_EN_CHANGE(IDC_EDIT6, &CWLPDMDTesterDlg::OnEnChangeEdit5)
  ON_BN_CLICKED(IDC_CHECKSave, &CWLPDMDTesterDlg::OnBnClickedChecksave)
  ON_BN_CLICKED(IDC_RADIO1, &CWLPDMDTesterDlg::OnEnChangeEdit5)
  ON_BN_CLICKED(IDC_RADIO2, &CWLPDMDTesterDlg::OnEnChangeEdit5)
  ON_BN_CLICKED(IDC_RADIO3, &CWLPDMDTesterDlg::OnEnChangeEdit5)
  ON_WM_HSCROLL()
  ON_EN_CHANGE(IDC_EDIT1, &CWLPDMDTesterDlg::OnEnChangeEdit1)
  ON_EN_CHANGE(IDC_EDIT3, &CWLPDMDTesterDlg::OnEnChangeEdit3)
  ON_CBN_SELCHANGE(IDC_COMBO1, &CWLPDMDTesterDlg::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// CWLPDMDTesterDlg 消息处理程序

BOOL CWLPDMDTesterDlg::OnInitDialog()
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
  mDMDManager = (IDMDManager*)LoadDlls(L"WlpDMDLib.dll");
  if (mDMDManager)
    mDMDManager->Attach(this);
  else
    AfxMessageBox(L"Init the DMD failed!");
  //新增加的代码
  m_trigger_mode.InsertString(0, L"mode four, WLP_TRIGGER_LOOP");
  m_trigger_mode.InsertString(0, L"mode three,WLP_TRIGGER_FRAMES_AND_STOP");
  m_trigger_mode.InsertString(0, L"mode two,  WLP_TRIGGER_FRAMES");
  m_trigger_mode.InsertString(0, L"mode one,  WLP_TRIGGER_SINGLE_FRAME");
  m_trigger_mode.SetCurSel(0);
  
  m_dev_type.AddString(_T("WLP"));
  m_dev_type.AddString(_T("ALP"));
  m_dev_type.SetCurSel(0);

  mGray.SetCurSel(0);
  OnEnChangeEdit5();
  m_ExSlider.SetRange(0, 3000);//设置曝光时间

  return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

CString CWLPDMDTesterDlg::GetModulePath()
{
  HMODULE t_mudule = GetModuleHandle(0);
  CString pfileName;
  GetModuleFileName(t_mudule, pfileName.GetBufferSetLength(MAX_PATH), MAX_PATH);
  pfileName.ReleaseBuffer();
  int nPos = pfileName.ReverseFind('\\');
  if (nPos < 0)
    return L"";
  else
    return pfileName.Left(nPos);
}

void CWLPDMDTesterDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWLPDMDTesterDlg::OnPaint()
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
HCURSOR CWLPDMDTesterDlg::OnQueryDragIcon()
{
  return static_cast<HCURSOR>(m_hIcon);
}

void CWLPDMDTesterDlg::OnBnClickedSavepointbtn()
{
  UpdateData();
  if (m_StrPath == L"")
  {
    CBrowseFolderDialog BrowseFolder;
    m_StrPath = BrowseFolder.BrowseFolder();
    if (!BuildDirectory(m_StrPath))
      return;
    UpdateData(false);
  }
  CWaitCursor wc;
  IPatternGenerate* pPatterPoint = mDMDManager->CreateDmdPattern(PATTERN_POINT);
  pPatterPoint->SetSave(true, m_StrPath);
  int w(0), h(0);
  GetDMDSize(&w, &h);
  pPatterPoint->SetSize(w, h);
  PointPara mV;
  mV.iPtSize = mPointSize;
  mV.iPtPeriod = mPtSpace;
  mV.iPtExSize = mPtExtraW;
  pPatterPoint->Gernerate(&mV);//点扫

  int iCount = pPatterPoint->GetFrameCount();
  for (int i = 0; i < iCount; i++)
    pPatterPoint->GetData(i, iGray);
  mDMDManager->DestroyDmdPattern(pPatterPoint);
  CString str = L"Save point finished!";
  SetDlgItemText(IDC_EDITInfo, str);
}

void CWLPDMDTesterDlg::OnBnClickedSavelinebtn()
{
  UpdateData();
  if (m_StrPath == L"")
  {
    CBrowseFolderDialog BrowseFolder;
    m_StrPath = BrowseFolder.BrowseFolder();
    if (!BuildDirectory(m_StrPath))
      return;
    UpdateData(false);
  }
  CWaitCursor wc;
  IPatternGenerate* pPatterLine = mDMDManager->CreateDmdPattern(PATTERN_LINE);
  pPatterLine->SetSave(true, m_StrPath);
  int w(0), h(0);
  GetDMDSize(&w, &h);
  pPatterLine->SetSize(w, h);
  LinePara mV;
  mV.iLineW = mLineW;
  mV.iLType = miLineType;
  mV.iLPeriod = mLineCount;
  mV.iLExW = mLExtraW;
  if (miLineType == 0)
    pPatterLine->Gernerate(&mV);
  else
    pPatterLine->Gernerate(&mV);
  int iCount = pPatterLine->GetFrameCount();
  for (int i = 0; i < iCount; i++)
    pPatterLine->GetData(i, iGray);

  mDMDManager->DestroyDmdPattern(pPatterLine);
  CString str = L"Save line finished!";
  SetDlgItemText(IDC_EDITInfo, str);
}


void CWLPDMDTesterDlg::OnBnClickedInitbtn()
{
  UpdateData();
  mWlpDMDParas.m_FrameMargin = m_FrameMargin;
  mWlpDMDParas.m_FrameInterval = m_FrameInterval;
  mWlpDMDParas.delay = delay;
  switch (mGray.GetCurSel())//图像阶数
  {
  case 0:
    mWlpDMDParas.gray = 1;
    iGray = 1;
    break;
  case 1:
    mWlpDMDParas.gray = 8;
    iGray = 8;
    break;
  }
  mWlpDMDParas.plusewidth = plusewidth;
  mWlpDMDParas.polay = false;
  mWlpDMDParas.trigeSync = 0x00000008;//WLP_SYNC_SINGLE_FRAME;
  switch (m_trigger_mode.GetCurSel())//触发模式
  {
  case 0:
    mWlpDMDParas.trigeMode = 0x00000000;//WLP_TRIGGER_SINGLE_FRAME;
    break;
  case 1:
    mWlpDMDParas.trigeMode = 0x00000002;//WLP_TRIGGER_FRAMES;
    break;
  case 2:
    mWlpDMDParas.trigeMode = 0x00000004;//WLP_TRIGGER_FRAMES_AND_AUTOSTOP;
    break;
  case 3:
    mWlpDMDParas.trigeMode = 0x00000006;//WLP_TRIGGER_LOOP;
    break;
  }

  int dev_idx = m_dev_type.GetCurSel();

  bool success(false);
  if (mDMDManager )
  {
    mDMDManager->UnInitDMD();
    if (mDMDManager->InitDMD(&mWlpDMDParas, dev_idx))
    {
      success = true;
    }
  }
  if(success)
  {
    WlpDMDInfo* mInfo = mDMDManager->GetInfo();
    SetDlgItemText(IDC_EDITInfo, mInfo->strInfo);
  }
  else
  {
    CString str = L"DMD is opened fail!";
    SetDlgItemText(IDC_EDITInfo, str);
  }
}

void CWLPDMDTesterDlg::OnBnClickedOpenimgbtn()
{
  UpdateData();
  //if (!mDMDManager) return;

  CString strFile;
  LPBYTE buffer = 0;
  int width, height;
  int wbit;
  if (!OpenFile(&buffer, width, height, wbit, 0, strFile))
    return;
  OpenCVShowImg(buffer, width, height, wbit, "ReadImage", width > 1200 ? 0.5 : 1.0);

  if (iGray == 8)
  {
    int iW = -1, iH = -1;
    //mDMDManager->GetDownSize(&iW, &iH);
	GetDMDSize(&iW, &iH);
    LPBYTE bufferNew = (LPBYTE)malloc(iW*iH);
    mDMDManager->pixel2bin((char*)buffer, iW, iH, 0XFF, (char*)bufferNew);
    OpenCVShowImg(bufferNew, width, height, wbit, "pixel2binImage", width > 1200 ? 0.5 : 1.0);
	mDMDManager->GetDownSize(&iW, &iH);
    if (iH == height)
    {
	  if (mDMDManager)
	  {
		  if (mDMDManager->GetType() == 0)
			  mDMDManager->DownLoadFrame(bufferNew, 1);
		  else
			  mDMDManager->DownLoadFrame(buffer, 1);
	  }
    }
    else
    {
      AfxMessageBox(L"The image size mismatch!");
    }
	if (mDMDManager)
	{
		mDMDManager->bin2pixel((char*)bufferNew, iW, iH, 0XFF, (char*)buffer);
		OpenCVShowImg(buffer, width, height, wbit, "bin2pixelImage", width > 1200 ? 0.5 : 1.0);
	}
    free(buffer);
    free(bufferNew);
  }
  else
  {
    LPBYTE buffer2 = (LPBYTE)malloc(height * width / 8);
    ChangeFormate(width, height, buffer, buffer2);
	if (mDMDManager)
	{
		int iW = -1, iH = -1;
		mDMDManager->GetDownSize(&iW, &iH);
		if (iH == height)
			mDMDManager->DownLoadFrame(buffer2, 1);
		else
			AfxMessageBox(L"The image size mismatch!");
	}
    free(buffer2);
    free(buffer);
  }

  CString str = L" State：Have downloaded  picture: " + strFile;
  SetDlgItemText(IDC_EDITState, str);
}

void CWLPDMDTesterDlg::OnBnClickedDownpointbtn()
{
  GetDlgItem(IDC_DownPointBtn)->EnableWindow(false);
  UpdateData();
  int iW = -1, iH = -1;
  mDMDManager->GetDownSize(&iW, &iH);
  if (iW < 0)
    return;
  IPatternGenerate* pPatterPoint = mDMDManager->CreateDmdPattern(PATTERN_POINT);
  int w(0), h(0);
  GetDMDSize(&w, &h);
  pPatterPoint->SetSize(w, h);

  PointPara mV;
  mV.iPtSize = mPointSize;
  mV.iPtPeriod = mPtSpace;
  mV.iPtExSize = mPtExtraW;
  pPatterPoint->Gernerate(&mV);//点扫

  CString str;
  str.Format(L" State：Download %dX%d point picture(Total:%d)", mPointSize, mPtSpace, pPatterPoint->GetFrameCount());
  if (mDMDManager->DownLoadPattern(pPatterPoint))
  {
    str += L" success!";
    SetDlgItemText(IDC_EDITState, str);
  }
  else
  {
    str += L" failed!";
    SetDlgItemText(IDC_EDITState, str);
  }
  mDMDManager->DestroyDmdPattern(pPatterPoint);
  GetDlgItem(IDC_DownPointBtn)->EnableWindow(true);
}

void CWLPDMDTesterDlg::OnBnClickedDownlinetbtn()
{
  GetDlgItem(IDC_DownLinetBtn)->EnableWindow(false);
  UpdateData();
  int iW = -1, iH = -1;
  mDMDManager->GetDownSize(&iW, &iH);
  if (iW < 0)
    return;

  IPatternGenerate* pPatterLine = mDMDManager->CreateDmdPattern(PATTERN_LINE);
  int w(0), h(0);
  GetDMDSize(&w, &h);
  pPatterLine->SetSize(w, h);

  LinePara mV;
  mV.iLineW = mLineW;
  mV.iLType = miLineType;
  mV.iLPeriod = mLineCount;
  mV.iLExW = mLExtraW;
  pPatterLine->Gernerate(&mV);

  int iCount = pPatterLine->GetFrameCount();

  int SingleSize = iW * iH / 8 * iGray;
  int LargeImageBuffSize = SingleSize*iCount;
  unsigned char* pLargeImageBuff = (unsigned char*)malloc(sizeof(BYTE)*LargeImageBuffSize);
  for (int i = 0; i < iCount; i++)
    memcpy(pLargeImageBuff + SingleSize*i, pPatterLine->GetData(i, iGray), SingleSize);

  CString str;
  str.Format(L" State：Download %dX%d line picture(Total:%d)", mLineW, mLineCount, iCount);
  if (mDMDManager->DownLoadFrame(pLargeImageBuff, iCount))
  {
    str += L" success!";
    SetDlgItemText(IDC_EDITState, str);
  }
  else
  {
    str += L" failed!";
    SetDlgItemText(IDC_EDITState, str);
  }
  free(pLargeImageBuff);
  mDMDManager->DestroyDmdPattern(pPatterLine);
  GetDlgItem(IDC_DownLinetBtn)->EnableWindow(true);
}

void CWLPDMDTesterDlg::OnBnClickedStartbtn()
{
  mDMDManager->SetStatus(DMD_START);
}

void CWLPDMDTesterDlg::OnBnClickedTriggerbtn()
{
  mDMDManager->SetStatus(DMD_Trigger);
}


void CWLPDMDTesterDlg::OnBnClickedStopbtn()
{
  mDMDManager->SetStatus(DMD_STOP);
}


void CWLPDMDTesterDlg::OnBnClickedBrightbtn()
{
  UpdateData();
  int iW = -1, iH = -1;
  mDMDManager->GetDownSize(&iW, &iH);
  if (iW > 0)
  {
    int LargeImageBuffSize = iW * iH / 8 * iGray;
    unsigned char* pLargeImageBuff = (unsigned char*)malloc(sizeof(BYTE)*LargeImageBuffSize);
    memset(pLargeImageBuff, 255, LargeImageBuffSize);
    mDMDManager->DownLoadFrame(pLargeImageBuff, 1);
    free(pLargeImageBuff);
    CString str = L" State：Have downloaded All-On picture!";
    SetDlgItemText(IDC_EDITState, str);
  }
}

void CWLPDMDTesterDlg::OnBnClickedBlackbtn()
{
  int iW = -1, iH = -1;
  mDMDManager->GetDownSize(&iW, &iH);
  if (iW > 0)
  {
    int LargeImageBuffSize = iW * iH / 8 * iGray;
    unsigned char* pLargeImageBuff = (unsigned char*)malloc(sizeof(BYTE)*LargeImageBuffSize);
    memset(pLargeImageBuff, 0, LargeImageBuffSize);
    mDMDManager->DownLoadFrame(pLargeImageBuff, 1);
    free(pLargeImageBuff);
    CString str = L" State：Have downloaded All-off picture!";
    SetDlgItemText(IDC_EDITState, str);
  }
}

void CWLPDMDTesterDlg::OpenCVShowImg(LPBYTE pBuffer, int iW, int iH, int wBits, std::string strN, double scale)
{
  cv::Mat mImg;
  if (wBits == 24)
    mImg = cv::Mat(iH, iW, CV_8UC3);
  else
    mImg = cv::Mat(iH, iW, CV_8U);
  int iSize = iW*iH*wBits / 8;
  memcpy(mImg.data, pBuffer, iSize);
  resize(mImg, mImg, Size(iW * scale, iH * scale));
  cv::namedWindow(strN);
  cv::imshow(strN, mImg);
}

bool CWLPDMDTesterDlg::OpenFile(LPBYTE* pBuffer, int& IWidth, int& IHeight, int& wBitsPerPixel, int iColor, CString &strFile)
{
  CFileDialog dlg(true);
  dlg.m_ofn.lpstrTitle = _T("图片打开对话框");
  dlg.m_ofn.lpstrFilter = _T("All Files(*.*)\0*.*\0JPEG Files(*.jpg)\0*.jpg\0All Files(*.*)\0*.*\0\0");
  if (IDOK == dlg.DoModal())
  {
    int w(0), h(0);
    GetDMDSize(&w, &h);
    if (w <= 0 || h <= 0)
      return false;
    strFile = dlg.GetPathName();
    std::string tempName = (LPCSTR)CStringA(dlg.GetPathName());
    const char *tmp = tempName.c_str();
    Mat mImg = cvLoadImage(tmp, iColor);
    if (mImg.empty()) // 图像无效
    {
      return false;
    }
    if (mImg.cols != w || mImg.rows != h)
    {
      Size mSize = cv::Size(w, h);
      cv::resize(mImg, mImg, mSize);
    }
    IWidth = mImg.cols;
    IHeight = mImg.rows;
    wBitsPerPixel = mImg.channels() * 8;
    int iStep = mImg.step;
    int iSize = mImg.rows*iStep;
    *pBuffer = (LPBYTE)malloc(iSize);
    memcpy(*pBuffer, mImg.data, iSize);
    return true;
  }
  return false;
}

void CWLPDMDTesterDlg::OnBnClickedUpdatebtn()
{
  UpdateData();
  mWlpDMDParas.m_FrameMargin = m_FrameMargin;
  mWlpDMDParas.m_FrameInterval = m_FrameInterval;
  mWlpDMDParas.delay = delay;
  switch (mGray.GetCurSel())//图像阶数
  {
  case 0:
    mWlpDMDParas.gray = 1;
    iGray = 1;
    break;
  case 1:
    mWlpDMDParas.gray = 8;
    iGray = 8;
    break;
  }
  mWlpDMDParas.plusewidth = plusewidth;
  mWlpDMDParas.polay = false;
  mWlpDMDParas.trigeSync = 0x00000008;//WLP_SYNC_SINGLE_FRAME;
  switch (m_trigger_mode.GetCurSel())//触发模式
  {
  case 0:
    mWlpDMDParas.trigeMode = 0x00000000;//WLP_TRIGGER_SINGLE_FRAME;
    break;
  case 1:
    mWlpDMDParas.trigeMode = 0x00000002;//WLP_TRIGGER_FRAMES;
    break;
  case 2:
    mWlpDMDParas.trigeMode = 0x00000004;//WLP_TRIGGER_FRAMES_AND_AUTOSTOP;
    break;
  case 3:
    mWlpDMDParas.trigeMode = 0x00000006;//WLP_TRIGGER_LOOP;
    break;
  }
  if (mDMDManager->RestPara(&mWlpDMDParas))
  {
    CString str = L" State: Update success!";
    SetDlgItemText(IDC_EDITState, str);
  }
  else
  {
    CString str = L" State: Update failed!";
    SetDlgItemText(IDC_EDITState, str);
  }
}

int CWLPDMDTesterDlg::OnSubjectNotified(IPtnSubject * pSubject, int ID, long wParam, void* pParam, float fParam, void* mParam)
{
  if (ID == Notify_WLPDMD_OPEN)
  {
    GetDlgItem(IDC_BrightBtn)->EnableWindow(true);
    GetDlgItem(IDC_BlackBtn)->EnableWindow(true);
    GetDlgItem(IDC_UpdateBtn)->EnableWindow(true);
    GetDlgItem(IDC_OpenImgBtn)->EnableWindow(true);
    GetDlgItem(IDC_DownPointBtn)->EnableWindow(true);
    GetDlgItem(IDC_DownLinetBtn)->EnableWindow(true);
    GetDlgItem(IDC_StartBtn)->EnableWindow(true);
  }
  else if (ID == Notify_WLPDMD_STOP)
  {
    GetDlgItem(IDC_InitBtn)->EnableWindow(true);
    GetDlgItem(IDC_BrightBtn)->EnableWindow(true);
    GetDlgItem(IDC_BlackBtn)->EnableWindow(true);
    GetDlgItem(IDC_UpdateBtn)->EnableWindow(true);
    GetDlgItem(IDC_OpenImgBtn)->EnableWindow(true);
    GetDlgItem(IDC_DownPointBtn)->EnableWindow(true);
    GetDlgItem(IDC_DownLinetBtn)->EnableWindow(true);
    GetDlgItem(IDC_StartBtn)->EnableWindow(true);
    GetDlgItem(IDC_TriggerBtn)->EnableWindow(false);
    GetDlgItem(IDC_StopBtn)->EnableWindow(false);
  }
  else if (ID == Notify_WLPDMD_START)
  {
    GetDlgItem(IDC_InitBtn)->EnableWindow(false);
    GetDlgItem(IDC_BrightBtn)->EnableWindow(false);
    GetDlgItem(IDC_BlackBtn)->EnableWindow(false);
    GetDlgItem(IDC_UpdateBtn)->EnableWindow(false);
    //GetDlgItem(IDC_OpenImgBtn)->EnableWindow(false);
    GetDlgItem(IDC_DownPointBtn)->EnableWindow(false);
    GetDlgItem(IDC_DownLinetBtn)->EnableWindow(false);
    GetDlgItem(IDC_StartBtn)->EnableWindow(false);
    GetDlgItem(IDC_TriggerBtn)->EnableWindow(true);
    GetDlgItem(IDC_StopBtn)->EnableWindow(true);
  }
  return 0;
}

void CWLPDMDTesterDlg::OnEnChangeEdit5()
{
  // TODO:  如果该控件是 RICHEDIT 控件，它将不
  // 发送此通知，除非重写 __super::OnInitDialog()
  // 函数并调用 CRichEditCtrl().SetEventMask()，
  // 同时将 ENM_CHANGE 标志“或”运算到掩码中。

  UpdateData();
  float mCameraScanTime = 0.0f;
  if (miResolution == 0)
    mCameraScanTime = 0.0097 * 2048 / 2;
  else if (miResolution == 1)
    mCameraScanTime = 0.0097 * 1024 / 2;
  else if (miResolution == 2)
    mCameraScanTime = 0.0097 * 512 / 2;
  float iV = (1000000.0f) / (m_FrameMargin + m_FrameInterval + 0.0f);
  CString str;
  float iExpose = (m_FrameMargin + m_FrameInterval) / 1000.0f - mCameraScanTime;
  str.Format(L"Frequence:%.2fHz;ExposeTime:%.1fms", iV, iExpose);
  GetDlgItem(IDC_EDITFRE)->SetWindowText(str);
  TRACE("更换选择图像分辨率!\n");
}

void CWLPDMDTesterDlg::CalDMDTime()
{
  UpdateData();
  float mCameraScanTime = 0.0f;
  if (miResolution == 0)
    mCameraScanTime = 0.0097 * 2048 / 2;
  else if (miResolution == 1)
    mCameraScanTime = 0.0097 * 1024 / 2;
  else if (miResolution == 2)
    mCameraScanTime = 0.0097 * 512 / 2;
  m_FrameInterval = 0;
  m_FrameMargin = (m_Vexposure + mCameraScanTime + 0.1)*1000.0f + 100;//累加0.1ms，并且m_FrameMargin也要再累加100，这样才可以保证采集频率跟上
  UpdateData(false);
}

void* CWLPDMDTesterDlg::LoadDlls(CString t_name)
{
  HMODULE t_mudule = GetModuleHandle(0);
  CString pfileName;
  GetModuleFileName(t_mudule, pfileName.GetBufferSetLength(MAX_PATH), MAX_PATH);
  pfileName.ReleaseBuffer();
  int nPos = pfileName.ReverseFind('\\');
  pfileName = pfileName.Left(nPos);
  pfileName += L"\\";
  pfileName += t_name;
  HMODULE t_module;
  t_module = ::LoadLibrary(pfileName);
  if (t_module == NULL)
    return 0;
  PluginLib_GetInterface t_GetInterface;
  PluginLib_GetName		t_GetName;
  t_GetInterface = (PluginLib_GetInterface)::GetProcAddress(t_module, "HGetPluginInterface");
  t_GetName = (PluginLib_GetName)::GetProcAddress(t_module, "HGetPluginName");
  CString tname = t_GetName();
  TRACE(tname);
  void* t = t_GetInterface();
  return t;
}



void CWLPDMDTesterDlg::OnBnClickedChecksave()
{
  if (((CButton*)GetDlgItem(IDC_CHECKSave))->GetCheck())
  {
    CBrowseFolderDialog BrowseFolder;
    m_StrPath = BrowseFolder.BrowseFolder(GetSafeHwnd());
    BuildDirectory(m_StrPath);
    UpdateData(false);
  }
}

bool CWLPDMDTesterDlg::BuildDirectory(CString strPath)
{
  if (strPath.GetLength() <= 0)
  {
    return false;
  }
  CString strSubPath;
  CString strInfo;
  int nCount = 0;
  int nIndex = 0;
  do
  {
    nIndex = strPath.Find(L"\\", nIndex) + 1;
    nCount++;
  } while ((nIndex - 1) != -1);
  nIndex = 0;
  while ((nCount - 1) >= 0)
  {
    nIndex = strPath.Find(L"\\", nIndex) + 1;
    if ((nIndex - 1) == -1)
    {
      strSubPath = strPath;
    }
    else
      strSubPath = strPath.Left(nIndex);
    if (!PathFileExists(strSubPath))
    {
      if (!::CreateDirectory(strSubPath, NULL))
      {
        strInfo = L"Build Directory";
        strInfo += strSubPath;
        strInfo += L" Fail!";
        AfxMessageBox(strInfo, MB_OK);
        return FALSE;
      }
    }
    nCount--;
  };
  return TRUE;
}

char* CWLPDMDTesterDlg::pixel2bin(char* pSrc, int width, int height, unsigned char bitmask, char* pOut)
{
  if (pSrc == NULL || pOut == NULL || width % 8 != 0)
    return NULL;
  char* ret = pOut;
  int cnt = (width / 8)*height;  //once operate 8bytes.
  unsigned char bm;
  int i, j, k, m;
  char c;
  UINT64 temp;
  UINT64 mark[8];
  for (i = 0; i < 8; i++)
  {
    temp = 0x80 >> i;
    mark[i] = 0;
    for (int j = 0; j < 8; j++)
      mark[i] |= temp << j * 8;
  }
  UINT64* p = (UINT64*)pSrc;
  for (i = 0; i < cnt; i++)
  {
    for (j = 0, m = 0, bm = bitmask; j < 8 && (bm & 0x80); j++, bm <<= 1)
    {	// get  A7,B7,C7,D7,E7,F7,G7,H7 (most high bit), then A6,B6,C6..H6.
      c = 0;
      temp = *p & mark[j];
      temp >>= (7 - j);
      for (k = 0; k < 8; k++)
      {
        c |= ((temp & 0x01) << k);
        temp >>= 8;
      }
      pOut[m * cnt] = c;
      m++;
    }
    pOut++;
    p++;
  }
  return ret;
}

char* CWLPDMDTesterDlg::bin2pixel(char* pSrc, int width, int height, unsigned char bitmask, char* pOut)
{
  if (pOut == NULL || pSrc == NULL || width % 8 != 0)
    return NULL;
  char* ret = pOut;
  int cnt = (width / 8)*height;  //once operate 8bytes.
  unsigned char bm;
  int i, j, k, m;
  UINT64 c;
  UINT64 temp;
  UINT64* p = (UINT64*)pOut;
  for (i = 0; i < cnt; i++)
  {
    temp = 0;
    for (j = 0, m = 0, bm = bitmask; j < 8 && (bm & 0x80); j++, bm <<= 1)  //j means which bit
    {
      c = (UINT64) *(pSrc + cnt*m);
      m++;
      for (k = 0; k < 8; k++) //which pixel
      {
        temp |= (c & 0x01) << (k * 8 + 7 - j);
        c >>= 1;
      }
    }
    pSrc++;
    *p++ = temp;
  }
  return ret;
}

void CWLPDMDTesterDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  m_Vexposure = m_ExSlider.GetPos();
  m_Vexposure = m_Vexposure / 10;
  UpdateData(false);
  CalDMDTime();
  OnEnChangeEdit5();
  __super::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CWLPDMDTesterDlg::OnEnChangeEdit1()
{
  // TODO:  如果该控件是 RICHEDIT 控件，它将不
  // 发送此通知，除非重写 __super::OnInitDialog()
  // 函数并调用 CRichEditCtrl().SetEventMask()，
  // 同时将 ENM_CHANGE 标志“或”运算到掩码中。

  UpdateData();
  mPtExtraW = mPointSize;
  UpdateData(false);
}


void CWLPDMDTesterDlg::OnEnChangeEdit3()
{
  // TODO:  如果该控件是 RICHEDIT 控件，它将不
  // 发送此通知，除非重写 __super::OnInitDialog()
  // 函数并调用 CRichEditCtrl().SetEventMask()，
  // 同时将 ENM_CHANGE 标志“或”运算到掩码中。

  UpdateData();
  mLExtraW = mLineW;
  UpdateData(false);
}

void CWLPDMDTesterDlg::OnCbnSelchangeCombo1()
{
  UpdateData();
  switch (mGray.GetCurSel())//图像阶数
  {
  case 0:
    mWlpDMDParas.gray = 1;
    iGray = 1;
    m_FrameInterval = 0;
    break;
  case 1:
    mWlpDMDParas.gray = 8;
    iGray = 8;
    m_FrameInterval = 150;
    break;
  }
  UpdateData(false);
}

void CWLPDMDTesterDlg::GetDMDSize(int* width, int* height)
{
  if (mDMDManager)
  {
    mDMDManager->GetDownSize(width, height);
    if (width && *width <= 0)
    {
      *width = 1920;
    }
    if (height && *height <= 0)
    {
      *height = 1080;
    }
  }
  else
  {
    if (width)
    {
      *width = 1920; // 1024
    }
    if (height)
    {
      *height = 1080; // 768
    }
  }
}