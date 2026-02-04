#pragma once
#include "afxwin.h"
#include "Resource.h"
#include "HConfocalCore.h"
#include "CarlVideo.h"
#include <Gdiplus.h>
using namespace Gdiplus;

class ViewScan:public CWnd,public HCorePanel//,public HVideoRender
{
public:
	ViewScan(void);
	~ViewScan(void);
	//afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	virtual bool InitPanel(LPVOID p_Param, LPVOID p_Param2 = 0);
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0,float fParam=0.0f,
		void* mParam=0);
	virtual CWnd* GetCWnd() {return this;};

private:
	HConfocalCore* m_ConfocalCore;//核心
	BITMAPINFO* m_bmi;//调色板
	//以下gdi
	ULONG_PTR _gdiplusToken;
	FontFamily* fontFamily;
	Gdiplus::Font* font;
	LOGFONT lf;
	SolidBrush* brush;
	CRect rcClient;
	StringFormat stringformat;

	long iDestLeft,iDestTop,iDestW,iDestH;
	float ZoomAmp;//放大缩小的倍率
	long	m_hPos;
	long	m_vPos;
	long    m_nHOffset;//这两个是设置图片1:1显示的时候，横向和纵向应该给的位移
	long    m_nVOffset;

	void DoubleDCFun( CDC* pDC,CRect pRc );
	void DrawFunctionM2(CDC* pDC,CRect pRc);
	void DrawFunction(CDC* pDC,CRect pRc);
	void DrawCurImg(CDC* pDC,CRect pRc);

	void UpdataView(int pHpos,int pVpos );
	void update_scrollbar( int width,int height );

	HVideoHeader hFlushHeader;
	HCoreProcess* m_ReBuildPro;
	int iCur,iTotal;//当前显示的以及总共的图像总数

	bool m_bFitWndows;
	bool bFlush;

	void OnEpiFitwindows();
	void OnEpiSaveframe();
	void OnEpiReleaseImg();

public:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
//	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
//	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
//	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
//	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
};

