#pragma once

#include "afxwin.h"
#include "CarlVideo.h"
#include <Gdiplus.h>
using namespace Gdiplus;

class CRenderWnd :public  CWnd,public HVideoRender
{
public:
	CRenderWnd(void);
	~CRenderWnd(void);
	void set_zoom( double fZoomH, double fZoomV );
	void get_zoom( double& fZoomH, double& fZoomV );
	void UpdataView(int pHpos,int pVpos );
	void Render(LPBYTE pBuffer, int width,int heidth,int wbit );
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	void SetDrawable(bool bDrawV);

protected:
	BITMAPINFO* m_bmi;
	LPBYTE		m_Buffer;
	LPBYTE		m_LeftTop;
	RGBQUAD*    m_Quad;
	double ZoomWidth,ZoomHeight;
	long		m_hPos;
	long		m_vPos;
	bool		m_bUpdata;
	long    m_nHOffset;
	long    m_nVOffset;
	void update_scrollbar(int width,int height);

public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CRect mROIRealRec;

private:
	bool m_bFitWndows;//是满屏显示与否
	long iDestLeft,iDestTop,iDestW,iDestH;
	void DrawFunctionM2(CDC* pDC,CRect pRc);
	void DrawFunction(CDC* pDC,CRect pRc);
	void DrawFps(CDC* pDC,CRect pRc,float pFps);
	void DrawROIS( CDC* pDC);

	CPoint m_ShowSize,m_RealSIze;
	float ScaleValue;//缩放比例
	CPoint m_DrawROIPt,m_DrawROISize,m_DrawROIEndPt;//扫描的区域ROI
	bool bDrawRec;//绘制ROI区域
	bool bDraw;
	ULONG_PTR _gdiplusToken;
	FontFamily* fontFamily;
	Gdiplus::Font* font;
	LOGFONT lf;
	SolidBrush* brush;
	StringFormat stringformat;

	DWORD m_FpsLastTime;//上一副图的到达时间
	float m_FpsLastValue;//上一副图的采集速率
	float ZoomAmp;//放大缩小的倍率

public:
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnEpiFitwindows();
	afx_msg void OnEpiSaveframe();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};

