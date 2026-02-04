// ScannerParas.cpp : 实现文件
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "ScannerParas.h"
#include "afxdialogex.h"


// CScannerParas 对话框

IMPLEMENT_DYNAMIC(CScannerParas, CDialogEx)

CScannerParas::CScannerParas(CWnd* pParent /*=NULL*/)
	: CDialogEx(CScannerParas::IDD, pParent)
	, m_iScale(0)
	, m_ImgSize(0)
	, m_iStep(0)
{

}

CScannerParas::~CScannerParas()
{
}

void CScannerParas::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT5, m_iScale);
	DDX_Text(pDX, IDC_EDIT6, m_ImgSize);
	DDX_Text(pDX, IDC_EDIT7, m_iStep);
}


BEGIN_MESSAGE_MAP(CScannerParas, CDialogEx)
	ON_BN_CLICKED(IDC_SaveBtn, &CScannerParas::OnBnClickedSavebtn)
	ON_BN_CLICKED(IDC_CancelBtn, &CScannerParas::OnBnClickedCancelbtn)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CScannerParas 消息处理程序


BOOL CScannerParas::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	mReBuildParas.iScaleV=m_ConfFile.GetInt(L"3DRebuild",L"ScaleV",L"Confocal_Scaner");//尺度缩小
	mReBuildParas.iImgSize=m_ConfFile.GetInt(L"3DRebuild",L"ImgSize",L"Confocal_Scaner");//贴图大小
	mReBuildParas.iIdxSize=m_ConfFile.GetInt(L"3DRebuild",L"IdxSize",L"Confocal_Scaner");//高度图大小
	mReBuildParas.iImgStep=m_ConfFile.GetInt(L"3DRebuild",L"ImgStep",L"Confocal_Scaner");//高度图步长

	m_iScale=mReBuildParas.iScaleV;
	m_ImgSize=mReBuildParas.iImgSize;
	m_iStep=mReBuildParas.iImgStep;

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CScannerParas::OnBnClickedSavebtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	m_ConfFile.RecValue(L"3DRebuild",L"ScaleV",m_iScale,L"Confocal_Scaner");
	m_ConfFile.RecValue(L"3DRebuild",L"ImgSize",m_ImgSize,L"Confocal_Scaner");
	m_ConfFile.RecValue(L"3DRebuild",L"ImgStep",m_iStep,L"Confocal_Scaner");
	CDialogEx::OnOK();
}


void CScannerParas::OnBnClickedCancelbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


HBRUSH CScannerParas::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	switch(pWnd->GetDlgCtrlID())
	{
	case IDC_STATIC:
		pDC->SetBkMode(TRANSPARENT);
		break;
	}
	// TODO:  在此更改 DC 的任何特性
	return afxGlobalData.brBarFace;
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}
