
// DMDConfocalView.h : CDMDConfocalView 类的接口
//

#pragma once


class CDMDConfocalView : public CView
{
protected: // 仅从序列化创建
	CDMDConfocalView();
	DECLARE_DYNCREATE(CDMDConfocalView)

// 特性
public:
	CDMDConfocalDoc* GetDocument() const;
	void SetShowWnd(CWnd* p_Wnd);

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CDMDConfocalView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	CWnd* m_Wnd;
	bool m_bools;

protected:

// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

#ifndef _DEBUG  // DMDConfocalView.cpp 中的调试版本
inline CDMDConfocalDoc* CDMDConfocalView::GetDocument() const
   { return reinterpret_cast<CDMDConfocalDoc*>(m_pDocument); }
#endif

