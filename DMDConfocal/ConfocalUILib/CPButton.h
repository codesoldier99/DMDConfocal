
#pragma once
#include <afxwin.h>
#include <afxcmn.h>
//#include "stdafx.h"
#include "Resource.h"
#include "IHsmObserver.h"

//消息定义
//点击按钮的事件发生
#define EVENT_BUTTONDOWN 7550
//按钮弹起的事件发生
#define EVENT_BUTTONUP 7551
//按钮点击事件发生
#define EVENT_BUTTONCLICKED 7552
// CCPButton
//可以捕捉鼠标按下去弹起来消息的一个按钮类
//Check Press Button
class CCPButton : public CButton,public IHsmSubject
{
	DECLARE_DYNAMIC(CCPButton)

public:
	CCPButton();
	virtual ~CCPButton();

public:
	//int Attach(IPtnObserver * pObserver);
	//int Detach(IPtnObserver * pObserver);
	//int Notify(IPtnSubject * pSubject,int ID,long wParam = 0 ,void* pParam = 0 );
	void Check(bool bChecked);
	bool IsChecked();
private:
	CToolTipCtrl m_tip;
	BOOL m_bTracking;
	bool m_bOver;
	int w,h;
	bool m_bDown;
	bool m_bChecked;
	HICON m_hGrayIcon;
protected:
	DECLARE_MESSAGE_MAP()
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void PreSubclassWindow();
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};