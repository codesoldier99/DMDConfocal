#pragma once


// CPager

class CPager : public CWnd
{
	DECLARE_DYNAMIC(CPager)

public:
	CPager();
	virtual ~CPager();

	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	void SetChild(CWnd *pWnd);

	void SetBorder(int iBorder);

	void SetHorz(bool bHorz);
protected:
	DECLARE_MESSAGE_MAP()
private:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
};


