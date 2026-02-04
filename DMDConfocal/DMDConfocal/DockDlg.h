#pragma once
#include "afxdockablepane.h"
class CDockDlg :
	public CDockablePane
{
public:
	CDockDlg(void);
	~CDockDlg(void);

	void SetWnd(CWnd* pWnd);

	void LockPane(bool p_lock);

	void NewWnd(void);//ÐÂ´°¿Ú

	CDockDlg* GetNextWnd(void) {return m_WndNext;};
protected:
	CWnd* m_Wnd;
	CDockDlg* m_WndNext;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	int iW;
	int iH;
};

