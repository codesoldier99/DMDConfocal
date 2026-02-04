// HeatmapWnd.cpp : implementation file
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "HeatmapWnd.h"
// HeatmapWnd
#include "HGloableFunction.h"
#include <Mmsystem.h>            
#pragma comment(lib, "Winmm.lib") 
#include "resource.h"

BEGIN_MESSAGE_MAP(HeatmapWnd, CWnd)
  ON_WM_ERASEBKGND()
  ON_WM_SIZE()
  ON_WM_TIMER()
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONUP()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_COMMAND(ID_D_FITWINDOW, &HeatmapWnd::OnEpiFitwindows)
  ON_COMMAND(ID_D_SAVEFRAME, &HeatmapWnd::OnEpiSaveframe)
  ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

HeatmapWnd::HeatmapWnd(void)
{
  //初始化调色板
  m_bmi = (BITMAPINFO*)malloc(sizeof(BITMAPINFO) + 256 * 4);
  memset(m_bmi, 0, sizeof(BITMAPINFO) + 256 * 4);
  m_bmi->bmiHeader.biPlanes = 1;
  m_bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  for (int i = 0; i<256; i++)//调色板
  {
    m_bmi->bmiColors[i].rgbBlue = i;
    m_bmi->bmiColors[i].rgbGreen = i;
    m_bmi->bmiColors[i].rgbRed = i;
    m_bmi->bmiColors[i].rgbReserved = 0;
  }
  m_nHOffset = 0;
  m_nVOffset = 0;
  //以下gdi
  GdiplusStartupInput gdiplusStartupInput;
  GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL);
  fontFamily = new FontFamily(L"Arial");//L"幼圆"); 
  font = new Gdiplus::Font(fontFamily, 20, FontStyleRegular, UnitPixel);
  ::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
  memcpy(lf.lfFaceName, "Arial", 5);
  lf.lfHeight = 13;
  //brush=new SolidBrush(Color(255,0x2A,0x77,188));  
  brush = new SolidBrush(Color(255, 0x00, 181, 4));
  stringformat.SetAlignment(StringAlignmentCenter);
  stringformat.SetLineAlignment(StringAlignmentCenter);
  //变量初始化
  m_bFitWndows = true;
  m_Buffer = 0;
  m_hPos = 0;
  m_vPos = 0;
  ZoomAmp = 1;
  m_bDragMouse = false;
  m_MouseX.SetPoint(0, 0);
  m_MouseY.SetPoint(0, 0);
  m_FpsLastValue = 0.0;
  bSetCalCV = false;

  m_CaptureVideo.Vsize = 0;
  m_CaptureVideo.Vbuffer = 0;
  m_AxisX = 0;
  m_AxisY = 0;
  m_AxisZ = 0;

  InitializeCriticalSection(&m_cs);
}

HeatmapWnd::~HeatmapWnd(void)
{
  EnterCriticalSection(&m_cs);
  if (m_Buffer)
  {
    delete(m_Buffer);
    m_Buffer = 0;
  }
  LeaveCriticalSection(&m_cs);
  delete(m_bmi);
  GdiplusShutdown(_gdiplusToken);

  DeleteCriticalSection(&m_cs);
}

BOOL HeatmapWnd::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
  // TODO: 在此添加专用代码和/或调用基类
  //this->SetWindowTextW(L"Real View");  窗口还没创建成功，没办法赋值窗口名称
  return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

bool HeatmapWnd::InitPanel(LPVOID p_Param, LPVOID p_Param2)
{
  //m_ConfocalCore = (HConfocalCore*)p_Param;
  //m_ConfocalCore->GetRenderChain()->push_back(this);
  //if (m_ConfocalCore->GetGearBox())
  //{
  //  m_AxisX = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_X);
  //  m_AxisY = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Y);
  //  m_AxisZ = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Z);
  //}

  //CString strV = m_ConfocalCore->GetConfigure()->GetString(L"Object", L"curobj", L"Confocal_Microscope");
  //strV += L"_mm2pixel";
  //m_CurPixelSize = m_ConfocalCore->GetConfigure()->GetDouble(L"PixelSize", strV, L"Confocal_Microscope");

  SetTimer(529, 100, NULL);// 刷新进程
  return true;
}

