
// WLPDMDTesterDlg.h : 头文件
//

#pragma once
#include "IDMDManager.h"
#include "afxwin.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "afxcmn.h"
using namespace cv;
using namespace WlpDMDControl;

typedef void* (*PluginLib_GetInterface)(void);
typedef CString (*PluginLib_GetName)(void);

// CWLPDMDTesterDlg 对话框
class CWLPDMDTesterDlg:public CDialogEx,public IHsmObserver
{
// 构造
public:
	CWLPDMDTesterDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_WLPDMDTESTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	CString GetModulePath();
public:
	afx_msg void OnBnClickedSavepointbtn();
	afx_msg void OnBnClickedSavelinebtn();
	int mPointSize;
	int mPtSpace;
	int mLineW;
	int mLineCount;
	//CWlpDMDLib mWlpDMDLib;
	IDMDManager* mDMDManager; 
	WlpDMDParas mWlpDMDParas;

	int m_FrameMargin;
	int m_FrameInterval;
	int iGray;
	int delay;
	int plusewidth;
	CComboBox m_trigger_mode;
	afx_msg void OnBnClickedInitbtn();
	afx_msg void OnBnClickedOpenimgbtn();
	afx_msg void OnBnClickedDownpointbtn();
	afx_msg void OnBnClickedDownlinetbtn();
	afx_msg void OnBnClickedStartbtn();
	afx_msg void OnBnClickedTriggerbtn();
	afx_msg void OnBnClickedStopbtn();
	afx_msg void OnBnClickedBrightbtn();
	afx_msg void OnBnClickedBlackbtn();
	CComboBox mGray;
	afx_msg void OnCbnSelchangeCombo2();

	virtual int OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);

private:
	void OpenCVShowImg(LPBYTE pBuffer,int iW, int iH, int wBits,std::string strN, double scale = 1.0);
	bool OpenFile(LPBYTE* pBuffer,int& IWidth, int& IHeight, int& wBitsPerPixel,int iColor,CString &strFile);

  void GetDMDSize(int* width, int* height);
public:
	afx_msg void OnBnClickedUpdatebtn();
	afx_msg void OnEnChangeEdit5();
	void CalDMDTime();
	void* LoadDlls(CString t_name);
	afx_msg void OnBnClickedChecksave();
	CString m_StrPath;
	bool BuildDirectory(CString strPath);
	char* pixel2bin(char* pSrc, int width,int height, unsigned char bitmask, char* pOut);
	char* bin2pixel(char* pSrc, int width,int height, unsigned char bitmask, char* pOut);
	int miResolution;
	int miLineType;
	CSliderCtrl m_ExSlider;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	float m_Vexposure;

	int mPtExtraW;
	int mLExtraW;
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnCbnSelchangeCombo1();
  CComboBox m_dev_type;
};
