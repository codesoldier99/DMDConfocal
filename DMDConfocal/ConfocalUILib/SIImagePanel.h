#pragma once

#include "SIImageWnd.h"
// SIImagePanel dialog

class SIImagePanel : public CDialogEx
{
	DECLARE_DYNAMIC(SIImagePanel)

public:
	SIImagePanel(CWnd* pParent = NULL);   // standard constructor
	virtual ~SIImagePanel();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIIMAGEPANEL };
#endif
  void SetDiffData(int w, int h, unsigned char* a,
    unsigned char* b, unsigned char* c);
private:
  SIImageWnd _imgs_wnd;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedCancel();
  virtual BOOL OnInitDialog();
  afx_msg void OnSize(UINT nType, int cx, int cy);
};