void HeatmapWnd::Renderer(HVideoHeader* pHeader, LPBYTE pBuffer)
{
  int width = pHeader->Vwidth;
  int heidth = pHeader->Vheight;
  int wbit = pHeader->VwBit;
  if (!pBuffer)
    return;

  if (bSetCalCV)
  {
    Cal_TYPE mCul = Set_LightAvg;
    if (m_CaptureVideo.Vsize != pHeader->Vsize)
    {
      if (m_CaptureVideo.Vbuffer)
        delete(m_CaptureVideo.Vbuffer);
      m_CaptureVideo.Vbuffer = (LPBYTE)malloc(pHeader->Vsize);
      m_CaptureVideo.Vwidth = pHeader->Vwidth;
      m_CaptureVideo.Vheight = pHeader->Vheight;
      m_CaptureVideo.Vsize = pHeader->Vsize;
      m_CaptureVideo.VwBit = pHeader->VwBit;
    }
    memcpy(m_CaptureVideo.Vbuffer, pBuffer, m_CaptureVideo.Vsize);
    m_ConfocalCore->GetCoreProcess(PROCESS_CalValue)->ProcessImg(&mCul, 0, &m_CaptureVideo);
    Render(m_CaptureVideo.Vbuffer, width, heidth, wbit);
    return;
  }

  Render(pBuffer, width, heidth, wbit);
}

void HeatmapWnd::Render(LPBYTE pBuffer, int width, int heidth, int wbit)
{
  DWORD newSize = width*heidth*wbit / 8;
  if (newSize != m_bmi->bmiHeader.biSizeImage)
  {
    m_bmi->bmiHeader.biWidth = width;
    m_bmi->bmiHeader.biHeight = heidth;
    m_bmi->bmiHeader.biBitCount = wbit;
    m_bmi->bmiHeader.biSizeImage = width*heidth*wbit / 8;
    EnterCriticalSection(&m_cs);
    if (m_Buffer)
    {
      delete(m_Buffer);
      m_Buffer = 0;
    }
    m_Buffer = (LPBYTE)malloc(m_bmi->bmiHeader.biSizeImage);
    LeaveCriticalSection(&m_cs);
    update_scrollbar(m_bmi->bmiHeader.biWidth, m_bmi->bmiHeader.biHeight);
  }
  EnterCriticalSection(&m_cs);
  if (m_Buffer)
    memcpy(m_Buffer, pBuffer, m_bmi->bmiHeader.biSizeImage);

  LeaveCriticalSection(&m_cs);
  //算fps
  //DWORD t_now=::GetTickCount();
  DWORD t_now = timeGetTime();
  DWORD dutime = t_now - m_FpsLastTime;
  m_FpsLastValue = 1000.0 / dutime;
  m_FpsLastTime = t_now;
  //TRACE(L"Speed FPS:%.2f!\n\t",m_FpsLastValue);
}

BOOL HeatmapWnd::OnEraseBkgnd(CDC* pDC)
{
  // TODO: 在此添加消息处理程序代码和/或调用默认值
  CRect rc;
  GetClientRect(&rc);
  DoubleDCFun(pDC, rc);//用双缓冲的方法
  return CWnd::OnEraseBkgnd(pDC);
}

void HeatmapWnd::DoubleDCFun(CDC* pDC, CRect pRc)
{
  //准备工作
  CDC MemDC;
  MemDC.CreateCompatibleDC(NULL);
  CBitmap MemBitmap;
  MemBitmap.CreateCompatibleBitmap(pDC, pRc.Width(), pRc.Height());
  MemDC.SelectObject(&MemBitmap);
  //CBitmap *pOldBit=MemDC.SelectObject(&MemBitmap);
  //准备结束，绘制图像数据...
  if (m_bFitWndows)//充满窗口用另一种方法 
    DrawFunctionM2(&MemDC, pRc);
  else
    DrawFunction(&MemDC, pRc);
  DrawRuler(&MemDC, m_MouseX, m_MouseY);//画标尺
  //DrawFps(&MemDC, pRc, m_FpsLastValue);
  pDC->BitBlt(0, 0, pRc.Width(), pRc.Height(), &MemDC, 0, 0, SRCCOPY);
  MemBitmap.DeleteObject();
  MemDC.DeleteDC();
}

