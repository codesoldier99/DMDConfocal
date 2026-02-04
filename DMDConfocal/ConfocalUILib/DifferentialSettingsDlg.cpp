// DifferentialSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "DifferentialSettingsDlg.h"
#include "afxdialogex.h"


// DifferentialSettingsDlg dialog

IMPLEMENT_DYNAMIC(DifferentialSettingsDlg, CDialogEx)

DifferentialSettingsDlg::DifferentialSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIFFERENTIALSETTINGSDLG, pParent)
  , _coeff_k(1.0)
  , _coeff_b(0)
  , _defocus(3.0)
  ,_focus_cap_mode(0)
  ,_defocus_cap_mode(0)
  ,_z_ctrl_mode(0)
  , _backlash(1.0)
	, m_iCapDelay(200)
{
  _coeff_k = m_ConfFile.GetDouble(L"DiffSettings", L"coeff_k", L"Confocal_DiffMeasure");
  _coeff_b = m_ConfFile.GetDouble(L"DiffSettings", L"coeff_b", L"Confocal_DiffMeasure");
  _defocus = m_ConfFile.GetDouble(L"DiffSettings", L"defocus", L"Confocal_DiffMeasure");
  if (_defocus < 0.01)_defocus = 0.01;
  _focus_cap_mode = m_ConfFile.GetInt(L"DiffSettings", L"focus_cap_mode", L"Confocal_DiffMeasure");
  if (_focus_cap_mode < 0)_focus_cap_mode = 0;
  if (_focus_cap_mode > 2)_focus_cap_mode = 2;
  _defocus_cap_mode = m_ConfFile.GetInt(L"DiffSettings", L"defocus_cap_mode", L"Confocal_DiffMeasure");
  if (_defocus_cap_mode < 0)_defocus_cap_mode = 0;
  if (_defocus_cap_mode > 2)_defocus_cap_mode = 2;
  _z_ctrl_mode = m_ConfFile.GetInt(L"DiffSettings", L"z_ctrl_mode", L"Confocal_DiffMeasure");
  if (_z_ctrl_mode < 0)_z_ctrl_mode = 0;
  if (_z_ctrl_mode > 1)_z_ctrl_mode = 1;
  _backlash = m_ConfFile.GetDouble(L"DiffSettings", L"backlash", L"Confocal_DiffMeasure");
  if (_backlash < 0)_backlash = 0;

  m_ConfocalCore = 0;
}

DifferentialSettingsDlg::~DifferentialSettingsDlg()
{
}

void DifferentialSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COEFF_K, _coeff_k);
	DDX_Text(pDX, IDC_EDIT_COEFF_B, _coeff_b);
	DDX_Text(pDX, IDC_EDIT_DEFOCUS, _defocus);
	DDX_Radio(pDX, IDC_RADIO_FOCUS_CM_WIDE, _focus_cap_mode);
	DDX_Radio(pDX, IDC_RADIO_DEFOCUS_CM_WIDE, _defocus_cap_mode);
	DDX_Radio(pDX, IDC_RADIO_ZMODE_STEP, _z_ctrl_mode);
	DDX_Text(pDX, IDC_EDIT_BACKLASH, _backlash);
	DDX_Text(pDX, IDC_EDIT_CAPDELAY, m_iCapDelay);
}

BEGIN_MESSAGE_MAP(DifferentialSettingsDlg, CDialogEx)
  ON_BN_CLICKED(IDC_BUTTON_START, &DifferentialSettingsDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STARTPRO, &DifferentialSettingsDlg::OnBnClickedButtonStartpro)
	ON_BN_CLICKED(IDC_BUTTON_STARTTEST, &DifferentialSettingsDlg::OnBnClickedButtonStarttest)
	ON_BN_CLICKED(IDC_BTNDACAF, &DifferentialSettingsDlg::OnBnClickedBtndacaf)
END_MESSAGE_MAP()


// DifferentialSettingsDlg message handlers
bool DifferentialSettingsDlg::InitPanel(LPVOID p_Param, LPVOID p_Param2)
{
  m_ConfocalCore = (HConfocalCore*)p_Param;

  HAxis* pAxisPIZ = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_PIZ);
  if (!pAxisPIZ)
  {
	  GetDlgItem(IDC_RADIO_ZMODE_PI)->EnableWindow(false);
  }
  return true;
}

int DifferentialSettingsDlg::OnSubjectNotified(IPtnSubject * pSubject, int ID, long wParam,
  void* pParam, float fParam, void* mParam)
{
  return 0;
}

