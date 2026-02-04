#pragma once
#include "HCoreProcess.h"
#include "afxwin.h"
#include <vector>
using std::vector;

// CMutiFocusDlg 对话框

class CMutiFocusDlg : public CDialogEx,public IHsmObserver
{
	DECLARE_DYNAMIC(CMutiFocusDlg)

public:
	CMutiFocusDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMutiFocusDlg();
	bool SetProcess(HCoreProcess* mProcess);
	bool SetFunction(HEasyFunction* mF);

	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);

// 对话框数据
	enum { IDD = IDD_MutiFocusDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSelectbtn();

private:
	HCoreProcess* mReBuildProcess;
	HEasyFunction* mEasyFunction;

	ReBuildParas mParas;//参数
	DWORD  dwGTCBegin;//计时  
	vector<CString> imgFiles;//选择所有图像文件
	vector<CString> DoSelectFiles(LPCTSTR lpszDefExt,DWORD	dwFlags,LPCTSTR	lpszFilter,
		LPCWSTR	lpstrTitle,LPCWSTR	lpstrInitialDir);
	CString GetModulePath();

	HVideoHeader pheader;
	static void on_mouse(int event, int x, int y, int flags, void* ustc);
	void onMouse(int event, int x, int y);
public:
	afx_msg void OnBnClickedMergebtn();
	int mScale;
	int mImageStep;
	afx_msg void OnBnClickedViewimgbtn();
	afx_msg void OnBnClickedBtnfind();
};
