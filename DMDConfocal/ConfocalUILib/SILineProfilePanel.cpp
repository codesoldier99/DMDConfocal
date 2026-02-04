// SILineProfilePanel.cpp : implementation file
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "SILineProfilePanel.h"
#include "afxdialogex.h"
#include "resource.h"

// SILineProfilePanel dialog

IMPLEMENT_DYNAMIC(SILineProfilePanel, CDialogEx)

SILineProfilePanel::SILineProfilePanel(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SILINEPROFILEPANEL, pParent)
{
  _width = 0;
  _height = 0;
  _line_serie = 0;
}

SILineProfilePanel::~SILineProfilePanel()
{
}

void SILineProfilePanel::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LINE_PROFILE, _line_profile);
}


BEGIN_MESSAGE_MAP(SILineProfilePanel, CDialogEx)
  ON_BN_CLICKED(IDOK, &SILineProfilePanel::OnBnClickedOk)
  ON_BN_CLICKED(IDCANCEL, &SILineProfilePanel::OnBnClickedCancel)
  ON_WM_SIZE()
END_MESSAGE_MAP()

// SILineProfilePanel message handlers
void SILineProfilePanel::OnBnClickedOk()
{
  return;
}

void SILineProfilePanel::OnBnClickedCancel()
{
  return;
}


BOOL SILineProfilePanel::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  CRect r;
  GetClientRect(&r);
  _heatmap.Create(NULL, L"", WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE, r, this, 0);
  _heatmap.InitPanel(0);

  CChartAxis *pAxis = NULL;
  pAxis = _line_profile.CreateStandardAxis(CChartCtrl::BottomAxis);
  pAxis->SetAutomatic(true);
  pAxis = _line_profile.CreateStandardAxis(CChartCtrl::LeftAxis);
  pAxis->SetAutomatic(true);
  _line_serie = _line_profile.CreateLineSerie();

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

void SILineProfilePanel::OnSize(UINT nType, int cx, int cy)
{
  if (_heatmap.GetSafeHwnd() && _line_profile.GetSafeHwnd())
  {
    CRect r;
    GetClientRect(&r);
    CRect left = r;
    left.right = (left.left + left.right) / 2;
    _heatmap.MoveWindow(&left);
    CRect right = r;
    right.left = left.right;
    _line_profile.MoveWindow(&right);
  }
  CDialogEx::OnSize(nType, cx, cy);
}

void SILineProfilePanel::SetHeatmap(int width, int height, double* data)
{
  if (width <= 0 || height <= 0 || !data)
  {
    return;
  }
  _width = width;
  _height = height;
  _data.resize(_width * _height);
  memcpy(_data.data(), data, _width * _height * sizeof(double));

  // 映射到 unsigned char
  double minv(_data[0]), maxv(_data[0]);
  for (int i = 0; i < _data.size(); i++)
  {
    if (_data[i] > maxv)
    {
      maxv = _data[i];
    }
    if (_data[i] < minv)
    {
      minv = _data[i];
    }
  }
  // minv 0
  // maxv 255
  vector<unsigned char> img;
  img.resize(_width * _height);
  if (maxv <= minv)
  {
    for (int i = 0; i < img.size(); i++)
    {
      img[i] = 0;
    }
  }
  else
  {
    for (int i = 0; i < img.size(); i++)
    {
      int k = (_data[i] - minv) * 255 / (maxv - minv);
      if (k < 0)k = 0;
      if (k > 255)k = 255;
      img[i] = k;
    }
  }
  _heatmap.Render(img.data(), _width, _height, 8);
}

void SILineProfilePanel::SampleLine(int x0, int y0, int x1, int y1,
  vector<pair<int, int>>& pts)
{
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = (dx > dy ? dx : -dy) / 2, e2;

  for (;;) {
    pts.push_back(make_pair(x0, _height - y0 - 1));
    if (x0 == x1 && y0 == y1) break;
    e2 = err;
    if (e2 > -dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

LRESULT SILineProfilePanel::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  if (message == WM_USER + 2000)
  {
    if (_width <= 0 || _height <= 0)
    {
      return 0;
    }
    CPoint pt1(0, 0), pt2(0, 0);
    _heatmap.GetLinePosition(pt1, pt2);
    if (pt1.x < 0)
    {
      pt1.x = 0;
    }
    else if (pt1.x >= _width)
    {
      pt1.x = _width - 1;
    }
    if (pt1.y < 0)
    {
      pt1.y = 0;
    }
    else if (pt1.y >= _height)
    {
      pt1.y = _height - 1;
    }

    if (pt2.x < 0)
    {
      pt2.x = 0;
    }
    else if (pt2.x >= _width)
    {
      pt2.x = _width - 1;
    }
    if (pt2.y < 0)
    {
      pt2.y = 0;
    }
    else if (pt2.y >= _height)
    {
      pt2.y = _height - 1;
    }

    bool need_swap = pt1.x > pt2.x || (pt1.x == pt2.x && pt1.y > pt2.y);
    if (need_swap)
    {
      swap(pt1, pt2);
    }
    vector<pair<int, int>> pts;
    SampleLine(pt1.x, pt1.y, pt2.x, pt2.y, pts);
    vector<double> x, y;
    for (int i = 0; i < pts.size(); i++)
    {
      x.push_back(i);
      y.push_back(_data[pts[i].second * _width + pts[i].first]);
    }
    // 获取线段上的点
    _line_serie->ClearSerie();
    if (x.size() > 0)
    {
      _line_serie->AddPoints(x.data(), y.data(), x.size());
    }
  }

  return CDialogEx::WindowProc(message, wParam, lParam);
}
