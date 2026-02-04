#pragma once

#include "Resource.h"
// CScannerParas 对话框

class CScannerParas : public CDialogEx
{
	DECLARE_DYNAMIC(CScannerParas)

public:
	CScannerParas(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScannerParas();

	HConfigure m_ConfFile;
	ReBuildParas mReBuildParas;
// 对话框数据
	enum { IDD = IDD_3DScanParas };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int m_iScale;
	int m_ImgSize;
	int m_iStep;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedSavebtn();
	afx_msg void OnBnClickedCancelbtn();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
