#pragma once
#include "hconfocalcore.h"
#include "GlHeight.h"


class CGLviewWnd :public CWnd,public HCorePanel
{
public:
	CGLviewWnd(void);

	~CGLviewWnd(void);

	//HCorePanel 虚函数
	virtual CWnd* GetCWnd() {return this;};
	virtual bool InitPanel(LPVOID p_Param, LPVOID p_Param2 = 0);
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);
private:

protected:
	CGlHeight m_GlHeight;//绘等高图
	BOOL SetWindowPixelFormat(HDC hDC);
	int m_GLPicexIndex;
	BOOL CreatViewGLContext(HDC hDC);
	HGLRC m_hGLContext;
	HDC m_hDC;

	//opengl 视角操作

	float m_RotateX;
	float m_RotateY;
	float m_RotateZ;
	float m_RotateXR,m_RotateYR,m_RotateZR;
	float m_Scale;
	CPoint m_PointLast,m_PointEnd;
	bool m_bRotate;
	void FlushView();
	HConfocalCore* m_ConfocalCore;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

