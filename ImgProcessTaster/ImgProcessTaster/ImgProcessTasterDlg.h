
// ImgProcessTasterDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "HCoreProcess.h"
#include "FocusDlg.h"
#include "MutiFocusDlg.h"
#include "MutiBuild.h"
#include "PreProcessDlg.h"
#include "TestDlg.h"

typedef void* (*PluginLib_GetInterface)(void);
typedef CString (*PluginLib_GetName)(void);

// CImgProcessTasterDlg 对话框
class CImgProcessTasterDlg : public CDialogEx
{
// 构造
public:
	CImgProcessTasterDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_IMGPROCESSTASTER_DIALOG };
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	bool InitDlg();

	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl mTab;

private:
	//添加窗口的步骤：1、Dlg的属性，Boder 为none style为 Child；2、再mTab.InsertItem 中加进去，3、根据其他窗口方式，添加
	CFocusDlg mFocusDlg;
	CMutiFocusDlg mMapDlg;
	CMutiBuild mMutiDlg;
	CPreProcessDlg mPreDlg;
	TestDlg mTestDlg;
	HCoreProcessMg* mProcessMg;

public:
	void* LoadDlls(CString t_name);
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
};
