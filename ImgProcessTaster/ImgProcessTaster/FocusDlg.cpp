// FocusDlg.cpp : 实现文件
//
#include "stdafx.h"
#include "ImgProcessTaster.h"
#include "FocusDlg.h"
#include "afxdialogex.h"
#include "HGloableFunction.h"

// CFocusDlg 对话框
IMPLEMENT_DYNAMIC(CFocusDlg, CDialogEx)

CFocusDlg::CFocusDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFocusDlg::IDD, pParent)
	, iCuScale(20)
	, iXiScale(8)
{
	pheader.Vbuffer=0;
	pheader.VwBit=pheader.Vwidth=pheader.Vheight=0;
	mEasyFunction=0;
}

CFocusDlg::~CFocusDlg()
{
}

void CFocusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, GayValue);
	DDX_Control(pDX, IDC_LIST2, GridentValue);
	DDX_Text(pDX, IDC_EDIT1, iCuScale);
	DDX_Text(pDX, IDC_EDIT2, iXiScale);
}


BEGIN_MESSAGE_MAP(CFocusDlg, CDialogEx)
	ON_BN_CLICKED(IDC_OpenImgBtn, &CFocusDlg::OnBnClickedOpenimgbtn)
	ON_BN_CLICKED(IDC_CaptureImgBtn, &CFocusDlg::OnBnClickedCaptureimgbtn)
	ON_BN_CLICKED(IDC_ClearBtn, &CFocusDlg::OnBnClickedClearbtn)
	ON_LBN_DBLCLK(IDC_LIST1, &CFocusDlg::OnLbnDblclkList1)
	ON_LBN_DBLCLK(IDC_LIST2, &CFocusDlg::OnLbnDblclkList2)
	ON_BN_CLICKED(IDC_ViewImgBtn, &CFocusDlg::OnBnClickedViewimgbtn)
END_MESSAGE_MAP()


// CFocusDlg 消息处理程序


void CFocusDlg::OnBnClickedOpenimgbtn()
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
	UpdateData();
	mFocusParas.mCuScale=iCuScale;
	mFocusParas.mXiScale=iXiScale;
	mFocusParas.bDebug=((CButton*)GetDlgItem(IDC_CHECKShow))->GetCheck();
	mAutoProcess->InitProcess(&mFocusParas);

	CString str=(CString)imgFiles[0];
	SetDlgItemText(IDC_EDITPath,str);
	str.Format(L"%d/%d",pheader.VcaptureTime,pheader.VnCount);
	SetDlgItemText(IDC_EDITCount,str);
	//清空
	GayValue.ResetContent();
	GridentValue.ResetContent();
}

bool CFocusDlg::SetProcess(HCoreProcess* mProcess)
{
	mAutoProcess=mProcess;
	return true;
}

bool CFocusDlg::SetFunction(HEasyFunction* mF)
{
	mEasyFunction=mF;
	return true;
}

vector<CStringA> CFocusDlg::DoSelectFiles(LPCTSTR lpszDefExt,DWORD dwFlags,LPCTSTR lpszFilter,
	LPCWSTR	lpstrTitle,LPCWSTR	lpstrInitialDir)
{
	vector<CStringA> selectedFiles;
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
			CStringA path;
			path = dlg.GetNextPathName(filePosition);
			selectedFiles.push_back( path );  
		}  
	}
	delete []pc;
	return selectedFiles;
}