void HeatmapWnd::DrawFunctionM2(CDC* pDC, CRect pRc)
{
  pDC->FillSolidRect(0, 0, pRc.Width(), pRc.Height(), afxGlobalData.clrBarFace);
  EnterCriticalSection(&m_cs);
  if (m_Buffer)
  {
    long	srcwidth = m_bmi->bmiHeader.biWidth;
    long	srcheight = m_bmi->bmiHeader.biHeight;
    iDestLeft = 0;
    iDestTop = 0;
    iDestW = pRc.Width();
    iDestH = pRc.Height();
    //算比例.....
    float scalex, scaley;
    scalex = (float)srcwidth / (float)iDestW;
    scaley = (float)srcheight / (float)iDestH;
    float AllScale = scalex>scaley ? scalex : scaley;//谁更小，用谁的
    ZoomAmp = AllScale;
    //输出大小 
    iDestH = srcheight / AllScale;
    iDestW = srcwidth / AllScale;
    //算顶点坐标
    iDestLeft = (pRc.Width() - iDestW) / 2;
    iDestTop = (pRc.Height() - iDestH) / 2;
    ::SetStretchBltMode(pDC->GetSafeHdc(), HALFTONE);
    ::StretchDIBits(
      pDC->GetSafeHdc(),
      iDestLeft, iDestTop, iDestW, iDestH, 0, 0,
      srcwidth, srcheight, m_Buffer, m_bmi, DIB_RGB_COLORS, SRCCOPY);
  }
  LeaveCriticalSection(&m_cs);
}

void HeatmapWnd::DrawFunction(CDC* pDC, CRect pRc)
{
  ZoomAmp = 1.0f;
  pDC->FillSolidRect(0, 0, pRc.Width(), pRc.Height(), afxGlobalData.clrBarFace);
  EnterCriticalSection(&m_cs);
  if (m_Buffer)
  {
    iDestLeft = 0;
    iDestTop = 0;
    iDestW = pRc.Width();
    iDestH = pRc.Height();
    long	srcwidth = m_bmi->bmiHeader.biWidth;
    long	srcheight = m_bmi->bmiHeader.biHeight;
    if (srcwidth<iDestW)
    {
      iDestLeft = (iDestW - srcwidth) / 2;
      iDestW = srcwidth;
    }
    else
      srcwidth = iDestW;
    if (srcheight<iDestH)
    {
      iDestTop = (iDestH - srcheight) / 2;
      iDestH = srcheight;
    }
    else
      srcheight = iDestH;
    ::StretchDIBits(
      pDC->GetSafeHdc(),
      iDestLeft, iDestTop, iDestW, iDestH,
      m_hPos, m_vPos, srcwidth, srcheight, m_Buffer,
      m_bmi, DIB_RGB_COLORS, SRCCOPY);
  }
  LeaveCriticalSection(&m_cs);
}

void HeatmapWnd::DrawRuler(CDC* pDC, CPoint p_x1, CPoint p_x2)
{
  if (p_x1 == p_x2)
    return;
  Graphics g(pDC->m_hDC);
  int x1 = p_x1.x;
  int y1 = p_x1.y;
  int x2 = p_x2.x;
  int y2 = p_x2.y;

  if (x1<iDestLeft)
    x1 = iDestLeft;
  else if (x1>iDestLeft + iDestW)
    x1 = iDestLeft + iDestW;
  if (y1<iDestTop)
    y1 = iDestTop;
  else if (y1>iDestTop + iDestH)
    y1 = iDestTop + iDestH;
  if (x2<iDestLeft)
    x2 = iDestLeft;
  else if (x2>iDestLeft + iDestW)
    x2 = iDestLeft + iDestW;
  if (y2<iDestTop)
    y2 = iDestTop;
  else if (y2>iDestTop + iDestH)
    y2 = iDestTop + iDestH;

  Pen roi_pen(Color::Red, 2);
  g.DrawLine(&roi_pen, x1, y1, x2, y2);
  GetClientRect(rcClient);
  //CString text;
  ////text.Format(_T("%.2f"),sqrt((float)((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)))/ZoomAmp);
  //text.Format(_T("%.2fum"), sqrt((float)((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2)))*m_CurPixelSize * 1000 * ZoomAmp);
  //g.DrawString(text.GetBuffer(), text.GetLength(), font, PointF(x2 + 15, y2 - 10), &stringformat, brush);
  ////下面是计算出标尺起终点应该再绘制的一个小直线
  //int LineLong = 5;
  //int xx = x2 - x1;
  //int yy = y2 - y1;
  //double xc, yc;
  //if (xx == 0)
  //{
  //  xc = LineLong;
  //  yc = 0;
  //}
  //else if (yy == 0)
  //{
  //  yc = LineLong;
  //  xc = 0;
  //}
  //else
  //{
  //  xc = LineLong / sqrt((double)(1.0 + (double)xx*xx / (yy*yy))); //X^+X*(X1^*Y1^)=LineLong^;
  //  yc = xc*xx / (double)yy;
  //}
  //int xc1 = x1 - xc;
  //int yc1 = y1 + yc;
  //int xc2 = x2 - xc;
  //int yc2 = y2 + yc;
  //xc += x1;
  //yc = y1 - yc;
  //g.DrawLine(&roi_pen, xc1, yc1, xc, yc);
  //g.DrawLine(&roi_pen, xc2, yc2, xc + xx, yc + yy);
}