BOOL DifferentialSettingsDlg::OnInitDialog()
{
  __super::OnInitDialog();

  SetWindowTextW(L"DiffMeasurement");

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

void DifferentialSettingsDlg::OnBnClickedButtonStart()
{
	HConfocalPlug* tscan = m_ConfocalCore->GetPlugin(PLUGIN_DIFFMEASURE);
	if (!tscan)
	{
		return;
	}

	CWaitCursor wc;
	if (!UpdateData())
	{
		return;// 参数设置不合适
	}

	// 保存参数
	m_ConfFile.RecValue(L"DiffSettings", L"coeff_k", _coeff_k, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"coeff_b", _coeff_b, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"defocus", _defocus, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"focus_cap_mode", _focus_cap_mode, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"defocus_cap_mode", _defocus_cap_mode, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"z_ctrl_mode", _z_ctrl_mode, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"backlash", _backlash, L"Confocal_DiffMeasure");

	DiffMeasureParam dmp;
	dmp.coeff_k = _coeff_k;
	dmp.coeff_b = _coeff_b;
	dmp.defocus = _defocus;
	dmp.backlash = _backlash;
	dmp.focus_cap_mode = _focus_cap_mode;
	dmp.defocus_cap_mode = _defocus_cap_mode;
	dmp.z_ctrl_mode = _z_ctrl_mode;
	dmp.bTest = false;
	dmp.iSleep = m_iCapDelay;
	dmp.bFilter = ((CButton*)GetDlgItem(IDC_CHECKFilter))->GetCheck();

	tscan->Set(&dmp);
	tscan->Start();
}


void DifferentialSettingsDlg::OnBnClickedButtonStartpro()
{
	// TODO: 在此添加控件通知处理程序代码
	HConfocalPlug* tscan = m_ConfocalCore->GetPlugin(PLUGIN_DIFFMEASUREPRO);
	if (!tscan)
		return;

	CWaitCursor wc;
	if (!UpdateData())
	{
		// 参数设置不合适
		return;
	}

	// 保存参数
	m_ConfFile.RecValue(L"DiffSettings", L"coeff_k", _coeff_k, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"coeff_b", _coeff_b, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"defocus", _defocus, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"focus_cap_mode", _focus_cap_mode, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"defocus_cap_mode", _defocus_cap_mode, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"z_ctrl_mode", _z_ctrl_mode, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"backlash", _backlash, L"Confocal_DiffMeasure");

	DiffMeasureParam dmp;
	dmp.coeff_k = _coeff_k;
	dmp.coeff_b = _coeff_b;
	dmp.defocus = _defocus;
	dmp.backlash = _backlash;
	dmp.focus_cap_mode = _focus_cap_mode;
	dmp.defocus_cap_mode = _defocus_cap_mode;
	dmp.z_ctrl_mode = _z_ctrl_mode;
	dmp.bTest = false;
	dmp.bFilter = ((CButton*)GetDlgItem(IDC_CHECKFilter))->GetCheck();

	tscan->Set(&dmp);
	tscan->Start();
}


void DifferentialSettingsDlg::OnBnClickedButtonStarttest()
{
	// TODO: 在此添加控件通知处理程序代码
	HConfocalPlug* tscan = m_ConfocalCore->GetPlugin(PLUGIN_DIFFMEASUREPRO);
	if (!tscan)
		return;

	CWaitCursor wc;
	if (!UpdateData())
	{
		// 参数设置不合适
		return;
	}

	// 保存参数
	m_ConfFile.RecValue(L"DiffSettings", L"coeff_k", _coeff_k, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"coeff_b", _coeff_b, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"defocus", _defocus, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"focus_cap_mode", _focus_cap_mode, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"defocus_cap_mode", _defocus_cap_mode, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"z_ctrl_mode", _z_ctrl_mode, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"backlash", _backlash, L"Confocal_DiffMeasure");

	DiffMeasureParam dmp;
	dmp.coeff_k = _coeff_k;
	dmp.coeff_b = _coeff_b;
	dmp.defocus = _defocus;
	dmp.backlash = _backlash;
	dmp.focus_cap_mode = _focus_cap_mode;
	dmp.defocus_cap_mode = _defocus_cap_mode;
	dmp.z_ctrl_mode = _z_ctrl_mode;
	dmp.bTest = true;
	dmp.bFilter = ((CButton*)GetDlgItem(IDC_CHECKFilter))->GetCheck();

	tscan->Set(&dmp);
	tscan->Start();
}


void DifferentialSettingsDlg::OnBnClickedBtndacaf()
{
	// TODO: 在此添加控件通知处理程序代码

	HConfocalPlug* tscan = m_ConfocalCore->GetPlugin(PLUGIN_AutoFocusDAC);
	if (!tscan)
		return;

	CWaitCursor wc;
	if (!UpdateData())
	{
		// 参数设置不合适
		return;
	}

	// 保存参数
	m_ConfFile.RecValue(L"DiffSettings", L"coeff_k", _coeff_k, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"coeff_b", _coeff_b, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"defocus", _defocus, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"focus_cap_mode", _focus_cap_mode, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"defocus_cap_mode", _defocus_cap_mode, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"z_ctrl_mode", _z_ctrl_mode, L"Confocal_DiffMeasure");
	m_ConfFile.RecValue(L"DiffSettings", L"backlash", _backlash, L"Confocal_DiffMeasure");

	DiffMeasureParam dmp;
	dmp.coeff_k = _coeff_k;
	dmp.coeff_b = _coeff_b;
	dmp.defocus = _defocus;
	dmp.backlash = _backlash;
	dmp.focus_cap_mode = _focus_cap_mode;
	dmp.defocus_cap_mode = _defocus_cap_mode;
	dmp.z_ctrl_mode = _z_ctrl_mode;
	dmp.bTest = true;
	dmp.bFilter = ((CButton*)GetDlgItem(IDC_CHECKFilter))->GetCheck();

	tscan->Set(&dmp);
	tscan->Start();
}
