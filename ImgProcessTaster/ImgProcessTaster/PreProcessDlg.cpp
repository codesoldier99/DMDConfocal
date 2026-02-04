// PreProcessDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ImgProcessTaster.h"
#include "PreProcessDlg.h"
#include "afxdialogex.h"



// CPreProcessDlg 对话框

IMPLEMENT_DYNAMIC(CPreProcessDlg, CDialogEx)

CPreProcessDlg::CPreProcessDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPreProcessDlg::IDD, pParent)
	, m_strParas0_0(_T("Average"))
	, iScale(2)
	, m_Gama(1)
	, iTh(150)
	, mMinArea(-1)
	, mMaxArea(-1)
	, m_X(0)
	, m_Y(0)
	, m_Width(100)
	, m_Height(100)
	, iXStart(0)
	, iXEnd(0)
	, iYStart(0)
	, iYEnd(0)
	, m_iTh(0)
	, iAddPicCount(100)
	, iCVUseImgs(1)
{
	for (int i=0;i<MAX_PROCESS;i++)
	{
		mPreProcess[i]=0;
		mImgProcess[i]=0;
	}
	pOriginHeader.Vbuffer=0;
	pEndHeader.Vbuffer=0;
	iUseImg=0;
}

CPreProcessDlg::~CPreProcessDlg()
{
}

void CPreProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strParas0_0);
	DDX_Text(pDX, IDC_EDITSCale, iScale);
	DDX_Text(pDX, IDC_EDITSCale2, m_Gama);
	DDX_Text(pDX, IDC_EDITSCale3, iTh);
	DDX_Text(pDX, IDC_EDITMinArea, mMinArea);
	DDX_Text(pDX, IDC_EDITMaxArea, mMaxArea);
	DDX_Text(pDX, IDC_EDITX, m_X);
	DDX_Text(pDX, IDC_EDITY, m_Y);
	DDX_Text(pDX, IDC_EDITWidth, m_Width);
	DDX_Text(pDX, IDC_EDITHeight, m_Height);
	DDX_Text(pDX, IDC_EDITX2, iXStart);
	DDX_Text(pDX, IDC_EDITY2, iXEnd);
	DDX_Text(pDX, IDC_EDITWidth2, iYStart);
	DDX_Text(pDX, IDC_EDITHeight2, iYEnd);
	DDX_Text(pDX, IDC_ThValue, m_iTh);
	DDV_MinMaxInt(pDX, m_iTh, 0, 255);
	DDX_Text(pDX, IDC_EDIT2, iAddPicCount);
	DDX_Text(pDX, IDC_ThValue2, iCVUseImgs);
}


BEGIN_MESSAGE_MAP(CPreProcessDlg, CDialogEx)
	ON_BN_CLICKED(IDC_GrayBtn, &CPreProcessDlg::OnBnClickedGraybtn)
	ON_BN_CLICKED(IDC_OpenImgBtn, &CPreProcessDlg::OnBnClickedOpenimgbtn)
	ON_BN_CLICKED(IDC_ScaleBtn, &CPreProcessDlg::OnBnClickedScalebtn)
	ON_BN_CLICKED(IDC_GamaBtn, &CPreProcessDlg::OnBnClickedGamabtn)
	ON_BN_CLICKED(IDC_BlockScanBtn, &CPreProcessDlg::OnBnClickedBlockscanbtn)
	ON_BN_CLICKED(IDC_CutBtn, &CPreProcessDlg::OnBnClickedCutbtn)
	ON_BN_CLICKED(IDC_CalCVBtn2, &CPreProcessDlg::OnBnClickedCalcvbtn2)
	ON_BN_CLICKED(IDC_CalLightBtn, &CPreProcessDlg::OnBnClickedCallightbtn)
	ON_BN_CLICKED(IDC_SetAveBtn, &CPreProcessDlg::OnBnClickedSetavebtn)
	ON_BN_CLICKED(IDC_ReadMaskBtn, &CPreProcessDlg::OnBnClickedReadmaskbtn)
	ON_BN_CLICKED(IDC_AddPicCountBtn, &CPreProcessDlg::OnBnClickedAddpiccountbtn)
	ON_BN_CLICKED(IDC_ReleaseVectorBtn, &CPreProcessDlg::OnBnClickedReleasevectorbtn)
	ON_BN_CLICKED(IDC_ResetVectorBtn, &CPreProcessDlg::OnBnClickedResetvectorbtn)
	ON_BN_CLICKED(IDC_AWBBtn, &CPreProcessDlg::OnBnClickedAwbbtn)
	ON_BN_CLICKED(IDC_BTNFILTER, &CPreProcessDlg::OnBnClickedBtnfilter)
