#pragma once
#include "resource.h"
#include "HCoreProcess.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
using namespace cv;

// CPreProcessDlg 对话框

class CPreProcessDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPreProcessDlg)

public:
	CPreProcessDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPreProcessDlg();

// 对话框数据
	enum { IDD = IDD_PreProcessDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:	
	//设置图像处理、图像预处理的接口
	bool SetPreProcess(HImgPreProcess* mProcess,CString strName);
	bool SetProcess(HImgProcess* mProcess,CString strName);
	

	//存储图像处理以及图像预处理的指针
	#define MAX_PROCESS	20 
	HImgPreProcess* mPreProcess[MAX_PROCESS];
	HImgProcess* mImgProcess[MAX_PROCESS];
	

	//图像处理、预处理的参数表
	ProcessParas mProcessParas;
	CString m_strParas0_0;
	int iScale;	
	double m_Gama;	
	int iTh;
	int mMinArea;
	int mMaxArea;
	int m_X;
	int m_Y;
	int m_Width;
	int m_Height;
	
	HVideoHeader pOriginHeader;//原始读取的图像，StepOn的时候就更新自己
	HVideoHeader pEndHeader;//如果没有StepOn，就作为临时存放图的地方

	bool OpenFile(int iColor);
	afx_msg void OnBnClickedOpenimgbtn();
	afx_msg void OnBnClickedScalebtn();
	afx_msg void OnBnClickedGamabtn();
	afx_msg void OnBnClickedBlockscanbtn();	
	afx_msg void OnBnClickedGraybtn();
	afx_msg void OnBnClickedCutbtn();
	afx_msg void OnBnClickedCalcvbtn2();
	afx_msg void OnBnClickedCallightbtn();
	afx_msg void OnBnClickedSetavebtn();

	//CV值计算相关
	bool SetCalProcess(HCoreProcess* mProcess);
	HCoreProcess* mCalProcess;
	int iCVUseImgs;//用于计算平衡化CV值的图像数
	int iUseImg;//计算平衡化CV值的图计数
	int iXStart;
	int iXEnd;
	int iYStart;
	int iYEnd;
	int m_iTh;
	CalCVParas mCVParas;
	afx_msg void OnBnClickedReadmaskbtn();
	afx_msg void OnBnClickedAddpiccountbtn();
	afx_msg void OnBnClickedReleasevectorbtn();
	
	//测试图像列表的内存相关
	vector<cv::Mat> mImgs;
	int iAddPicCount;
	afx_msg void OnBnClickedResetvectorbtn();
	afx_msg void OnBnClickedAwbbtn();
	afx_msg void OnBnClickedBtnfilter();
};