void CFocusDlg::OnBnClickedCaptureimgbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	long width,height;
	int wbit;
	Quality_TYPE mfCuQua=ImgQuality_CuQuality;
	Quality_TYPE mfXiQua=ImgQuality_XiQuality;
	/*mFocusParas.mCuScale=iCuScale;
	mFocusParas.mXiScale=iXiScale;
	mFocusParas.bDebug=((CButton*)GetDlgItem(IDC_CHECKShow))->GetCheck();*/
	if (pheader.VcaptureTime<pheader.VnCount)
	{
		LPBYTE buffer=0;
		CString str=(CString)imgFiles[pheader.VcaptureTime];
		double mCuV=0.0,mXiV=0.0;
		DWORD  dwGTCBegin=0.0f;  
		if (mAutoProcess)
		{
			//std::string tempName=(LPCSTR)CStringA(str);
			//const char *tmp=tempName.c_str();
			//Mat mImg = cvLoadImage(tmp,0);
			//if (pheader.Vbuffer==0||pheader.Vwidth!=mImg.cols||pheader.Vheight!=mImg.rows
			//	||pheader.VwBit!=mImg.channels()*8)
			//{
			//	pheader.VwBit=mImg.channels()*8;
			//	pheader.Vwidth=mImg.cols;
			//	pheader.Vheight=mImg.rows;
			//	pheader.Vsize=pheader.Vheight*pheader.Vwidth*pheader.VwBit/8;
			//	if (pheader.Vbuffer)
			//		delete(pheader.Vbuffer);
			//	pheader.Vbuffer=(LPBYTE)malloc(pheader.Vsize);
			//}
			//memcpy(pheader.Vbuffer,mImg.data,pheader.Vsize);
			if (pheader.Vbuffer)
			{
				delete(pheader.Vbuffer);
				pheader.Vbuffer=0;
			}
				
			if (mEasyFunction)
			{
				if (mEasyFunction->OpenFile(str,&pheader,0))
				{
					dwGTCBegin = GetTickCount();
					mAutoProcess->ProcessImg(&mfCuQua,&mCuV,&pheader);
					mAutoProcess->ProcessImg(&mfXiQua,&mXiV,&pheader);
				}
			}

		}
		SetDlgItemInt(IDC_EDITTime,( GetTickCount() - dwGTCBegin));
		int nPos=str.ReverseFind('\\');
		CString strJpg=str.Right(str.GetLength()-nPos-1);

		//str.Format(L"FX%04d->%.5f",pheader.VcaptureTime+1,mXiV);
		str.Format(L"%s->%.5f",strJpg,mXiV);
		GridentValue.AddString(str);
		//str.Format(L"FX%04d->%.5f",pheader.VcaptureTime+1,mCuV);
		str.Format(L"%s->%.5f",strJpg,mCuV);
		GayValue.AddString(str);

		SetDlgItemText(IDC_EDITPath,(CString)imgFiles[pheader.VcaptureTime]);
	}
	else
	{
		AfxMessageBox(L"图片已经完全读取完毕");
		return;
	}
	pheader.VcaptureTime++;
	CString str;
	str.Format(L"%d/%d",pheader.VcaptureTime,pheader.VnCount);
	SetDlgItemText(IDC_EDITCount,str);
}


void CFocusDlg::OnBnClickedClearbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	pheader.VcaptureTime=0;
	CString str;
	str.Format(L"%d/%d",pheader.VcaptureTime,pheader.VnCount);
	SetDlgItemText(IDC_EDITCount,str);
	GayValue.ResetContent();
	GridentValue.ResetContent();

	mFocusParas.mCuScale=iCuScale;
	mFocusParas.mXiScale=iXiScale;
	mFocusParas.bDebug=((CButton*)GetDlgItem(IDC_CHECKShow))->GetCheck();
	mAutoProcess->InitProcess(&mFocusParas);
}


void CFocusDlg::OnLbnDblclkList1()
{
	// TODO: 在此添加控件通知处理程序代码
	//GayValue.ResetContent();
	Quality_TYPE mfCuQua=ImgQuality_CuEnd;
	double mCuIdx=0.0;
	mAutoProcess->ProcessImg(&mfCuQua,&mCuIdx,NULL);
	CString str;
	str.Format(L"Peak:%.3f",mCuIdx);
	AfxMessageBox(str);
}


void CFocusDlg::OnLbnDblclkList2()
{
	// TODO: 在此添加控件通知处理程序代码
	//GridentValue.ResetContent();
	Quality_TYPE mfXiQua=ImgQuality_XiEnd;
	double mXiIdx=0.0;
	mAutoProcess->ProcessImg(&mfXiQua,&mXiIdx,NULL);
	CString str;
	str.Format(L"Peak:%.3f",mXiIdx);
	AfxMessageBox(str);
}


void CFocusDlg::OnBnClickedViewimgbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strSystemDir;
	::GetSystemDirectory(strSystemDir.GetBuffer(256), 256);
	strSystemDir.ReleaseBuffer();
	CString strRundll;
	strRundll = strSystemDir + L"\\rundll32.exe ";
	CString strParm;
	CString strPicFile;//=”F:\\photo”;
	GetDlgItemText(IDC_EDITPath,strPicFile);
	strParm.Format(L"%s\\shimgvw.dll,ImageView_Fullscreen %s", strSystemDir, strPicFile);
	ShellExecute(NULL,L"Open",strRundll,strParm,NULL,SW_SHOWNORMAL);
}


BOOL CFocusDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	((CButton*)GetDlgItem(IDC_CHECKShow))->SetCheck(true);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