END_MESSAGE_MAP()


// CPreProcessDlg 消息处理程序


bool CPreProcessDlg::SetPreProcess(HImgPreProcess* mProcess,CString strName)
{
	if (strName==L"GrayConvert")
	{
		mPreProcess[0]=mProcess;
		mPreProcess[0]->SetDebug(true);
		return true;
	}
	else if (strName==L"ScaleConvert")
	{
		mPreProcess[1]=mProcess;
		mPreProcess[1]->SetDebug(true);
		return true;
	}
	else if (strName==L"GamaConvert")
	{
		mPreProcess[2]=mProcess;
		mPreProcess[2]->SetDebug(true);
		return true;
	}
	else if (strName==L"CutConvert")
	{
		mPreProcess[3]=mProcess;
		mPreProcess[3]->SetDebug(true);
		return true;
	}
	else if (strName == L"FilterConvert")
	{
		mPreProcess[4] = mProcess;
		mPreProcess[4]->SetDebug(true);
		return true;
	}
	return false;
}

bool CPreProcessDlg::SetProcess(HImgProcess* mProcess,CString strName)
{
	if (strName==L"BlockScan")
	{
		mImgProcess[0]=mProcess;
		mImgProcess[0]->SetDebug(true);
		return true;
	}
	else if (strName==L"AutoWB")
	{
		mImgProcess[1]=mProcess;
		mImgProcess[1]->SetDebug(true);
		return true;
	}
	return false;
}

bool CPreProcessDlg::SetCalProcess(HCoreProcess* mProcess)
{
	mCalProcess=mProcess;
	return true;
}

bool CPreProcessDlg::OpenFile(int iColor)
{
	CFileDialog dlg(true);
	dlg.m_ofn.lpstrTitle=_T("图片打开对话框");
	dlg.m_ofn.lpstrFilter=_T("All Files(*.*)\0*.*\0彩色图(*.jpg)\0*.jpg\0All Files(*.*)\0*.*\0\0");
	if (IDOK==dlg.DoModal())
	{
		SetDlgItemText(IDC_EDITPath,dlg.GetPathName());
		std::string tempName=(LPCSTR)CStringA(dlg.GetPathName());
		const char *tmp=tempName.c_str();
		Mat mImg = cvLoadImage(tmp,iColor);
		if (pOriginHeader.Vbuffer==0||pOriginHeader.Vwidth!=mImg.cols||pOriginHeader.Vheight!=mImg.rows
			||pOriginHeader.VwBit!=mImg.channels()*8)
		{		
			pOriginHeader.VwBit=mImg.channels()*8;
			pOriginHeader.Vwidth=mImg.cols;
			pOriginHeader.Vheight=mImg.rows;
			pOriginHeader.Vsize=mImg.step*pOriginHeader.Vheight;
			if (pOriginHeader.Vbuffer)
				delete(pOriginHeader.Vbuffer);
			pOriginHeader.Vbuffer=(LPBYTE)malloc(pOriginHeader.Vsize);			
		}
		memcpy(pOriginHeader.Vbuffer,mImg.data,pOriginHeader.Vsize);
		//((CButton*)GetDlgItem(IDC_CHECKStepOn))->SetCheck(true);
		return true;
	}
	return false;
}

void CPreProcessDlg::OnBnClickedOpenimgbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (OpenFile(-1))
	{
	}
}

