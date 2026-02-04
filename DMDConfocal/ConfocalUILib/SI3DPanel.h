#pragma once

/*#include "vtkMFCWindow.h"
#include "vtkSmartPointer.h"
#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkDataSetMapper.h"
#include "vtkDebugLeaks.h"
#include "vtkFloatArray.h"
#include "vtkPlaneSource.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkWarpScalar.h"
#include "vtkImageData.h"
#include "vtkImageDataGeometryFilter.h"
#include "vtkBMPReader.h"
#include "vtkPlaneWidget.h"
#include "vtkCallbackCommand.h"
#include "vtkWin32OpenGLRenderWindow.h"
#include "vtkPolyDataMapper.h"*/

#include "vtkMFCWindow.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkRenderer.h"
#include "vtkWarpScalar.h"
#include "vtkImageData.h"
#include "vtkImageDataGeometryFilter.h"
#include "vtkWin32OpenGLRenderWindow.h"
#include "vtkPolyDataMapper.h"

// SI3DPanel dialog

class SI3DPanel : public CDialogEx
{
	DECLARE_DYNAMIC(SI3DPanel)

public:
	SI3DPanel(CWnd* pParent = NULL);   // standard constructor
	virtual ~SI3DPanel();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SI3DPANEL };
#endif
	// 设置高度数据
	void SetHeightData(int width, int height, double* data, const DiffMeasureParam& dmp);

private:
	vtkMFCWindow          *pvtkMFCWindow;
	vtkRenderer           *pvtkRenderer;
	vtkActor              *pvtkActor;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};
