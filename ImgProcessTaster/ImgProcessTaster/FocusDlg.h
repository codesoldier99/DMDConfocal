#pragma once
#include "HCoreProcess.h"
#include "afxwin.h"
#include <vector>
using std::vector;

// CFocusDlg 对话框

class CFocusDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFocusDlg)

public:
	CFocusDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFocusDlg();
	bool SetProcess(HCoreProcess* mProcess);
	bool SetFunction(HEasyFunction* mF);

// 对话框数据
	enum { IDD = IDD_FocusDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOpenimgbtn();

private:
	HCoreProcess* mAutoProcess;
	HEasyFunction* mEasyFunction;

	vector<CStringA> imgFiles;//选择所有图像文件
	vector<CStringA> DoSelectFiles(LPCTSTR lpszDefExt,DWORD	dwFlags,LPCTSTR	lpszFilter,
		LPCWSTR	lpstrTitle,LPCWSTR	lpstrInitialDir);
	HVideoHeader pheader;

public:
	CListBox GayValue;
	CListBox GridentValue;
	int iCuScale;
	int iXiScale;
	AutoFocusParas mFocusParas;
	afx_msg void OnBnClickedCaptureimgbtn();
	afx_msg void OnBnClickedClearbtn();
	afx_msg void OnLbnDblclkList1();
	afx_msg void OnLbnDblclkList2();
	afx_msg void OnBnClickedViewimgbtn();
	virtual BOOL OnInitDialog();
};