void CPreProcessDlg::OnBnClickedGraybtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (mPreProcess[0])
	{
		bool bStepOn=((CButton*)GetDlgItem(IDC_CHECKStepOn))->GetCheck();
		if (!pOriginHeader.Vbuffer)
			if (!OpenFile(1))
				return;

		mProcessParas.strParas[0]=m_strParas0_0;
		mPreProcess[0]->InitProcess(&mProcessParas);
		DWORD  dwGTCBegin=0.0f;  
		dwGTCBegin = GetTickCount();
		bool bReturn=false;
		if (bStepOn)
		{
			bReturn=mPreProcess[0]->PreProcessImg(&pOriginHeader,&pOriginHeader);
			//if (pEndHeader.Vbuffer)
			//	bReturn=mPreProcess[0]->PreProcessImg(&pEndHeader,&pEndHeader);
			//else
			//	bReturn=mPreProcess[0]->PreProcessImg(&pOriginHeader,&pEndHeader);
		}
		else
			bReturn=mPreProcess[0]->PreProcessImg(&pOriginHeader,&pEndHeader);
		SetDlgItemInt(IDC_EDITTime,( GetTickCount() - dwGTCBegin));
		if (!bReturn)
			SetDlgItemText(IDC_EDITTime,L"Gray Failed!");
	}
	else
		SetDlgItemText(IDC_EDITTime,L"Gray Empty!");
}

void CPreProcessDlg::OnBnClickedScalebtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (mPreProcess[1])
	{
		bool bStepOn=((CButton*)GetDlgItem(IDC_CHECKStepOn))->GetCheck();
		if (!pOriginHeader.Vbuffer)
			if (!OpenFile(1))
				return;

		mProcessParas.iParas[0]=iScale;
		mPreProcess[1]->InitProcess(&mProcessParas);
		DWORD  dwGTCBegin=0.0f;  
		dwGTCBegin = GetTickCount();
		bool bReturn=false;
		if (bStepOn)
		{
			bReturn=mPreProcess[1]->PreProcessImg(&pOriginHeader,&pOriginHeader);
			//if (pEndHeader.Vbuffer)
			//	bReturn=mPreProcess[1]->PreProcessImg(&pEndHeader,&pEndHeader);
			//else
			//	bReturn=mPreProcess[1]->PreProcessImg(&pOriginHeader,&pEndHeader);
		}
		else
			bReturn=mPreProcess[1]->PreProcessImg(&pOriginHeader,&pEndHeader);

		SetDlgItemInt(IDC_EDITTime,( GetTickCount() - dwGTCBegin));
		if (!bReturn)
			SetDlgItemText(IDC_EDITTime,L"Scale Failed!");
	}
	else
		SetDlgItemText(IDC_EDITTime,L"Scale Empty!");
}

void CPreProcessDlg::OnBnClickedGamabtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (mPreProcess[2])
	{
		bool bStepOn=((CButton*)GetDlgItem(IDC_CHECKStepOn))->GetCheck();
		if (!pOriginHeader.Vbuffer)
			if (!OpenFile(1))
				return;

		mProcessParas.fParas[0]=m_Gama;
		mPreProcess[2]->InitProcess(&mProcessParas);
		DWORD  dwGTCBegin=0.0f;  
		dwGTCBegin = GetTickCount();
		bool bReturn=false;
		if (bStepOn)
		{
			bReturn=mPreProcess[2]->PreProcessImg(&pOriginHeader,&pOriginHeader);
			//if (pEndHeader.Vbuffer)
			//	bReturn=mPreProcess[2]->PreProcessImg(&pEndHeader,&pEndHeader);
			//else
			//	bReturn=mPreProcess[2]->PreProcessImg(&pOriginHeader,&pEndHeader);
		}
		else
			bReturn=mPreProcess[2]->PreProcessImg(&pOriginHeader,&pEndHeader);
		SetDlgItemInt(IDC_EDITTime,( GetTickCount() - dwGTCBegin));
		if (!bReturn)
			SetDlgItemText(IDC_EDITTime,L"Gama Failed!");
	}
	else
		SetDlgItemText(IDC_EDITTime,L"Gama Empty!");
}

