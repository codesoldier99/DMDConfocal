#pragma once
#include "Resource.h"

// CMsgViewBox 对话框

class CMsgViewBox : public CDialogEx,public HCorePanel
{
	DECLARE_DYNAMIC(CMsgViewBox)

public:
	CMsgViewBox(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMsgViewBox();
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);
	virtual bool InitPanel(LPVOID p_Param, LPVOID p_Param2 = 0);
	virtual CWnd* GetCWnd(){return this;};
// 对话框数据
	enum { IDD = IDD_MESSAGEDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	float fCount;


	DECLARE_MESSAGE_MAP()
public:
	CString m_StatusStr;
	CString m_TimeStr;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedEndbtn();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	CString m_InfoStr;
};
