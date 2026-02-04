#pragma once
#include "resource.h"
#include "afxwin.h"
#include <Gdiplus.h>
using namespace Gdiplus;


const long USER_MESSAGE = 10000;
// SplashPanel 对话框

class SplashPanel : public CDialog
{
	DECLARE_DYNAMIC(SplashPanel)
	CString _path;
	ULONG_PTR _gdiplusToken;
	Bitmap *_bmp;
	long _width,_height;
public:
	SplashPanel(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~SplashPanel();
	void SetPath(CString path);//(const char *path);
	void SetString(const char *str);
// 对话框数据
	enum { IDD = IDD_DLGFLASH };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	CString _txt;
	CStatic _txtCtrl;
};
