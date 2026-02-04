// SplashPanel.cpp : 实现文件
//

#include "stdafx.h"
#include "SplashPanel.h"

// SplashPanel 对话框

IMPLEMENT_DYNAMIC(SplashPanel, CDialog)

SplashPanel::SplashPanel(CWnd* pParent /*=NULL*/)
	: CDialog(SplashPanel::IDD, pParent)
	, _bmp(NULL)
	, _width(0)
	, _height(0)
	, _txt(_T(""))
{
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL);
}

SplashPanel::~SplashPanel()
{
	if(_bmp != NULL)
	{
		delete _bmp;
	}

	GdiplusShutdown(_gdiplusToken);
}

void SplashPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Text(pDX, IDC_STATIC_TEXT, _txt);
	//DDX_Control(pDX, IDC_STATIC_TEXT, _txtCtrl);
}


BEGIN_MESSAGE_MAP(SplashPanel, CDialog)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// SplashPanel 消息处理程序

void SplashPanel::SetPath(CString path)//(const char *path)
{
	if(GetSafeHwnd() == NULL || path == "")
	{
		return;
	}
	_path = path;
	USES_CONVERSION;
	_bmp = new Bitmap(path);//A2CW(_path.GetBuffer()));
	_width = _bmp->GetWidth();
	_height = _bmp->GetHeight();
	PostMessage(USER_MESSAGE);
}


void SplashPanel::SetString(const char *str)
{
	if(GetSafeHwnd() == NULL || str == NULL)
	{
		return;
	}
	_txt = str;
	PostMessage(USER_MESSAGE + 1);
}


BOOL SplashPanel::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void SplashPanel::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialog::OnPaint()
	if(_bmp != NULL)
	{
		Graphics gra(dc);
		// 绘制边框
		int border = 1;
		CRect r;
		GetClientRect(&r);
		Pen pen(Color::Black);
		gra.DrawRectangle(&pen, 0, 0, r.Width() - border, r.Height() - border);
		//gra.DrawImage(_bmp,Point(0,0));//具备缩放功能
		int gap = border;
		gra.DrawImage(_bmp, gap, gap, _width - gap * 2,_height - gap * 2);
	}
}

LRESULT SplashPanel::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	if(message == USER_MESSAGE)
	{
		CRect rc;
		GetWindowRect(rc);
		SetWindowPos(0,rc.left,rc.top,_width,_height + 30,SWP_NOZORDER);
		CenterWindow();
	}
	else if(message == USER_MESSAGE + 1)
	{
		CRect rcTxt;
		//_txtCtrl.GetWindowRect(rcTxt);
		int border = 1;
		//_txtCtrl.SetWindowPos(0,5,_height + 5,_width - border - 10,rcTxt.Height(),SWP_NOZORDER);
		//_txtCtrl.SetWindowTextA(_txt);
	}
	return CDialog::WindowProc(message, wParam, lParam);
}
