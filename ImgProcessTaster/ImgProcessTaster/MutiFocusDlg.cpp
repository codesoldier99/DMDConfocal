// CMutiFocusDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ImgProcessTaster.h"
#include "MutiFocusDlg.h"
#include "afxdialogex.h"
#include "HGloableFunction.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/stitcher.hpp"
using namespace cv;


// CMutiFocusDlg 对话框

IMPLEMENT_DYNAMIC(CMutiFocusDlg, CDialogEx)

CMutiFocusDlg::CMutiFocusDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMutiFocusDlg::IDD, pParent)
	, mScale(2)
	, mImageStep(3)
{
	mEasyFunction=0;
	mReBuildProcess=0;
	pheader.Vbuffer=0;
	pheader.VwBit=pheader.Vwidth=pheader.Vheight=0;
}

CMutiFocusDlg::~CMutiFocusDlg()
{
}

bool CMutiFocusDlg::SetProcess(HCoreProcess* mProcess)
{
	mReBuildProcess=mProcess;
	mReBuildProcess->Attach(this);
	return true;
}

bool CMutiFocusDlg::SetFunction(HEasyFunction* mF)
{
	mEasyFunction=mF;
	return true;
}

void CMutiFocusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDITSCale, mScale);
	DDX_Text(pDX, IDC_EDITStep, mImageStep);
}


BEGIN_MESSAGE_MAP(CMutiFocusDlg, CDialogEx)
	ON_BN_CLICKED(IDC_SelectBtn, &CMutiFocusDlg::OnBnClickedSelectbtn)
	ON_BN_CLICKED(IDC_MergeBtn, &CMutiFocusDlg::OnBnClickedMergebtn)
	ON_BN_CLICKED(IDC_ViewImgBtn, &CMutiFocusDlg::OnBnClickedViewimgbtn)
	ON_BN_CLICKED(IDC_BTNFIND, &CMutiFocusDlg::OnBnClickedBtnfind)
END_MESSAGE_MAP()


// CMutiFocusDlg 消息处理程序

void CMutiFocusDlg::OnBnClickedSelectbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	pheader.VcaptureTime=0;
	imgFiles.clear();
	imgFiles = DoSelectFiles(_T("*.bmp"), 
		OFN_ENABLESIZING   |OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY,
		_T("image files (*.bmp; *.png; *.jpg) |*.bmp; *.png; *.jpg; *.jpeg| All Files (*.*) |*.*||"),
		_T("SelectImg"),_T("Imgs\\Left"));
	pheader.VnCount=imgFiles.size();
	if (pheader.VnCount<=0)
	{
		AfxMessageBox(L"没有图像！");
		return;
	}

	CString str=(CString)imgFiles[0];
	SetDlgItemText(IDC_EDITPath,str);
	str.Format(L"%d pictures have been selected.",pheader.VnCount);
	SetDlgItemText(IDC_EDITCount,str);
}

vector<CString> CMutiFocusDlg::DoSelectFiles(LPCTSTR lpszDefExt,DWORD dwFlags,LPCTSTR lpszFilter,
	LPCWSTR	lpstrTitle,LPCWSTR	lpstrInitialDir)
{
	vector<CString> selectedFiles;
	POSITION filePosition;
	DWORD MAXFILE = 4000;  
	TCHAR* pc = new TCHAR[MAXFILE];  
	CFileDialog dlg( TRUE, lpszDefExt, NULL, dwFlags, lpszFilter, NULL );	
	dlg.m_ofn.nMaxFile = MAXFILE; 
	dlg.m_ofn.lpstrFile = pc;   
	dlg.m_ofn.lpstrFile[0] = NULL; 
	dlg.m_ofn.lpstrTitle = lpstrTitle;
	dlg.m_ofn.lpstrInitialDir = lpstrInitialDir;
	if( dlg.DoModal() == IDOK )
	{
		filePosition = dlg.GetStartPosition();
		while(filePosition != NULL)   
		{   
			CString path;
			path = dlg.GetNextPathName(filePosition);
			selectedFiles.push_back( path );  
		}  
	}
	delete []pc;
	return selectedFiles;
}


void CMutiFocusDlg::OnBnClickedMergebtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (!mEasyFunction||!mReBuildProcess||imgFiles.size()<=0)
		return;

	if (mEasyFunction->OpenFile(imgFiles[0],&pheader,-1))
	{
		mParas.iImgSize=pheader.Vheight/mScale;
		mParas.iScaleV=mScale;
		mParas.iImgStep=mImageStep;
		mParas.strBKImg=L"";
		mParas.iIdxSize=256;
		mReBuildProcess->InitProcess(&mParas);
	}
	dwGTCBegin=GetTickCount();
	//SetDlgItemText(IDC_EDITTime,L"In Merging!");
	//UpdateData();
	for (int i=0;i<imgFiles.size();i++)
	{
		if (mEasyFunction->OpenFile(imgFiles[i],&pheader,-1))
		{
			mReBuildProcess->GetVideoRender()->Renderer(&pheader,pheader.Vbuffer);
		}
	}
}

