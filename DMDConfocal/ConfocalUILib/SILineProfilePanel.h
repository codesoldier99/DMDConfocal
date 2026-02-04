#pragma once

#include "resource.h"
#include "HeatmapWnd.h"
#include "ChartCtrl.h"
#include "ChartLineSerie.h"
#include "ChartBarSerie.h"
#include "ChartAxis.h"
#include <vector>
using namespace std;

// SILineProfilePanel dialog

class SILineProfilePanel : public CDialogEx
{
	DECLARE_DYNAMIC(SILineProfilePanel)

public:
	SILineProfilePanel(CWnd* pParent = NULL);   // standard constructor
	virtual ~SILineProfilePanel();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SILINEPROFILEPANEL };
#endif

  void SetHeatmap(int width, int height, double* data);
private:
  void SampleLine(int x0, int y0, int x1, int y1,
    vector<pair<int, int>>& pts);
private:
  HeatmapWnd _heatmap;
  int _width;
  int _height;
  vector<double> _data;
  CChartLineSerie* _line_serie;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedCancel();
  CChartCtrl _line_profile;
  virtual BOOL OnInitDialog();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};
