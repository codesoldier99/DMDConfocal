
// VideoTesterDlg.h : 头文件
//
#pragma once

#include "WndBmp.h"
#include "DeviceConfigureDlg.h"
#include "CarlVideo.h"
#include "afxwin.h"

typedef void* (*PluginLib_GetInterface)(void);
typedef CString (*PluginLib_GetName)(void);

// CVideoTesterDlg 对话框
class CVideoTesterDlg : public CDialogEx,public IHsmObserver
{
// 构造
public:
	CVideoTesterDlg(CWnd* pParent = NULL);	// 标准构造函数

	//CRenderWnd m_renderwnd;
	CDeviceConfigureDlg m_DeviceConf;
	HVideoType* mVideoType;
	HVideoManager* m_VideoManager;
	HVideoDevice* m_VideoDevice;

	enum { IDD = IDD_VIDEOTESTER_DIALOG };// 对话框数据
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual int OnSubjectNotified(IPtnSubject * pSubject, int ID,long wParam = 0,
		void* pParam = 0,float fParam=0.0f,void* mParam=0);

// 实现
protected:
	HICON m_hIcon;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	HVideoType* LoadDlls(CString t_name);
	CComboBox mCoCameraTypes;
	CComboBox mCoCameraName;

	afx_msg void OnBnClickedBtnCam();
	afx_msg void OnBnClickedBtnGrab();
	afx_msg void OnBnClickedBtnTriger();
	afx_msg void OnCbnSelchangeCombotype();
	afx_msg void OnBnClickedBtnstart();
	afx_msg void OnCbnSelchangeCombocamera();
	afx_msg void OnBnClickedClosecamerabtn();
	afx_msg void OnBnClickedBtncapture();
	afx_msg void OnBnClickedCreatebtn();

private:
	CDeviceConfigureDlg mDeviceNewList[10];
	int miCameraCreateCount;
public:
  afx_msg void OnDestroy();
};
