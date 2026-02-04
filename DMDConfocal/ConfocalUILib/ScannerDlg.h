#pragma once
#include "Resource.h"

// CScannerDlg 对话框

class CScannerDlg : public CDialogEx,public HCorePanel
{
	DECLARE_DYNAMIC(CScannerDlg)

public:
	CScannerDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScannerDlg();

// 对话框数据
	enum { IDD = IDD_SCANCONTROL };

public:
	virtual CWnd* GetCWnd() {return this;};
	virtual bool InitPanel(LPVOID p_Param, LPVOID p_Param2 = 0);
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);

protected:

	HConfocalCore* m_ConfocalCore;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	HConfigure m_ConfFile;
	void SaveParas();
	AutoFocusParas mV;
	MapBuildParas mMapBuildParasV;
	DECLARE_MESSAGE_MAP()
public:
	HAxis* m_AxisZ;
	HAxis* m_AxisPIZ;
	float m_fUpPos;
	float m_fDownPos;
	float m_fStepPos;
	float m_iSpeed;		//控制步进电机和PI电机的速度
	float miMapStartX;
	float miMapStartY;
	float miMapEndX;
	float miMapEndY;
	afx_msg void OnBnClickedSetupbtn();
	afx_msg void OnBnClickedSetdownbtn();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedRebuildbtn();
	afx_msg void OnBnClickedAdvancebtn();
	afx_msg void OnBnClickedAutofocusbtn();
	afx_msg void OnBnClickedColorbuildbtn();
	afx_msg void OnBnClickedMapscanbtn();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedDefaultbtn();
	int mScanType;
	afx_msg void OnBnClickedAutofocusexbtn();
	afx_msg void OnBnClickedComtinuebtn();
	int iScanSave;
	double mdPIUp;
	double mdPIDown;
	double mdPIStep;
	double mdPIMove;
	afx_msg void OnBnClickedSetuppibtn();
	afx_msg void OnBnClickedSetdownpibtn();
	afx_msg void OnBnClickedRebuildpibtn();
	afx_msg void OnBnClickedUppibtn();
	afx_msg void OnBnClickedDownpibtn();
	afx_msg void OnBnClickedResetposbtn();
	afx_msg void OnBnClickedBtncapture3d();
	int m_iCtrlAxisMode;
};
