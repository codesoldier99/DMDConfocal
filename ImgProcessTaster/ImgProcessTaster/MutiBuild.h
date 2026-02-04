#pragma once


// CMutiBuild 对话框

class CMutiBuild : public CDialogEx
{
	DECLARE_DYNAMIC(CMutiBuild)

public:
	CMutiBuild(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMutiBuild();

// 对话框数据
	enum { IDD = IDD_MutiBuild };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
