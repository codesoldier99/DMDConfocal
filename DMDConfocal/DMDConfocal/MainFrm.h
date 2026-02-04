
// MainFrm.h : CMainFrame 类的接口
//

#pragma once
#include "DockDlg.h"

class CMainFrame : public CMDIFrameWndEx,public IHsmObserver
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// 特性
public:

// 操作
public:

// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);
// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 控件条嵌入成员
	CMFCMenuBar       m_wndMenuBar;
	//CMFCToolBar       m_wndToolBar;
	CMFCToolBar       m_wndToolBar;
	CMFCToolBar       m_wndMeasureBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;

	CDockDlg		m_wndPane;
	HConfocalCore*	m_ConfocalCore;

	//载物台
	HAxis* AxisX,*AxisY,*AxisZ;
	float PosX,PosY,PosZ;

	//是否锁定对话框
	bool bLockDlg;
	bool bToolIcon;

// 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT OnAxisUpdata(WPARAM wParam,LPARAM lParam);
	DECLARE_MESSAGE_MAP()
	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
	afx_msg void OnUpdateIdsStatusPanelX(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIdsStatusPanelY(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIdsStatusPanelZ(CCmdUI *pCmdUI);
	afx_msg void OnUpdateLockDlg(CCmdUI *pCmdUI);
	afx_msg void OnUpdateLargeIcon(CCmdUI *pCmdUI);
public:
	afx_msg void OnViewLock();
	afx_msg void OnDefaultview();
	afx_msg void OnViewlargetool();
	afx_msg void OnCapture();
	afx_msg void OnMeasureline();
	afx_msg void OnMeasureround();
	afx_msg void OnMeasurerec();

	//bool InitAllDlg();

private:
	void LockAllSliders(bool bLock);//获取所有的CPaneDivider去设置它们Diable
	void LockToolBar(bool bLock);
public:
	afx_msg void OnHelpmenubtn();
	afx_msg void OnMenucvbtn();
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	afx_msg void OnToolsDefaultlayout();
};


