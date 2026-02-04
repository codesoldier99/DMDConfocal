// SIImagePanel.cpp : implementation file
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "SIImagePanel.h"
#include "afxdialogex.h"
#include "resource.h"

// SIImagePanel dialog

IMPLEMENT_DYNAMIC(SIImagePanel, CDialogEx)

SIImagePanel::SIImagePanel(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SIIMAGEPANEL, pParent)
{

}

SIImagePanel::~SIImagePanel()
{
}

void SIImagePanel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(SIImagePanel, CDialogEx)
  ON_BN_CLICKED(IDOK, &SIImagePanel::OnBnClickedOk)
  ON_BN_CLICKED(IDCANCEL, &SIImagePanel::OnBnClickedCancel)
  ON_WM_SIZE()
END_MESSAGE_MAP()


// SIImagePanel message handlers


void SIImagePanel::OnBnClickedOk()
{
  return;
}

void SIImagePanel::OnBnClickedCancel()
{
  return;
}


BOOL SIImagePanel::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  CRect r;
  GetClientRect(&r);
  _imgs_wnd.Create(NULL, L"", WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE, r, this, 0);

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

void SIImagePanel::OnSize(UINT nType, int cx, int cy)
{
  if (_imgs_wnd.GetSafeHwnd())
  {
    CRect r;
    GetClientRect(&r);
    _imgs_wnd.MoveWindow(&r);
  }

  CDialogEx::OnSize(nType, cx, cy);
}

void SIImagePanel::SetDiffData(int w, int h, unsigned char* a,
  unsigned char* b, unsigned char* c)
{
  _imgs_wnd.SetDiffData(w, h, a, b, c);
}