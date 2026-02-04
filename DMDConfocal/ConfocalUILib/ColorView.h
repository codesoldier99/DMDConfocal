#pragma once

#include "afxwin.h"
#include "Resource.h"
#include "HConfocalCore.h"
#include "CarlVideo.h"

#include <Gdiplus.h>
using namespace Gdiplus;

class CColorView:public CWnd,public HCorePanel,public HVideoRender
{
public:
	CColorView(void);
	~CColorView(void);
	virtual bool InitPanel(LPVOID p_Param, LPVOID p_Param2 = 0){return false;};
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	virtual CWnd* GetCWnd() {return this;};
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);

private:
	void Render(LPBYTE pBuffer, int width,int heidth,int wbit);
	void update_scrollbar( int width,int height );
	void UpdataView(int pHpos,int pVpos);
	void DrawFunction(CDC* pDC,CRect pRc);//绘图处
	void DoubleDCFun(CDC* pDC,CRect pRc);//双缓冲准备

	BITMAPINFO* m_bmi;
	LPBYTE		m_Buffer;

	long    m_nHOffset;
	long    m_nVOffset;
	long		m_hPos;
	long		m_vPos;

	//以下gdi
	ULONG_PTR _gdiplusToken;
	FontFamily* fontFamily;
	Gdiplus::Font* font;
	LOGFONT lf;
	SolidBrush* brush;
	CRect rcClient;
	StringFormat stringformat;
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

