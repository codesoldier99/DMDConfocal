#pragma once

#include "afxwin.h"
#include "Resource.h"
#include "HConfocalCore.h"
#include "CarlVideo.h"

#include <Gdiplus.h>
using namespace Gdiplus;

class CMergeView:public CWnd,public HCorePanel,public HVideoRender
{
public:
	CMergeView(void);
	~CMergeView(void);

	virtual bool InitPanel(LPVOID p_Param, LPVOID p_Param2 = 0);
	void UpdataView(int pHpos,int pVpos );
	void Render(LPBYTE pBuffer, int width,int heidth,int wbit );
	virtual CWnd* GetCWnd() {return this;};
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);

protected:
	bool m_bFitWndows;//是满屏显示与否
	HConfocalCore* m_ConfocalCore;//核心

	BITMAPINFO* m_bmi;
	LPBYTE		m_Buffer;
	LPBYTE		m_LeftTop;
	RGBQUAD*    m_Quad;
	double ZoomWidth,ZoomHeight;
	long		m_hPos;
	long		m_vPos;
	bool		m_bUpdata;
	bool		m_bDragMouse;

	float ZoomAmp;
	CPoint m_MouseX;
	CPoint m_MouxeY;

	DWORD m_FpsLastTime;
	float m_FpsLastValue;

	long    m_nHOffset;
	long    m_nVOffset;
	void update_scrollbar(int width,int height);
	void DrawFunction(CDC* pDC,CRect pRc);//绘图处
	void DrawFunctionM2(CDC* pDC,CRect pRc);
	void DoubleDCFun(CDC* pDC,CRect pRc);//双缓冲准备
	bool DownloadFrame(LPBYTE pBuffer);
	void DrawRuler(CDC* pDC,CPoint p_x1,CPoint p_x2);
	void DrawFps(CDC* pDC,CRect pRc,float pFps);
	////////////////以下是GDI相关参数////////////////
	ULONG_PTR _gdiplusToken;
	FontFamily* fontFamily;
	Gdiplus::Font* font;
	LOGFONT lf;
	SolidBrush* brush;
	CRect rcClient;
	StringFormat stringformat;
public:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnModeStartconfocal();
	afx_msg void OnModeSaveframe();
	afx_msg void OnModeStopconfocal();
	afx_msg void OnModeSaveprocessimg();
	afx_msg void OnFITWINDOW();

private:
	bool m_bExType;
};

