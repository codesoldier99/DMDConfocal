#pragma once
#include "afxwin.h"

#include "GdiPlus.h"
using namespace Gdiplus;
#include <vector>

class CSplashWnd :public CWnd
{

public:
	CSplashWnd(void);
	~CSplashWnd(void);

public:
	void Show(CString p_Res=L".\\splash.png");//显示窗口
	void Print(CString p_msg);//打印消息
	void DestroyWnd(void);//结束

private:
	ULONG_PTR m_gdiplusToken;
	Image* m_BkImg;
	BLENDFUNCTION m_Blend;
	int m_width,m_height;
	std::vector<CString> m_Message;
	bool m_bInited;
	//gdi++
	//以下gdi
	ULONG_PTR _gdiplusToken;
	FontFamily* fontFamily;
	Gdiplus::Font* font;
	LOGFONT lf;
	SolidBrush* brush;
	CRect rcClient;
	StringFormat stringformat;

protected:
	void DrawObject(Graphics &p_graph);

public:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual void PostNcDestroy();
};

