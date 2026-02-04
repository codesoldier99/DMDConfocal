#pragma once

#include "afxwin.h"
#include "Resource.h"
#include "HConfocalCore.h"

#include <Gdiplus.h>
using namespace Gdiplus;
#include "SIViewPanel.h"
#include <vector>
using namespace std;

class DiffMeasureView :public CWnd,public HCorePanel,public HVideoRender
{
public:
	DiffMeasureView(void);
	~DiffMeasureView(void);
	virtual CWnd* GetCWnd() {return this;};
	virtual bool InitPanel(LPVOID p_Param,LPVOID p_Param2=0);
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);

	void Render(LPBYTE pBuffer, int width,int heidth,int wbit );
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
/*private:
private:
  struct DiffMeasureResult
  {
    DiffMeasureResult()
    {
      width = 0;
      height = 0;
      a = 0;
      b = 0;
      c = 0;
    }
    int width;
    int height;
    unsigned char* a;
    unsigned char* b;
    unsigned char* c;
  };*/
private:
	void FlushView();
	void medianFilter(double* corrupted, double* smooth, int width, int height);

	HConfocalCore* m_ConfocalCore;

  SIViewPanel _panel;

  int _width;
  int _height;
  vector<unsigned char> _imgs[3];
  vector<double> _height_img;

  DiffMeasureParam _dmp;
};

