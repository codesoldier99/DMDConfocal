// SI3DPanel.cpp : implementation file
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "SI3DPanel.h"
#include "afxdialogex.h"
#include "resource.h"

// SI3DPanel dialog

IMPLEMENT_DYNAMIC(SI3DPanel, CDialogEx)


static void handle_double_click(vtkObject* obj, unsigned long,void*, void*)
{
  vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(obj);
  if (iren && iren->GetRepeatCount())
  {
    //AfxMessageBox(_T("Double Click"));
  }
}

SI3DPanel::SI3DPanel(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SI3DPANEL, pParent)
{
  this->pvtkMFCWindow = NULL;
  this->pvtkRenderer = vtkRenderer::New();
  //this->pvtkActor = vtkActor::New();
}

SI3DPanel::~SI3DPanel()
{
}

void SI3DPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(SI3DPanel, CDialogEx)
  ON_BN_CLICKED(IDOK, &SI3DPanel::OnBnClickedOk)
  ON_BN_CLICKED(IDCANCEL, &SI3DPanel::OnBnClickedCancel)
  ON_WM_DESTROY()
  ON_WM_SIZE()
END_MESSAGE_MAP()

// SI3DPanel message handlers

void SI3DPanel::OnBnClickedOk()
{
  return;
}

void SI3DPanel::OnBnClickedCancel()
{
  return;
}


BOOL SI3DPanel::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  
  // adjust dialog & window size
  this->pvtkMFCWindow = new vtkMFCWindow(this->GetDlgItem(IDC_3D_IMAGE));
  // set the vtk renderer, windows, etc
  this->pvtkMFCWindow->GetRenderWindow()->AddRenderer(this->pvtkRenderer);

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

void SI3DPanel::OnDestroy()
{
	// Delete the renderer, window and interactor objects.
	if (this->pvtkRenderer)
		this->pvtkRenderer->Delete();

	delete this->pvtkMFCWindow;
	CDialogEx::OnDestroy();
}

void SI3DPanel::OnSize(UINT nType, int cx, int cy)
{
  CDialogEx::OnSize(nType, cx, cy);
  if (::IsWindow(this->GetSafeHwnd()))
  {
	if (this->pvtkMFCWindow)
	{
		this->GetDlgItem(IDC_3D_IMAGE)->SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
		this->pvtkMFCWindow->SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}
  }
}

void SI3DPanel::SetHeightData(int width, int height, double* data, const DiffMeasureParam& dmp)
{
	SendMessage(WM_USER + 3);

	vtkSmartPointer<vtkImageData> imgCreate = vtkImageData::New();
	imgCreate->SetDimensions(width, height, 1);
	imgCreate->AllocateScalars(VTK_FLOAT, 1);

	vtkSmartPointer<vtkImageDataGeometryFilter> geo = vtkImageDataGeometryFilter::New();
	geo->SetInputData(imgCreate);
	geo->Update();

	vtkSmartPointer<vtkPolyData> input = geo->GetOutput();

	int numPts = input->GetNumberOfPoints();

	vtkSmartPointer<vtkPoints> newPts = vtkSmartPointer<vtkPoints>::New();
	newPts->SetNumberOfPoints(numPts);

	vtkSmartPointer<vtkFloatArray> derivs = vtkSmartPointer<vtkFloatArray>::New();
	derivs->SetNumberOfTuples(numPts);

	vtkSmartPointer<vtkPolyData> bessel = vtkSmartPointer<vtkPolyData>::New();
	bessel->CopyStructure(input);
	bessel->SetPoints(newPts);
	bessel->GetPointData()->SetScalars(derivs);

	double x[3];
	for (int i = 0; i < numPts; i++)
	{
		input->GetPoint(i, x);
		x[2] = (data[(int)x[0] + ((int)x[1]) * width]*dmp.coeff_k)/0.4;
		newPts->SetPoint(i, x);
		derivs->SetValue(i, -x[2]);
	}

	vtkSmartPointer<vtkWarpScalar> scalar = vtkWarpScalar::New();
	scalar->SetInputData(bessel);
	scalar->XYPlaneOn();
	scalar->SetScaleFactor(1.0);

	// Create a mapper and actor
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(scalar->GetOutputPort());

	double tmp[2];
	bessel->GetScalarRange(tmp);
	mapper->SetScalarRange(tmp[0], tmp[1]);

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	this->pvtkRenderer->AddActor(actor);
	this->pvtkRenderer->SetBackground(0.0, 0.0, 0.0);
	//this->pvtkRenderer->ResetCamera();
	if (this->pvtkMFCWindow)
		this->pvtkMFCWindow->RedrawWindow();
	Invalidate();

	SendMessage(WM_USER + 4);
	/*this->pvtkRenderer->RemoveActor(this->pvtkActor);
	pvtkActor->SetMapper(mapper);
	this->pvtkRenderer->AddActor(pvtkActor);
	this->pvtkRenderer->SetBackground(0.0, 0.0, 0.0);
	this->pvtkRenderer->ResetCamera();
	if (this->pvtkMFCWindow)
		this->pvtkMFCWindow->RedrawWindow();*/
}

LRESULT SI3DPanel::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	return CDialogEx::WindowProc(message, wParam, lParam);
}


BOOL SI3DPanel::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (message == WM_USER + 3)
	{
		vtkActorCollection* actorCollection = pvtkRenderer->GetActors();
		int num = actorCollection->GetNumberOfItems();
		actorCollection->InitTraversal();
		for (int i = 0; i < num; ++i)
		{
			vtkActor* curActor = actorCollection->GetNextActor();
			pvtkRenderer->RemoveActor(curActor);
		}
	}
	else if (message == WM_USER + 4)
	{
		this->pvtkRenderer->ResetCamera(); //重新置位观察点
	}
	return CDialogEx::OnWndMsg(message, wParam, lParam, pResult);
}


BOOL SI3DPanel::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	return CDialogEx::OnCommand(wParam, lParam);
}
