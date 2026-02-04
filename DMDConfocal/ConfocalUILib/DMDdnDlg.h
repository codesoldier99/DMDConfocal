#pragma once
#include "Resource.h"
#include "HConfocalCore.h"
#include "HGloableFunction.h"
#include "afxwin.h"
#include "PagerWnd.h"
#include "afxeditbrowsectrl.h"

// CDMDdnDlg 对话框

using namespace WlpDMDControl;

class CDMDdnDlg : public CDialogEx,public HCorePanel
{
	DECLARE_DYNAMIC(CDMDdnDlg)

public:
	CDMDdnDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDMDdnDlg();

	virtual CWnd* GetCWnd() {return this;};
	virtual bool InitPanel(LPVOID p_Param, LPVOID p_Param2 = 0);
	void SetDmdManager(IDMDManager* pManager) 
			{m_DmdManager=pManager;};

// 对话框数据
	enum { IDD = IDD_DMDPATTERN };

protected:
	IDMDManager* m_DmdManager;
	HConfocalCore* m_ConfocalCore;
	HConfigure m_ConfFile;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int m_PtSize;
	int m_PtPeriod;
	int m_PtEx;

	int m_LineSize;
	int m_LinePeriod;
	int m_LineEx;

	int m_FramRate;
	int m_trigger_delay;
	int m_trigger_pw;
	WlpDMDParas mDMDParas;
	HEasyFunction* mF;

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedWhitedmdbtn();
	afx_msg void OnBnClickedBlackdmdbtn();
	afx_msg void OnBnClickedPicbtn();
	afx_msg void OnBnClickedResetbtn();
	afx_msg void OnBnClickedStopbtn();
	afx_msg void OnBnClickedStartdmdbtn();
	afx_msg void OnBnClickedUpdateframe();
	afx_msg void OnBnClickedPointscan();
	afx_msg void OnBnClickedHr();
	afx_msg void OnBnClickedVr();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	int mBlackTime;
	CComboBox comboGray;

	virtual int OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);
	afx_msg void OnBnClickedStartconbtn();
	afx_msg void OnBnClickedStopconbtn();
	afx_msg void OnBnClickedSaveresbtn();
	afx_msg void OnBnClickedCheckprocess();
	int miPicCount;
private:
	void UpdateUI();
	void CalcDMDTime(int width, int height, float exposure);
	void ResetSIFrames();
	void Func_FlushDMDParas();
	void Func_DownLinePic(int iLineType);
private:
	PagerWnd m_wndScroll;
	CString strShow;
	CString strShowEx;
	bool m_start;
	bool m_Open;
	int _si_frames;
public:
	int m_iBackGray;
	CComboBox m_trigger_mode;
	CComboBox comboDMDType;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEnChangeWidth2();
	afx_msg void OnEnChangeWidth();
	afx_msg void OnBnClickedButtonStartsi();
	afx_msg void OnBnClickedButtonStopsi();
	CMFCEditBrowseCtrl _sl_path;
	CString _str_sl_path;
	afx_msg void OnBnClickedButtonSiPattern();
	afx_msg void OnBnClickedInitdmdbtn();
};
