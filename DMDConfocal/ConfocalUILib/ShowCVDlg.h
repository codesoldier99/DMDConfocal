#pragma once
#include "resource.h"
#include "HConfocalCore.h"
#include "WndBmp.h"
#include "ChartCtrl.h"
#include "ChartLineSerie.h"
#include "ChartBarSerie.h"
#include "ChartAxis.h"
#include "afxcmn.h"

// CShowCVDlg 对话框

class CShowCVDlg : public CDialogEx,public HCorePanel,public HVideoRender
{
	DECLARE_DYNAMIC(CShowCVDlg)
public:
	CShowCVDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShowCVDlg();
	virtual bool InitPanel(LPVOID p_Param, LPVOID p_Param2 = 0);// {return 0;};
	virtual bool UnInitPanel() {return 0;};
	virtual CWnd* GetCWnd(){return this;};//得到当前窗口
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0,float fParam=0.0f,void* mParam=0);

// 对话框数据
	enum { IDD = IDD_ShowCVDlg };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStartbtn();
	virtual void OnCancel();

private:
	HConfocalCore*	m_ConfocalCore;
	HVideoHeader m_CaptureVideo;//帧频的数据
	HCoreProcess* CalCVProcess;
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	int iCount;
	bool bCalNow;//指示有没有在进行计算均值，或者使用均值模板
	bool bTh;
	HANDLE m_hCaptureEvent;
	friend DWORD WINAPI CalProc(LPVOID lp);
	void CalImage();
	CRenderWnd m_renderwnd;

	HANDLE m_hMoveEvent;
	HANDLE m_CalCVThread;//计算CV平均的线程
	friend DWORD WINAPI SetCVProc(LPVOID lp);
	void SetCVImage();
	bool bSetCVImage;

	bool MoveAxisRef(HAxis	*m_Axis,float mPos,CString strOut);
	bool Capture();
	HAxis *m_AxisX,*m_AxisY;

	CalCVParas* mCVParas;
	float MoveStep;//移动步长

private:
	CChartCtrl m_ChartCV;  
	CChartCtrl m_HistView;  
	int m_ArrayCount,m_realArryCount;
	double *m_CVArray;//[300];
	double *m_X;//[300];
	double *m_HistX;
	double *m_HistValue;
	Point* m_Pt;
	CChartLineSerie* m_pLineCVSerie;
	CChartBarSerie* m_BarSerie;
	void LeftMoveArray(double* ptr,int length,double data);
	bool bSetCal;
	void ReadProcessParas();	

public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	int m_iTh;
	afx_msg void OnBnClickedCheck1();
	CSliderCtrl mThSlitder;
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedSentcalbtn();
	afx_msg void OnBnClickedCheck3();
};
