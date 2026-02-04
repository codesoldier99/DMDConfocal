#pragma once

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/stitcher.hpp"
//#include "opencv2/opencv.hpp"
#include <vector>
#include "afxwin.h"
using std::vector;
using namespace cv;
// TestDlg 对话框

class TestDlg : public CDialogEx
{
	DECLARE_DYNAMIC(TestDlg)

public:
	TestDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~TestDlg();

// 对话框数据
	enum { IDD = IDD_TestDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCombinebtn();

private:
	vector<CStringA> imgFiles;
	vector<CStringA> DoSelectFiles(LPCTSTR lpszDefExt,DWORD	dwFlags,LPCTSTR	lpszFilter,
		LPCWSTR	lpstrTitle,LPCWSTR	lpstrInitialDir);

	void SetStitcher(void* mP);
public:
	int miSize;
	afx_msg void OnBnClickedUsesize();
	afx_msg void OnBnClickedNewcombinebtn();

	double Match_8bits(const unsigned char* pTmplBuf,int tw,int th,
		const unsigned char* pSearchBuf,int sw,int sh,int &x,int &y,int step);
	void CutImg(Mat src,Mat& dst,CRect mV);
	void ComBineImg(Mat src,Mat dst,Mat& mCombine,Point mPt);

	void CalSearchRect(Rect& srcRect,Rect& dstRect);

private:
	enum Dir//方向定义
	{
		none = -1
		,left = 0
		,right = 1
		,top = 2
		,bottom = 3
	};

	struct AssemblyData
	{
		unsigned int m_rows,m_cols;//行列数 --要取的模板大小
		int m_overlap;//相邻图像重叠率（像素）
		int TempleSize;

		//int m_overlapH,m_overlapV;//相邻图像重叠率（像素）
		//ImageHeader m_thdr;//单元图像格式
		//int m_nApc;//已拼接的图片数
		//mCOLORREF m_bgColor;//拼图默认背景色
		//float m_bgEval;//背景区域评估值，值最小的区域为背景区域，此区域的灰度平均值为拼图背景色			
		//mRECT m_ovlpL,m_ovlpR,m_ovlpT,m_ovlpB;//重叠区域,左、右、上、下
		//mRECT m_minOvlpL,m_minOvlpR,m_minOvlpT,m_minOvlpB;//最小重叠区域,左、右、上、下	
		//int m_toleranceH,m_toleranceV;//相邻图像最大可能相对位移（像素）	
		//int _maxDeltaX,_maxDeltaY;//x,y方向最大偏移量。
		//int _minBg;//最小背景灰度，低于此灰度的不作为背景色填充。
		//int _defaultDeltaX,_defaultDeltaY;
		//聚焦和拼接因子比例
		//float m_ratio;
	};


	Dir _dr;
	AssemblyData _ad;
public:
	int iCutSize;
	int iTempleSize;
	int iCurRation;
	afx_msg void OnBnClickedNewcombinebtn2();
	CListBox mResultList;

private:
	int iCalCount;
public:
	afx_msg void OnLbnDblclkList2();
	afx_msg void OnBnClickedReadimgbtn();
};