void HeatmapWnd::DrawFps(CDC* pDC, CRect pRc, float pFps)
{
  Graphics g(pDC->m_hDC);
  CString text;
  float mV = 1.0f / ZoomAmp * 100;
  text.Format(_T("%.2f fps (Zoom:%.2f"), pFps, mV);
  text += L"%)";
  g.DrawString(text.GetBuffer(), text.GetLength(), font, PointF(140, 50), &stringformat, brush);
}

void HeatmapWnd::OnSize(UINT nType, int cx, int cy)
{
  CWnd::OnSize(nType, cx, cy);
  // TODO: 在此处添加消息处理程序代码
  update_scrollbar(m_bmi->bmiHeader.biWidth, m_bmi->bmiHeader.biHeight);
  UpdataView(m_nHOffset, m_nVOffset);
}

void HeatmapWnd::update_scrollbar(int width, int height)
{
  if (m_bFitWndows)
  {
    width = 0;
    height = 0;
  }
  SCROLLINFO hInfo, vInfo;
  memset(&hInfo, 0, sizeof(hInfo));
  hInfo.cbSize = sizeof(hInfo);
  hInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
  GetScrollInfo(SB_HORZ, &hInfo);
  memset(&vInfo, 0, sizeof(vInfo));
  vInfo.cbSize = sizeof(vInfo);
  vInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
  GetScrollInfo(SB_VERT, &vInfo);
  CRect	rc;
  GetClientRect(&rc);
  if (width > 0 && height > 0)
  {
    if (hInfo.nMin < 0)
      hInfo.nMin = 0;
    hInfo.nMax = width;
    hInfo.nPage = rc.Width();
    if ((int)(hInfo.nPos + hInfo.nPage - hInfo.nMax) > 0)
    {
      hInfo.nPos = hInfo.nMax - hInfo.nPage;
      m_nHOffset = hInfo.nPos;//这里暂时不记录
    }
    if (width == rc.Width())
      hInfo.nMax = 0;
    if (vInfo.nMin < 0)
      vInfo.nMin = 0;
    vInfo.nMax = height;
    vInfo.nPage = rc.Height();
    if ((int)(vInfo.nPos + vInfo.nPage - vInfo.nMax) > 0)
    {
      vInfo.nPos = vInfo.nMax - vInfo.nPage;
      m_nVOffset = vInfo.nPos;//这里暂时不记录
    }
    if (height == rc.Height())
      vInfo.nMax = 0;
  }
  else
  {
    hInfo.nMin = 0;
    hInfo.nMax = 0;
    hInfo.nPage = rc.Width();
    vInfo.nMin = 0;
    vInfo.nMax = 0;
    vInfo.nPage = rc.Height();
  }
  SetScrollInfo(SB_HORZ, &hInfo);
  SetScrollInfo(SB_VERT, &vInfo);
}

void HeatmapWnd::UpdataView(int pHpos, int pVpos)
{
  if (pHpos<0)
    pHpos = 0;
  if (pVpos<0)
    pVpos = 0;
  CRect	rc;
  GetClientRect(&rc);
  long twidth = rc.right - rc.left;
  long theight = rc.bottom - rc.top;
  if (twidth >= m_bmi->bmiHeader.biWidth)
    m_hPos = 0;
  else
    m_hPos = pHpos;
  if (theight >= m_bmi->bmiHeader.biHeight)
    m_vPos = 0;
  else
    m_vPos = m_bmi->bmiHeader.biHeight - theight - pVpos;//要不要反向
  Invalidate();
}