void CPreProcessDlg::OnBnClickedCutbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (mPreProcess[3])
	{
		bool bStepOn=((CButton*)GetDlgItem(IDC_CHECKStepOn))->GetCheck();
		if (!pOriginHeader.Vbuffer)
			if (!OpenFile(1))
				return;
		mProcessParas.iParas[0]=m_X;
		mProcessParas.iParas[1]=m_Y;
		mProcessParas.iParas[2]=m_Width;
		mProcessParas.iParas[3]=m_Height;
		mPreProcess[3]->InitProcess(&mProcessParas);
		DWORD  dwGTCBegin=0.0f;  
		dwGTCBegin = GetTickCount();
		bool bReturn=false;
		if (bStepOn)
		{
			bReturn=mPreProcess[3]->PreProcessImg(&pOriginHeader,&pOriginHeader);
			//if (pEndHeader.Vbuffer)
			//	bReturn=mPreProcess[3]->PreProcessImg(&pEndHeader,&pEndHeader);
			//else
			//	bReturn=mPreProcess[3]->PreProcessImg(&pOriginHeader,&pEndHeader);
		}
		else
			bReturn=mPreProcess[3]->PreProcessImg(&pOriginHeader,&pEndHeader);
		SetDlgItemInt(IDC_EDITTime,( GetTickCount() - dwGTCBegin));
		if (!bReturn)
			SetDlgItemText(IDC_EDITTime,L"Cut Failed!");
	}
	else
		SetDlgItemText(IDC_EDITTime,L"Cut Empty!");
}

void CPreProcessDlg::OnBnClickedBlockscanbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	bool bInvert=((CButton*)GetDlgItem(IDC_CHECKBinary))->GetCheck();
	if (mImgProcess[0])
	{
		bool bStepOn=((CButton*)GetDlgItem(IDC_CHECKStepOn))->GetCheck();
		if (!pOriginHeader.Vbuffer)
			if (!OpenFile(1))
				return;
		mProcessParas.iParas[0]=iTh;
		mProcessParas.iParas[1]=mMaxArea;
		mProcessParas.iParas[2]=mMinArea;
		mProcessParas.bParas[0]=bInvert;
		mImgProcess[0]->InitProcess(&mProcessParas);
		DWORD  dwGTCBegin=0.0f;  
		dwGTCBegin = GetTickCount();		
		int iReturn=-1;//mImgProcess[0]->ProcessImg(&pOriginHeader,&pEndHeader,0);
		if (bStepOn)
		{
			iReturn=mImgProcess[0]->ProcessImg(&pOriginHeader,&pOriginHeader,0);
			//if (pEndHeader.Vbuffer)
			//	iReturn=mImgProcess[0]->ProcessImg(&pEndHeader,&pEndHeader,0);
			//else
			//	iReturn=mImgProcess[0]->ProcessImg(&pOriginHeader,&pEndHeader,0);
		}
		else
			iReturn=mImgProcess[0]->ProcessImg(&pOriginHeader,&pEndHeader,0);

		SetDlgItemInt(IDC_EDITTime,( GetTickCount() - dwGTCBegin));
		if (iReturn<=0)
			SetDlgItemText(IDC_EDITTime,L"Block Failed!");
		else 
		{
			CString strM;
			strM.Format(L"Return Value:%d",iReturn);
			AfxMessageBox(strM);
		}
	}
	else
		SetDlgItemText(IDC_EDITTime,L"Block Empty!");
}

void CPreProcessDlg::OnBnClickedAwbbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (OpenFile(1))
	{
		UpdateData();
		AWBParas mV;
		DWORD  dwGTCBegin=0.0f;  
		dwGTCBegin = GetTickCount();
		int iReturn=mImgProcess[1]->ProcessImg(&pOriginHeader,0,0,&mV);
		if (iReturn>0)
		{
			CString strV;
			strV.Format(L"RGain:%.3f\r\n GGain:%.3f\r\n BGain:%.3f\r\n",
				mV.dRGain,mV.dGGain,mV.dBGain);
			SetDlgItemText(IDC_GainTxt,strV);
		}
		SetDlgItemInt(IDC_EDITTime,( GetTickCount() - dwGTCBegin));
	}
}

