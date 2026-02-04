#pragma once
#include "Resource.h"
#include "HConfocalCore.h"

#include <Gdiplus.h>
using namespace Gdiplus;

// CMapViewDlg 对话框

class CMapViewDlg : public CDialogEx,public HCorePanel
{
	DECLARE_DYNAMIC(CMapViewDlg)

public:
	CMapViewDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMapViewDlg();

// 对话框数据
	enum { IDD = IDD_MAPVIEW };

	virtual bool InitPanel(LPVOID p_Param, LPVOID p_Param2 = 0);
	virtual CWnd* GetCWnd() {return this;};
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);

private:
	//以下为载物台接口
	HConfocalCore* m_ConfocalCore;
	HAxis *m_AxisX,*m_AxisY;
	float m_AxisX_Max,m_AxisY_Max;//地图整体范围大小
	CPointF mClocalPt;//当前的位置 mm为单位
	CPointF mScanSPt;//扫描起点 mm为单位
	CPointF mScanEPt;//扫描终点 mm为单位

	//参数
	LPBYTE m_MapDate;

	float ROITraveSize;//ROI的移动距离大小 mm为单位
	CPoint m_ROIStartPt,m_ROISize;//ROI起始位置，ROI大小
	CPoint m_MapStart,m_MapSize;// 地图相对与控件中显示的起始位置，地图的大小，用于可以计算出移动的范围
	CPoint m_DrawROIPt,m_DrawROISize,m_DrawROIEndPt;//扫描的区域ROI

	bool bDrawRec;//绘制ROI区域
	CPoint m_BROIDragPoint;//ROI改变的位置大小
	//bool bArrived;//看是否响应绘制

	float AllScale;//算比例

	//地图显示以及Gui的参数
	BITMAPINFO* m_bmi;
	ULONG_PTR _gdiplusToken;
	SolidBrush* brush;
	CRect rcClient;

	void DrawDoubleDC(CDC* pDC,CRect pRc);
	void DrawMap(CDC* pDC,CRect pRc);
	void DrawROIS( CDC* pDC);

	void MoveStage( CPoint p_Pos );//移动了载物台
	void CorrectPt(CPoint* mPT);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
};
