// SIViewPanel.cpp : implementation file
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "SIViewPanel.h"
#include "afxdialogex.h"
#include "resource.h"

// SIViewPanel dialog

IMPLEMENT_DYNAMIC(SIViewPanel, CDialogEx)

SIViewPanel::SIViewPanel(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SIVIEWPANEL, pParent)
{

}

SIViewPanel::~SIViewPanel()
{
}

void SIViewPanel::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_TAB_PANEL, _tab);
}


BEGIN_MESSAGE_MAP(SIViewPanel, CDialogEx)
  ON_BN_CLICKED(IDOK, &SIViewPanel::OnBnClickedOk)
  ON_BN_CLICKED(IDCANCEL, &SIViewPanel::OnBnClickedCancel)
  ON_WM_SIZE()
  ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_PANEL, &SIViewPanel::OnTcnSelchangeTabPanel)
END_MESSAGE_MAP()

// SIViewPanel message handlers

void SIViewPanel::OnBnClickedOk()
{
  return;
}

void SIViewPanel::OnBnClickedCancel()
{
  return;
}


BOOL SIViewPanel::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  // TODO:  Add extra initialization here
  _tab.InsertItem(0, _T("Image"));
  _tab.InsertItem(1, _T("Line Profile"));
  _tab.InsertItem(2, _T("3D"));
  _tab.SetCurSel(0);

  _img_panel.Create(IDD_SIIMAGEPANEL, this);
  _line_panel.Create(IDD_SILINEPROFILEPANEL, this);
  _3d_panel.Create(IDD_SI3DPANEL, this);

  _img_panel.ShowWindow(SW_HIDE);
  _line_panel.ShowWindow(SW_HIDE);
  _3d_panel.ShowWindow(SW_HIDE);

  UpdatePanel();

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

void SIViewPanel::OnSize(UINT nType, int cx, int cy)
{
  CDialogEx::OnSize(nType, cx, cy);

  CRect r;
  GetClientRect(&r);
  if (_tab.GetSafeHwnd())
  {
    _tab.MoveWindow(&r);
    UpdatePanel();
  }
}

void SIViewPanel::OnTcnSelchangeTabPanel(NMHDR *pNMHDR, LRESULT *pResult)
{
  UpdatePanel();
  *pResult = 0;
}

void SIViewPanel::UpdatePanel()
{
  CWnd* wnd[3] =
  {
    &_img_panel,
    &_line_panel,
    &_3d_panel,
  };
  for (int i = 0; i < _tab.GetItemCount(); i++)
  {
    if (_tab.GetCurSel() != i)
    {
      wnd[i]->ShowWindow(SW_HIDE);
    }
  }
  CRect r;
  _tab.GetClientRect(&r);
  CRect itemr;
  _tab.GetItemRect(0, &itemr);
  r.top = itemr.bottom;
  wnd[_tab.GetCurSel()]->MoveWindow(&r);
  wnd[_tab.GetCurSel()]->ShowWindow(SW_SHOW);
}

void SIViewPanel::SetDiffData(int w, int h, unsigned char* a,
  unsigned char* b, unsigned char* c,double* height_img,const DiffMeasureParam& dmp)
{
	_img_panel.SetDiffData(w, h, b, a, c); //注意顺序
	_line_panel.SetHeatmap(w, h, height_img);//显示高度图（含重新计算高度）
	// 重新采样，缩放图像，放大系数
	int sw = w / 2;
	int sh = h / 2;
	double scale = 100;
	vector<double> s_data(sw * sh);
	for (int i = 0; i < sh; i++)
	{
	for (int j = 0; j < sw; j++)
	{
		int sp = (i * 2) * w + j * 2;
		double v = height_img[sp] + height_img[sp + 1] +
		height_img[sp + w] + height_img[sp + w + 1];
		s_data[i * sw + j] = v * scale / 4;
	}
	}
	_3d_panel.SetHeightData(sw, sh, s_data.data(), dmp);//3D显示图形
}