void HeatmapWnd::OnTimer(UINT_PTR nIDEvent)
{
  // TODO: 在此添加消息处理程序代码和/或调用默认值
  if (nIDEvent == 529)//刷新进程
    Invalidate();
  CWnd::OnTimer(nIDEvent);
}

void HeatmapWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  // TODO: 在此添加消息处理程序代码和/或调用默认值
  SCROLLINFO info;
  memset(&info, 0, sizeof(info));
  info.cbSize = sizeof(info);
  info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
  GetScrollInfo(SB_HORZ, &info);
  long dx = 0;
  long nStep = 1;
  switch (nSBCode)
  {
  case SB_LEFT:			dx = -1;			break;
  case SB_LINELEFT:		dx = -10 * nStep;		break;
  case SB_PAGELEFT:		dx = -(int)info.nPage;	break;
  case SB_PAGERIGHT:		dx = (int)info.nPage;	break;
  case SB_LINERIGHT:		dx = 10 * nStep;		break;
  case SB_RIGHT:			dx = nStep;				break;
  case SB_ENDSCROLL:		dx = 0;					break;
  case SB_THUMBTRACK:		dx = nPos - info.nPos;	break;
  case SB_THUMBPOSITION:
    if (long(nPos - info.nPos) > 10 * nStep)
      dx = 10 * nStep;
    else
      if (long(nPos - info.nPos) < -10 * nStep)
        dx = -10 * nStep;
      else
        dx = nPos - info.nPos;

    break;
  }
  info.nPos += dx;
  if (info.nPos > (int)(info.nMax - info.nPage))
    info.nPos = info.nMax - info.nPage;
  if (info.nPos < info.nMin)
    info.nPos = info.nMin;
  if (m_nHOffset != info.nPos)
  {
    m_nHOffset = info.nPos;
    SetScrollPos(SB_HORZ, info.nPos);
    UpdataView(m_nHOffset, m_nVOffset);
  }
  CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void HeatmapWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  // TODO: 在此添加消息处理程序代码和/或调用默认值
  SCROLLINFO info;
  memset(&info, 0, sizeof(info));
  info.cbSize = sizeof(info);
  info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
  GetScrollInfo(SB_VERT, &info);
  long dy = 0;
  long	nStep = 1;// (long)fZoomV;
  if (nStep < 1)	nStep = 1;
  switch (nSBCode)
  {
  case SB_BOTTOM:			dy = nStep;				break;
  case SB_LINEDOWN:		dy = 10 * nStep;		break;
  case SB_PAGEDOWN:		dy = (int)info.nPage;	break;
  case SB_PAGEUP:			dy = -(int)info.nPage;	break;
  case SB_LINEUP:			dy = -10 * nStep;		break;
  case SB_TOP:			dy = -nStep;			break;
  case SB_ENDSCROLL:		dy = 0; break;
  case SB_THUMBTRACK:	dy = nPos - info.nPos;	break;
  case SB_THUMBPOSITION:
    if (long(nPos - info.nPos) > 10 * nStep)
      dy = 10 * nStep;
    if (long(nPos - info.nPos) < -10 * nStep)
      dy = -10 * nStep;
    else
      dy = nPos - info.nPos;

    break;
  }
  info.nPos += dy;
  if (info.nPos > (int)(info.nMax - info.nPage))
    info.nPos = info.nMax - info.nPage;
  if (info.nPos < info.nMin)
    info.nPos = info.nMin;
  if (m_nVOffset != info.nPos)
  {
    m_nVOffset = info.nPos;
    SetScrollPos(SB_VERT, info.nPos);
    UpdataView(m_nHOffset, m_nVOffset);
  }
  CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void HeatmapWnd::OnMouseMove(UINT nFlags, CPoint point)
{
  // TODO: 在此添加消息处理程序代码和/或调用默认值
  if (m_bDragMouse)
    m_MouseY = point;
  __super::OnMouseMove(nFlags, point);
}

