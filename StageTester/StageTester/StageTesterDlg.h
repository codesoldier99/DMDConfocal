
// StageTesterDlg.h : 头文件
//

#pragma once

#include "HConfFile.h"
#include "HGearBox.h"
#include "afxwin.h"
#include "afxcmn.h"

typedef void* (*PluginLib_GetInterface)(void);
typedef CString (*PluginLib_GetName)(void);

// CStageTesterDlg 对话框
class CStageTesterDlg : public CDialogEx,public IHsmObserver
{
// 构造
public:
	CStageTesterDlg(CWnd* pParent = NULL);	// 标准构造函数
// 对话框数据
	enum { IDD = IDD_STAGETESTER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);

private:
	HGearBox* LoadDlls(CString t_name);
	void InitGearBox(HGearBox* pGearBox);
	void RestPanelValue();
	void FindBox();
	bool OpenBox(CString mPluginName);

protected:

	HConfigure m_conf;//配置文件
	HGearBox*  m_Gearbox;
	HAxis*	m_AxisX;
	HAxis*  m_AxisY;
	HAxis*	m_AxisZ;
	HAxis*	m_AxisW;
	HAxis*	m_AxisPIZ;
	HAxis*	m_AXIST;
	HAxis*	m_AXISO;

	bool m_MsgIn;
	CString strStagName;
	bool m_bJoyStick;
	int m_posx;
	int m_posy;
	CPoint m_posPoint;
	double scalex;
	double scaley;

// 实现
protected:
	HICON m_hIcon;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	CEdit m_MesaageDis;
	float m_XStep;
	float m_YStep;
	float m_ZStep;
	float m_XSpeed;
	float m_YSpeed;
	float m_ZSpeed; 
	CSliderCtrl m_XSlider;
	CSliderCtrl m_YSlider;
	CSliderCtrl m_ZSlider;
	CSliderCtrl m_XJoy;
	CSliderCtrl m_YJoy;
	CSliderCtrl m_ZJoy;
	CString m_TextField;
	float mXPos,mYPos,mZPos;
	int iLight;

	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlider2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlider3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton10();
	afx_msg void OnBnClickedButton11();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton12();
	afx_msg void OnBnClickedXstopbtn();
	afx_msg void OnBnClickedYstopbtn();
	afx_msg void OnBnClickedZstopbtn();
	afx_msg void OnBnClickedInitbtn();
	CString GetMoveStr(HAxis* mAxis,float fVStep);
	CComboBox mGearCoBox;
	int iObjt;
	afx_msg void OnBnClickedZbtn();
	afx_msg void OnBnClickedButton14();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedZresetbtn();
	double miPIZPos;
	afx_msg void OnBnClickedPizdownbtn();
	afx_msg void OnBnClickedPizupbtn();
	afx_msg void OnBnClickedPizresetbtn();
	afx_msg void OnBnClickedReadlbtn();
	CSliderCtrl mSliderLight;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedZbtn3();
	afx_msg void OnBnClickedZresetbtn2();
	int iTurn;
};
