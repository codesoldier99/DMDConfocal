#pragma once
#include "Resource.h"

// CResizeDlg 对话框

class CResizeDlg : public CDialogEx,public HCorePanel
{
	DECLARE_DYNAMIC(CResizeDlg)

public:
	CResizeDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CResizeDlg();

// 对话框数据
	enum { IDD = IDD_RESIZE };

	void SetChildWnd(HCorePanel* p_ChildPanel);

	virtual bool InitPanel(LPVOID p_Param, LPVOID p_Param2 = 0) {if(m_pChildPanel) return m_pChildPanel->InitPanel(p_Param, p_Param2);};
	virtual CWnd* GetCWnd() {return this;};
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0,float fParam=0.0f,
		void* mParam=0);

protected:
	void UpdateScrollbar();	
	void UpdateChildPos();
	CWnd *m_pChild;
	bool m_bHorz;
	bool m_bVert;
	bool m_bScroll;
	HCorePanel* m_pChildPanel;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
