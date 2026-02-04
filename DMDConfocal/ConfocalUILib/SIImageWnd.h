#pragma once

#include <Gdiplus.h>
using namespace Gdiplus;
#include "resource.h"

class SIImageWnd :public CWnd, public HCorePanel, public HVideoRender
{
public:
  SIImageWnd(void);
  ~SIImageWnd(void);
  virtual CWnd* GetCWnd() { return this; };
  virtual bool InitPanel(LPVOID p_Param,LPVOID p_Param2=0);
  virtual	int	OnSubjectNotified(IPtnSubject * pSubject, int ID, long wParam = 0, void* pParam = 0
    , float fParam = 0.0f, void* mParam = 0);

  void UpdataView(int pHpos, int pVpos);
  void Render(LPBYTE pBuffer, int width, int heidth, int wbit);
  virtual void Renderer(HVideoHeader* pHeader, LPBYTE pBuffer);
  void SetDrawable(bool bDrawV);

  CRect mROIRealRec;

  void SetDiffData(int w, int h, unsigned char* a,
    unsigned char* b, unsigned char* c);
protected:
  BITMAPINFO* m_bmi;
  LPBYTE		m_Buffer;
  LPBYTE		m_LeftTop;
  RGBQUAD*    m_Quad;
  double ZoomWidth, ZoomHeight;
  long		m_hPos;
  long		m_vPos;
  bool		m_bUpdata;
  long    m_nHOffset;
  long    m_nVOffset;
  void update_scrollbar(int width, int height);

public:
  virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
  DECLARE_MESSAGE_MAP()
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

private:
  bool m_bFitWndows;//是满屏显示与否
  long iDestLeft, iDestTop, iDestW, iDestH;
  void DrawFunctionM2(CDC* pDC, CRect pRc);
  void DrawFunction(CDC* pDC, CRect pRc);
  void DrawROIS(CDC* pDC);
  void CorrectPt(CPoint* mPT);

  CPoint m_ShowSize, m_RealSIze, m_ShowPt;//地图的大小
  float ScaleValue;//缩放比例
  CPoint m_DrawROIPt, m_DrawROISize, m_DrawROIEndPt;//扫描的区域ROI
  bool bDrawRec;//绘制ROI区域
  bool bDraw;
  ULONG_PTR _gdiplusToken;
  FontFamily* fontFamily;
  Gdiplus::Font* font;
  LOGFONT lf;
  SolidBrush* brush;
  StringFormat stringformat;

  void FlushView();
  HConfocalCore* m_ConfocalCore;

  int _w;
  int _h;
  unsigned char* _imgs[3];
  int _cur_img_idx;
public:
  afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnFitwindows();
  afx_msg void OnSaveframe();
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnImageDefocusa();
  afx_msg void OnImageFocus();
  afx_msg void OnImageDefocusb();
};


