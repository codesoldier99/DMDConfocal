#pragma once
#include "Resource.h"
#include "afxwin.h"
#include "afxcmn.h"
//#include "HConfocalCore.h"

// CVideoConfDlg 对话框

class CVideoConfDlg : public CDialogEx,public HCorePanel
{
	DECLARE_DYNAMIC(CVideoConfDlg)

public:
	CVideoConfDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CVideoConfDlg();
	virtual bool InitPanel(LPVOID p_Param,LPVOID p_Param2=0);
	virtual CWnd* GetCWnd() {return this;};
	virtual int OnSubjectNotified(IPtnSubject * pSubject, int ID,long wParam = 0,
		void* pParam = 0,float fParam=0.0f,void* mParam=0);

// 对话框数据
	enum { IDD = IDD_VIDEOCONFIG };

protected:
	HConfocalCore* m_ConfocalCore;
	HVideoDevice* m_VideoDevice;
	//HVideoManager* m_VideoManager;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	TrigerType mTrigerV;
	bool bOpenEnable,bPauseEnable;
	CRITICAL_SECTION m_crtSec;
  int m_width;
  int m_height;
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_device;
	CComboBox m_bining;
	CSliderCtrl m_exposure;
	float m_Vexposure;
	CButton m_CheckAuto;
	CButton m_Check_Int;
	CButton m_Check_Ext;
	CButton m_Check_Soft;
	CButton m_Check_Timer;
	bool bRecMsg;//是否在接收消息时候，进行修改界面状态   防止这边界面修改，那边视频再反馈过来，界面接收消息又修改
	bool bOnTimeMsg;//计时器到了，是否刷新界面

	bool bVideoFlip,bVideoMirror;

	afx_msg void OnCbnSelchangeComboDev();
	afx_msg void OnCbnSelchangeComboBin();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedCheckIn();
	afx_msg void OnBnClickedCheckOut();
	afx_msg void OnBnClickedCheckSoft();
	afx_msg void OnBnClickedCheckTime();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedAdvanced();
	afx_msg void OnBnClickedTrigerbtn();
	//int miRadio;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedCheckmirror();
	afx_msg void OnBnClickedCheckflip();
	afx_msg void OnBnClickedCheck1();

private:
	int iExScale;
	bool m_bExType;//是否有额外的相机
};
