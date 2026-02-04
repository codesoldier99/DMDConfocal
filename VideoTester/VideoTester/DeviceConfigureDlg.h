#pragma once

#include "resource.h"
#include "CarlVideo.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "WndBmp.h"
//#include "AdvanceDlg.h"
// CDeviceConfigureDlg 对话框



class CDeviceConfigureDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDeviceConfigureDlg)

public:
	CDeviceConfigureDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDeviceConfigureDlg();
	void SetVideoDevice(HVideoManager* pVideoDevice,HVideoRender* pRender);
	void SetVideoDevice(HVideoManager* pVideoDevice,HVideoRender* pRender,CString strCamraName);
	void SetVideoDevice(HVideoManager* pVideoDevice,CString strCamraName);
	HVideoDevice* GetDevice() {return m_VideoDevice;};
	CString strDeviceName;

	CWnd m_AdvancedWnd;
// 对话框数据
	enum { IDD = IDD_VIDEOCONFIG };

protected:
	HVideoDevice* m_VideoDevice;
	HVideoManager* m_VideoManager;
	HVideoRender* m_VideoRender;
	CRenderWnd m_RenderWnd;
  UINT_PTR m_timer;
public:
	//int m_Vexposure;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
	CSliderCtrl m_exposure;
	CComboBox m_device;
	CComboBox m_bining;
	CButton m_CheckAuto;
	float m_Vexposure;
	afx_msg void OnCbnSelchangeComboBin();
	afx_msg void OnCbnSelchangeComboDev();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedAdvanced();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheckIn();
	afx_msg void OnBnClickedCheckSoft();
	afx_msg void OnBnClickedCheckTime();
	afx_msg void OnBnClickedCheckOut();
	afx_msg void OnBnClickedResetgbtn();
	afx_msg void OnBnClickedClosebtn();
	afx_msg void OnBnClickedOpenbtn();
	afx_msg void OnBnClickedMirror();
	afx_msg void OnBnClickedFlip();
//private:
//	bool m_bFitWndows;
	virtual BOOL OnInitDialog();
  afx_msg void OnBnClickedTrigger();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
};
