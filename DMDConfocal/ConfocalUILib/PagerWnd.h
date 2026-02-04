#pragma once

#include "Pager.h"
// PagerWnd

class PagerWnd : public CWnd
{
	DECLARE_DYNAMIC(PagerWnd)

public:
	PagerWnd();
	virtual ~PagerWnd();

	void SetChild(CWnd *pChild);

	void SetScrollSize(int cx,int cy);
private:
	void OnPagerCalcSize(NMHDR *pHdr,LRESULT *pResult);
private:
	CPager m_pager;

	CWnd *m_pChild;
	
	CSize m_sz;
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