///////////////////////////下面是用于测试vector<cv::Mat>的内存释放///////////////////////////////////////
void CPreProcessDlg::OnBnClickedAddpiccountbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	//SetDlgItemText(IDC_EDITPath,dlg.GetPathName());
	CString strFile=L"";
	GetDlgItemText(IDC_EDITPath,strFile);
	std::string tempName=(LPCSTR)CStringA(strFile);
	const char *tmp=tempName.c_str();
	bool bPush=((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
	vector<cv::Mat> mImgs2;
	if (pOriginHeader.Vbuffer)
	{
		int wBit=pOriginHeader.VwBit/8;
		UpdateData();
		for (int i=0;i<iAddPicCount;i++)
		{
			Mat mImg;
			if (wBit==3)
				mImg=cv::Mat(pOriginHeader.Vheight,pOriginHeader.Vwidth,CV_8UC3);	
			else if(wBit==1)
				mImg=cv::Mat(pOriginHeader.Vheight,pOriginHeader.Vwidth,CV_8U);
			memcpy(mImg.data,pOriginHeader.Vbuffer,pOriginHeader.Vsize);

			mImgs2.push_back(mImg);
			if (bPush)
				mImgs.push_back(mImg);
		}
	}
	AfxMessageBox(L"退出！");
}

void CPreProcessDlg::OnBnClickedReleasevectorbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (mImgs.size()<=0)
		return;

	cv::namedWindow("mImgs[size-1]");
	cv::imshow("mImgs[size-1]",mImgs[mImgs.size()-1]);

	AfxMessageBox(L"完成，开始clear()释放！");
	mImgs.clear();
	AfxMessageBox(L"完成，开始swap释放！");
	std::vector<cv::Mat>().swap(mImgs);
}

void CPreProcessDlg::OnBnClickedResetvectorbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	for (int i=0;i<mImgs.size();i++)
	{
		mImgs[i]=cv::Mat(pOriginHeader.Vheight,pOriginHeader.Vwidth,CV_8U);
	}
}
///////////////////////////上面是用于测试vector<cv::Mat>的内存释放///////////////////////////////////////

///////////////////////////下面是CV值的计算///////////////////////////////////////
void CPreProcessDlg::OnBnClickedCalcvbtn2()
{
	// TODO: 在此添加控件通知处理程序代码
	if (OpenFile(1))
	{
		UpdateData();
		Cal_TYPE mCul=Cal_CV;
		CalValueParas mV;
		mV.mROIRec.left=iXStart;mV.mROIRec.right=iXEnd;
		mV.mROIRec.top=iYStart;mV.mROIRec.bottom=iYEnd;
		mV.iTh=m_iTh;//去除低于这个阈值的部分做CV值分析

		DWORD  dwGTCBegin=0.0f;  
		dwGTCBegin = GetTickCount();

		bool bReturn=mCalProcess->ProcessImg(&mCul,&mV,&pOriginHeader);

		SetDlgItemInt(IDC_EDITTime,( GetTickCount() - dwGTCBegin));
		CString strV;
		strV.Format(L"Max:%.3f\r\nMin:%.3f\r\nMean:%.3f\r\nStd:%.3f\r\nCV:%.3f\r\n",
			mV.MaxValue,mV.MinValue,mV.MeanValue,mV.SDValue,mV.CVValue*100);
		SetDlgItemText(IDC_ValueTxt,strV);
	}
}

void CPreProcessDlg::OnBnClickedCallightbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (OpenFile(1))
	{
		Cal_TYPE mCul=Cal_LightAvg;

		mCVParas.iUseImg=iCVUseImgs;
		mCVParas.iBKErode=50;
		mCVParas.strBKImg=L".\\Gray.bmp";
		mCalProcess->InitProcess(&mCVParas);

		if (iUseImg>=mCVParas.iUseImg)
			iUseImg=0;

		DWORD  dwGTCBegin=0.0f;  
		dwGTCBegin = GetTickCount();

		bool bReturn=mCalProcess->ProcessImg(&mCul,&iUseImg,&pOriginHeader);

		SetDlgItemInt(IDC_EDITTime,( GetTickCount() - dwGTCBegin));
		iUseImg++;
	}
}

