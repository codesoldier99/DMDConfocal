#pragma once
#include "afxcmn.h"
#include "SIImagePanel.h"
#include "SILineProfilePanel.h"
#include "SI3DPanel.h"

// SIViewPanel dialog

class SIViewPanel : public CDialogEx
{
	DECLARE_DYNAMIC(SIViewPanel)

public:
	SIViewPanel(CWnd* pParent = NULL);   // standard constructor
	virtual ~SIViewPanel();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIVIEWPANEL };
#endif

  void SetDiffData(int w, int h, unsigned char* a,
    unsigned char* b, unsigned char* c,
    double* height_img,
    const DiffMeasureParam& dmp);
private:
  void UpdatePanel();
private:
  SIImagePanel _img_panel;
  SILineProfilePanel _line_panel;
  SI3DPanel _3d_panel;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedCancel();
  CTabCtrl _tab;
  virtual BOOL OnInitDialog();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnTcnSelchangeTabPanel(NMHDR *pNMHDR, LRESULT *pResult);
};
