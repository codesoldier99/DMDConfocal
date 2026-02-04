#pragma once

#include "resource.h"

// DifferentialSettingsDlg dialog

class DifferentialSettingsDlg : public CDialogEx, public HCorePanel
{
	DECLARE_DYNAMIC(DifferentialSettingsDlg)

public:
	DifferentialSettingsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~DifferentialSettingsDlg();

  virtual bool InitPanel(LPVOID p_Param,LPVOID p_Param2=0);
  virtual CWnd* GetCWnd() { return this; };
  virtual int OnSubjectNotified(IPtnSubject * pSubject, int ID, long wParam = 0,
    void* pParam = 0, float fParam = 0.0f, void* mParam = 0);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIFFERENTIALSETTINGSDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
  virtual BOOL OnInitDialog();
  double _coeff_k;
  double _coeff_b;
  double _defocus;
  double _backlash;
  int _focus_cap_mode;
  int _defocus_cap_mode;
  int _z_ctrl_mode;
  afx_msg void OnBnClickedButtonStart();
private:
  HConfigure m_ConfFile;
  HConfocalCore* m_ConfocalCore;
public:
	afx_msg void OnBnClickedButtonStartpro();
	afx_msg void OnBnClickedButtonStarttest();
	int m_iCapDelay;
	afx_msg void OnBnClickedBtndacaf();
};
