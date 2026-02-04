#pragma once
#include "Resource.h"
#include "afxwin.h"
#include "CPButton.h"
#include <vector>
#include "afxcmn.h"
using std::vector;

typedef struct 
{
	CString strName;
	double PtX;
	double PtY;
	double PtZ;
}ObservePt;

// CStageCtlDlg 对话框
class CStageCtlDlg : public CDialogEx,public HCorePanel
{
	DECLARE_DYNAMIC(CStageCtlDlg)

public:
	CStageCtlDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CStageCtlDlg();
	virtual CWnd* GetCWnd() {return this;};
	virtual bool InitPanel(LPVOID p_Param, LPVOID p_Param2 = 0);
	virtual int OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);
// 对话框数据
	enum { IDD = IDD_STAGE_DLG };

	friend DWORD WINAPI InitPosProc(LPVOID lp);
	void InitPos();
	friend DWORD WINAPI SwitchObjProc(LPVOID lp);
	void SwitchObj();

protected:
	HGearBox* m_GearBox;
	HAxis	*m_AxisX,*m_AxisY,*m_AxisZ,*m_AxisW,*m_AxisT,*m_AxisO;
	HConfocalCore* m_ConfocalCore;
	HConfigure m_ConfFile;
	void SaveParas();

	float mZPos,mNowExp;//用来给转化线程传递参数
	int iObj;//用来给转化线程传递参数
	HVideoDevice* m_VideoDevice;
	float m_ZInitPos,m_XInitPos,m_YInitPos;//轴初始位置
	float i4XObj,i10XObj,i20XObj,i40XObj,iMidObj,i100XObj;//不同倍镜的Z轴位置
	float m_4XExp,m_10XExp,m_20XExp,m_40XExp,m_100XExp;//不同倍镜的曝光时间
	int iZoffset;
	void ReadPosValue();
	void MakeMsgDlg();
	void MoveAxis(HAxis	*m_Axis,float mPos,CString strOut);
	HANDLE m_hMoveEvent;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedYaddbtn();
	afx_msg void OnBnClickedXaddbtn();
	afx_msg void OnBnClickedXsubbtn();
	afx_msg void OnBnClickedYsubbtn();
	afx_msg void OnBnClickedZaddbtn();
	afx_msg void OnBnClickedZsubbtn();
	afx_msg void OnBnClickedResetx();
	afx_msg void OnBnClickedResety();
	afx_msg void OnBnClickedResetz();
	afx_msg void OnBnClickedStopx();
	afx_msg void OnBnClickedStopy();
	afx_msg void OnBnClickedStopz();
	int mRadio;
	float m_XYMove;
	float m_ZMove;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedRadio0();
	afx_msg void OnBnClickedInitbtn();
	CButton m_btn_top;
	//CCPButton m_btn_top;
	CComboBox mObserComBo;

private:
	void InitObservePt();
	int splitString(CString str, char split, CStringArray& strArray);
	vector<ObservePt> mObservePt;
public:
	afx_msg void OnCbnSelchangeCboobser();
	int iTurnOrder;
	afx_msg void OnBnClickedRadio6();
	CSliderCtrl mSliderLight;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
