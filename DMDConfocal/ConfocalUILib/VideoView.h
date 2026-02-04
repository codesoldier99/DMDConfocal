#pragma once
#include "afxwin.h"
#include "Resource.h"
#include "HConfocalCore.h"
#include "CarlVideo.h"
#include <Gdiplus.h>
using namespace Gdiplus;

class CVideoView :public CWnd,public HCorePanel,public HVideoRender
{
public:
	CVideoView(void);
	~CVideoView(void);
	//void set_zoom( double fZoomH, double fZoomV );
	//void get_zoom( double& fZoomH, double& fZoomV );
	void Render(LPBYTE pBuffer, int width,int heidth,int wbit );
	virtual CWnd* GetCWnd() {return this;};
	virtual bool InitPanel(LPVOID p_Param, LPVOID p_Param2 = 0);
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);
  void EnableAlarm(bool enable);
protected:
	HConfocalCore* m_ConfocalCore;//核心
	BITMAPINFO* m_bmi;//调色板
	void DoubleDCFun(CDC* pDC,CRect pRc);//双缓冲绘图函数
	void DrawFunction(CDC* pDC,CRect pRc);//普通绘图
	void DrawFunctionM2(CDC* pDC,CRect pRc);//满窗口绘图
	void DrawRuler(CDC* pDC,CPoint p_x1,CPoint p_x2);
	void DrawFps(CDC* pDC,CRect pRc,float pFps);
	//以下gdi
	ULONG_PTR _gdiplusToken;
	FontFamily* fontFamily;
	Gdiplus::Font* font;
	LOGFONT lf;
	SolidBrush* brush;
	CRect rcClient;
	StringFormat stringformat;

	bool m_bFitWndows;//是满屏显示与否
	LPBYTE		m_Buffer;//绘制的图像内存
	long iDestLeft,iDestTop,iDestW,iDestH;
	void update_scrollbar(int width,int height);//在里面设置滚动条以及m_nHOffset、m_nVOffset
	long    m_nHOffset;//这两个是设置图片1:1显示的时候，横向和纵向应该给的位移
	long    m_nVOffset;
	void UpdataView(int pHpos,int pVpos );//根据m_nHOffset和m_nVOffset来设置滚动条的位置m_hPos,m_Vpos
	long	m_hPos;//m_hPos,m_Vpos是滚动条的滚后位置
	long	m_vPos;
	bool	m_bDragMouse;//确定数并左键正按住

	float m_CurPixelSize;
	float ZoomAmp;//放大缩小的倍率
	CPoint m_MouseX;//鼠标起始点
	CPoint m_MouxeY;//鼠标终结点

	DWORD m_FpsLastTime;//上一副图的到达时间
	float m_FpsLastValue;//上一副图的采集速率
	HAxis *m_AxisX,*m_AxisY,*m_AxisZ;

	bool bSetCalCV;//是否使用CV值校正
	HVideoHeader m_CaptureVideo;
  bool m_alarm;

  CRITICAL_SECTION m_cs;
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnEpiFitwindows();
	afx_msg void OnEpiSaveframe();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnVideoviewAlarm();
private:
	bool m_bExType;//是否有额外的相机
};