void CPreProcessDlg::OnBnClickedSetavebtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (OpenFile(1))
	{
		UpdateData();
		Cal_TYPE mCul=Set_LightAvg;
		CalValueParas mV;
		mV.mROIRec.left=iXStart;mV.mROIRec.right=iXEnd;
		mV.mROIRec.top=iYStart;mV.mROIRec.bottom=iYEnd;
		mV.iTh=m_iTh;
		DWORD  dwGTCBegin=0.0f;  
		dwGTCBegin = GetTickCount();
		bool bReturn=mCalProcess->ProcessImg(&mCul,&mV,&pOriginHeader);
		SetDlgItemInt(IDC_EDITTime,( GetTickCount() - dwGTCBegin));
	}
}

//没有使用库，直接读取均势图进行加权计算
void CPreProcessDlg::OnBnClickedReadmaskbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(true);
	dlg.m_ofn.lpstrTitle=_T("图片打开对话框");
	dlg.m_ofn.lpstrFilter=_T("All Files(*.*)\0*.*\0彩色图(*.jpg)\0*.jpg\0All Files(*.*)\0*.*\0\0");
	Mat mImg;
	if (IDOK==dlg.DoModal())
	{
		SetDlgItemText(IDC_EDITPath,dlg.GetPathName());
		std::string tempName=(LPCSTR)CStringA(dlg.GetPathName());
		const char *tmp=tempName.c_str();
		mImg = cvLoadImage(tmp,0);
	}
	else
		return;

	cv::namedWindow("PreImg");
	cv::imshow("PreImg",mImg);

	CString strRead;
	strRead.Format(L".\\EpiResult\\CVAvgValue%d.xml",mImg.cols);
	Mat mCVAvgImg=Mat(mImg.rows,mImg.cols,CV_32F);

	string strValHeader=(LPCSTR)CStringA(strRead);

	FileStorage fs(strValHeader, FileStorage::READ);
	Mat mat_vocabulary;
	fs["CVReBuildValue"] >> mCVAvgImg;

	//FILE * fp=fopen((const char*)strValHeader.c_str(),"rb");
	//if (!fp)
	//	return;
	//int iWSize=mImg.rows*mImg.cols*mImg.channels();//*sizeof(float);
	//fread(mCVAvgImg.data,sizeof(float),iWSize,fp);
	//fclose(fp);


	int height=mImg.rows,width=mImg.cols;
	for (int i=0;i<height;i++)
	{
		for (int j=0;j<width;j++)
		{
			float fV=mCVAvgImg.at<float>(i,j);//mAveValue/mImg.at<byte>(i,j);
			fV=fV*mImg.at<byte>(i,j);
			if (fV>255)
				mImg.at<byte>(i,j)=255;
			else 
				mImg.at<byte>(i,j)=fV;
		}
	}

	cv::namedWindow("After PreImg");
	cv::imshow("After PreImg",mImg);
}
///////////////////////////上面是CV值的计算///////////////////////////////////////

void CPreProcessDlg::OnBnClickedBtnfilter()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (mPreProcess[4])
	{
		bool bStepOn = ((CButton*)GetDlgItem(IDC_CHECKStepOn))->GetCheck();
		if (!pOriginHeader.Vbuffer)
			if (!OpenFile(1))
				return;

		mProcessParas.iParas[0] = iScale;
		mPreProcess[4]->InitProcess(&mProcessParas);
		DWORD  dwGTCBegin = 0.0f;
		dwGTCBegin = GetTickCount();
		bool bReturn = false;
		if (bStepOn)
			bReturn = mPreProcess[4]->PreProcessImg(&pOriginHeader, &pOriginHeader);
		else
			bReturn = mPreProcess[4]->PreProcessImg(&pOriginHeader, &pEndHeader);

		SetDlgItemInt(IDC_EDITTime, (GetTickCount() - dwGTCBegin));
		if (!bReturn)
			SetDlgItemText(IDC_EDITTime, L"Filter Failed!");
	}
	else
		SetDlgItemText(IDC_EDITTime, L"Filter Empty!");
}