BOOL HeatmapWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
  // TODO: 在此添加消息处理程序代码和/或调用默认值
  TRACE(L"MouseWheel%d\n", zDelta);
  if (m_AxisZ)
  {
    if (zDelta >= 0)
      m_AxisZ->MoveRef(0.05);
    else
      m_AxisZ->MoveRef(-0.05);
  }
  return TRUE;
  //return __super::OnMouseWheel(nFlags, zDelta, pt);
}

void HeatmapWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
  // TODO: 在此添加消息处理程序代码和/或调用默认值
  m_bDragMouse = true;
  m_MouseX = point;
  m_MouseY = point;
  CWnd::OnLButtonDown(nFlags, point);
}

void HeatmapWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
  // TODO: 在此添加消息处理程序代码和/或调用默认值
  m_bDragMouse = false;
  m_MouseY = point;

  // 计算对应的点
  if (GetParent())
  {
    GetParent()->PostMessage(WM_USER + 2000);
  }

  __super::OnLButtonUp(nFlags, point);
}

void HeatmapWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
  // TODO: 在此添加消息处理程序代码和/或调用默认值
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  //下面是菜单 
  ClientToScreen(&point);
  CMenu tmenu;
  tmenu.LoadMenuW(IDR_MENU_HEATMAP);//加入菜单
  CMenu* pPopMenu = tmenu.GetSubMenu(0);
  pPopMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);//显示菜单，左边，跟踪右键
  pPopMenu->Detach();
  tmenu.DestroyMenu();
}

void HeatmapWnd::OnEpiFitwindows()
{
  // TODO: 在此添加命令处理程序代码
  m_bFitWndows = !m_bFitWndows;
  SendMessage(WM_SIZE);
  //update_scrollbar(0,0);
  Invalidate();
}

void HeatmapWnd::OnEpiSaveframe()
{
  // TODO: 在此添加命令处理程序代码
  CString ImgName;
  SYSTEMTIME st;
  GetLocalTime(&st);
  CFileDialog fileDlg(false);
  fileDlg.m_ofn.lpstrTitle = _T("SaveImg");
  fileDlg.m_ofn.lpstrFilter = _T("Bmp Files(*.bmp)\0*.bmp\0All Files(*.*)\0*.*\0\0");
  fileDlg.m_ofn.lpstrDefExt = _T("bmp");
  ImgName.Format(_T("%02d-%02d_%02d%02d%02d.bmp"), st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
  fileDlg.m_ofn.lpstrFile = ImgName.GetBuffer(200);
  fileDlg.m_ofn.nMaxFile = 200;
  if (IDOK == fileDlg.DoModal())
  {
    HSaveBmp((LPBYTE)m_Buffer, m_bmi->bmiHeader.biWidth,
      m_bmi->bmiHeader.biHeight, m_bmi->bmiHeader.biBitCount, fileDlg.GetPathName().GetBuffer());
  }
}

int HeatmapWnd::OnSubjectNotified(IPtnSubject * pSubject, int ID, long wParam, void* pParam, float fParam, void* mParam)
{
  if (ID == NOTIFY_MAPROISIZE_FLUSH)
  {
    m_CurPixelSize = fParam;
    Invalidate();
  }
  else if (ID == NOTIFY_CVCal_FLUSH)
  {
    bSetCalCV = wParam;
  }
  return 0;
}

void HeatmapWnd::GetLinePosition(CPoint& pt1, CPoint& pt2)
{
  char buf[64];
  sprintf_s(buf, "Line %d %d %d %d\n", m_MouseX.x, m_MouseX.y,
    m_MouseY.x, m_MouseY.y);
  OutputDebugStringA(buf);

  pt1 = Map2Image(m_MouseX);
  pt2 = Map2Image(m_MouseY);

  sprintf_s(buf, "MapLine %d %d %d %d\n", pt1.x, pt1.y,
    pt2.x, pt2.y);
  OutputDebugStringA(buf);
}

CPoint HeatmapWnd::Map2Image(const CPoint& pt)
{
  CPoint p = pt;
  if (m_bFitWndows)
  {
    p.x = (int)((p.x - iDestLeft) * ZoomAmp);
    p.y = (int)((p.y - iDestTop) * ZoomAmp);
  }
  else
  {
    p.x += m_nHOffset;
    p.y += m_nVOffset;
  }
  return p;
}