int CMutiFocusDlg::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	CString strPath=GetModulePath();
	if(ID==NOTIFY_RESULT_HEIGHT)
	{
		LPBYTE pBuffer=(LPBYTE)pParam;
		strPath+=L"\\ResultHeight.bmp";
		
		HVideoHeader mImg;
		mImg.Vheight=mImg.Vwidth=wParam;
		mImg.VwBit=8;
		mImg.Vbuffer=pBuffer;
		mImg.Vsize=wParam*wParam*8/8;
		mReBuildProcess->SavePic(&mImg,strPath,0);

		CString strSystemDir;
		::GetSystemDirectory(strSystemDir.GetBuffer(256), 256);
		strSystemDir.ReleaseBuffer();
		CString strRundll;
		strRundll = strSystemDir + L"\\rundll32.exe ";
		CString strParm;
		strParm.Format(L"%s\\shimgvw.dll,ImageView_Fullscreen %s", strSystemDir, strPath);
		ShellExecute(NULL,L"Open",strRundll,strParm,NULL,SW_SHOWNORMAL);
	}
	else if(ID==NOTIFY_RESULT_PIC)
	{
		LPBYTE pBuffer=(LPBYTE)pParam;		
		strPath+=L"\\ResultPicture.bmp";
		HVideoHeader mImg;
		mImg.Vheight=mImg.Vwidth=wParam;
		mImg.VwBit=fParam;
		mImg.Vbuffer=pBuffer;
		mImg.Vsize=wParam*wParam*fParam/8;
		mReBuildProcess->SavePic(&mImg,strPath,0);
		//HSaveBmp(pBuffer,wParam,wParam,fParam,strPath);
	
		CString strSystemDir;
		::GetSystemDirectory(strSystemDir.GetBuffer(256), 256);
		strSystemDir.ReleaseBuffer();
		CString strRundll;
		strRundll = strSystemDir + L"\\rundll32.exe ";
		CString strParm;
		strParm.Format(L"%s\\shimgvw.dll,ImageView_Fullscreen %s", strSystemDir, strPath);
		ShellExecute(NULL,L"Open",strRundll,strParm,NULL,SW_SHOWNORMAL);
	}
	else if (ID==NOTIFY_RESULT_3D)
	{
		double mTime=GetTickCount() - dwGTCBegin;
		mTime=mTime/1000;
		CString str;
		str.Format(L"%.3fs",mTime);
		SetDlgItemText(IDC_EDITTime,str);			
		//SetDlgItemInt(IDC_EDITTime,( GetTickCount() - dwGTCBegin));	
	}
	return 0;
}

void CMutiFocusDlg::OnBnClickedViewimgbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (imgFiles.size()<=0)
		return;
	CString strSystemDir;
	::GetSystemDirectory(strSystemDir.GetBuffer(256), 256);
	strSystemDir.ReleaseBuffer();
	CString strRundll;
	strRundll = strSystemDir + L"\\rundll32.exe ";
	CString strParm=imgFiles[0];
	strParm.Format(L"%s\\shimgvw.dll,ImageView_Fullscreen %s", strSystemDir, strParm);
	ShellExecute(NULL,L"Open",strRundll,strParm,NULL,SW_SHOWNORMAL);
}

CString CMutiFocusDlg::GetModulePath()
{
	HMODULE t_mudule=GetModuleHandle(0);
	CString pfileName;
	GetModuleFileName(t_mudule,pfileName.GetBufferSetLength(MAX_PATH),MAX_PATH);
	pfileName.ReleaseBuffer();

	int nPos=pfileName.ReverseFind('\\');
	if(nPos<0)
		return L"";
	else
		return pfileName.Left(nPos);
}

void CMutiFocusDlg::onMouse(int event, int x, int y)
{
	if (pheader.Vbuffer)
	{
		delete(pheader.Vbuffer);
		pheader.Vbuffer = 0;
	}
	if (mEasyFunction)
	{
		if (mEasyFunction->OpenFile((CString)imgFiles[0], &pheader, 0))
		{
			FindEdgyInParas findParas;
			findParas.ptSearch.x = x;
			findParas.ptSearch.y = y;
			FindEdgyOutParas findRes;
			findRes.iCnt = 0;
			CString strPt;
			strPt.Format(L"坐标位置：%d,%d", x, y);
			AfxMessageBox(strPt);
		}
	}
}

void CMutiFocusDlg::on_mouse(int event, int x, int y, int flags, void* ustc)
{
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		CMutiFocusDlg* temp = reinterpret_cast<CMutiFocusDlg*>(ustc);
		temp->onMouse(event, x, y);
	}
}

void CMutiFocusDlg::OnBnClickedBtnfind()
{
	// TODO: 在此添加控件通知处理程序代码
	imgFiles.clear();
	imgFiles = DoSelectFiles(_T("*.bmp"),
		OFN_ENABLESIZING | OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY,
		_T("image files (*.bmp; *.png; *.jpg) |*.bmp; *.png; *.jpg; *.jpeg| All Files (*.*) |*.*||"),
		_T("SelectImg"), _T("Imgs\\Left"));

	CString str = (CString)imgFiles[0];
	std::string tempName = (LPCSTR)CStringA(str);
	const char *tmp = tempName.c_str();
	Mat mImg = cvLoadImage(tmp, 0);

	str = L"111111111";
	std::string ShowName = (LPCSTR)CStringA(str);
	cv::namedWindow(ShowName, 0);
	cv::imshow(ShowName, mImg);
	setMouseCallback(ShowName, on_mouse, this);
}
