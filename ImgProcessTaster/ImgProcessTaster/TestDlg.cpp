// TestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ImgProcessTaster.h"
#include "TestDlg.h"
#include "afxdialogex.h"
#include "HGloableFunction.h"


// TestDlg 对话框

IMPLEMENT_DYNAMIC(TestDlg, CDialogEx)

TestDlg::TestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(TestDlg::IDD, pParent)
	, miSize(1100)
	, iCutSize(1100)
	, iTempleSize(900)
	, iCurRation(3)
{
	iCalCount=0;
}

TestDlg::~TestDlg()
{
}

void TestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, miSize);
	DDX_Text(pDX, IDC_EDITTime2, iCutSize);
	DDX_Text(pDX, IDC_EDITTime3, iTempleSize);
	DDX_Radio(pDX, IDC_RADIO1, iCurRation);
	DDX_Control(pDX, IDC_LIST2, mResultList);
}


BEGIN_MESSAGE_MAP(TestDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CombineBtn, &TestDlg::OnBnClickedCombinebtn)
	ON_BN_CLICKED(IDC_UseSize, &TestDlg::OnBnClickedUsesize)
	ON_BN_CLICKED(IDC_NewCombineBtn, &TestDlg::OnBnClickedNewcombinebtn)
	ON_BN_CLICKED(IDC_NewCombineBtn2, &TestDlg::OnBnClickedNewcombinebtn2)
	ON_LBN_DBLCLK(IDC_LIST2, &TestDlg::OnLbnDblclkList2)
	ON_BN_CLICKED(IDC_ReadImgBtn, &TestDlg::OnBnClickedReadimgbtn)
END_MESSAGE_MAP()


// TestDlg 消息处理程序


void TestDlg::OnBnClickedCombinebtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	bool bEnable=((CButton*)GetDlgItem(IDC_UseSize))->GetCheck();
	imgFiles.clear();
	imgFiles = DoSelectFiles(_T("*.bmp"), 
		OFN_ENABLESIZING   |OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY,
		_T("image files (*.bmp; *.png; *.jpg) |*.bmp; *.png; *.jpg; *.jpeg| All Files (*.*) |*.*||"),
		_T("SelectImg"),_T("Imgs\\Left"));

	vector<Mat> imgs;
	for (int i=0;i<imgFiles.size();i++)
	{
		CString str=(CString)imgFiles[i];
		std::string tempName=(LPCSTR)CStringA(str);
		const char *tmp=tempName.c_str();
		Mat mImg = cvLoadImage(tmp);
		if (bEnable)
		{
			if (miSize<mImg.cols&&miSize<mImg.rows)
			{
				int iY=mImg.rows/2-miSize/2;
				int iX=mImg.cols/2-miSize/2;
				Mat frame=mImg(Rect(iX,iY,miSize,miSize));  //测试  合成图/8-7以及8-8
				imgs.push_back(frame);

				str.Format(L"Img%d",i);
				std::string ShowName=(LPCSTR)CStringA(str);
				cv::namedWindow(ShowName,0);
				cv::imshow(ShowName,frame);
			}
			else
			{
				AfxMessageBox(L"图像过小！");
				return;
			}
		}
		else
			imgs.push_back(mImg);		
	}

	Mat pano;
	bool bR=((CButton*)GetDlgItem(IDC_TryUseGpu))->GetCheck();
	Stitcher stitcher = Stitcher::createDefault(bR);

	SetStitcher(&stitcher);

	DWORD  dwGTCBegin=0.0f;  
	dwGTCBegin = GetTickCount();

	Stitcher::Status status = stitcher.stitch(imgs, pano);


	SetDlgItemInt(IDC_EDITTime,( GetTickCount() - dwGTCBegin));
	
	if (status == Stitcher::OK)
	{
		cv::namedWindow("ProcessImg",0);
		cv::imshow("ProcessImg",pano);
	}
}

vector<CStringA> TestDlg::DoSelectFiles(LPCTSTR lpszDefExt,DWORD dwFlags,LPCTSTR lpszFilter,
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


void TestDlg::SetStitcher(void* mP)
{
	Stitcher stitcher =  *(Stitcher*)mP;//Stitcher::createDefault(try_use_gpu);
	stitcher.setRegistrationResol(0.1);//为了加速，我选0.1,默认是0.6,最大值1最慢，此方法用于特征点检测阶段，如果找不到特征点，调高吧
	//stitcher.setSeamEstimationResol(0.1);//默认是0.1
	//stitcher.setCompositingResol(-1);//默认是-1，用于特征点检测阶段，找不到特征点的话，改-1
	stitcher.setPanoConfidenceThresh(1);//默认是1,见过有设0.6和0.4的
	stitcher.setWaveCorrection(false);//默认是true，为加速选false，表示跳过WaveCorrection步骤
	//stitcher.setWaveCorrectKind(detail::WAVE_CORRECT_HORIZ);//还可以选detail::WAVE_CORRECT_VERT ,波段修正(wave correction)功能（水平方向/垂直方向修正）。因为setWaveCorrection设的false，此语句没用

	//找特征点surf算法，此算法计算量大,但对刚体运动、缩放、环境影响等情况下较为稳定
	detail::SurfFeaturesFinder *featureFinder = new detail::SurfFeaturesFinder();
	stitcher.setFeaturesFinder(featureFinder);

	//找特征点ORB算法,但是发现草地这组图，这个算法不能完成拼接
	//detail::OrbFeaturesFinder *featureFinder = new detail::OrbFeaturesFinder();
	//stitcher.setFeaturesFinder(featureFinder);

	//Features matcher which finds two best matches for each feature and leaves the best one only if the ratio between descriptor distances is greater than the threshold match_conf.
	detail::BestOf2NearestMatcher *matcher = new detail::BestOf2NearestMatcher(false, 0.5f/*=match_conf默认是0.65，我选0.8，选太大了就没特征点啦,0.8都失败了*/);
	stitcher.setFeaturesMatcher(matcher);

	// Rotation Estimation,It takes features of all images, pairwise matches between all images and estimates rotations of all cameras.
	//Implementation of the camera parameters refinement algorithm which minimizes sum of the distances between the rays passing through the camera center and a feature,这个耗时短
	stitcher.setBundleAdjuster(new detail::BundleAdjusterRay());
	//Implementation of the camera parameters refinement algorithm which minimizes sum of the reprojection error squares.
	//stitcher.setBundleAdjuster(new detail::BundleAdjusterReproj());

	//Seam Estimation
	//Minimum graph cut-based seam estimator
	//stitcher.setSeamFinder(new detail::GraphCutSeamFinder(detail::GraphCutSeamFinderBase::COST_COLOR));//默认就是这个
	//stitcher.setSeamFinder(new detail::GraphCutSeamFinder(detail::GraphCutSeamFinderBase::COST_COLOR_GRAD));//GraphCutSeamFinder的第二种形式
	//啥SeamFinder也不用，Stub seam estimator which does nothing.
	stitcher.setSeamFinder(new detail::NoSeamFinder);
	//Voronoi diagram-based seam estimator.
	//stitcher.setSeamFinder(new detail::VoronoiSeamFinder);

	//exposure compensators曝光补偿
	//stitcher.setExposureCompensator(new detail::BlocksGainCompensator());//默认的就是这个
	//不要曝光补偿
	stitcher.setExposureCompensator(new detail::NoExposureCompensator());
	//Exposure compensator which tries to remove exposure related artifacts by adjusting image intensities
	//stitcher.setExposureCompensator(new detail::detail::GainCompensator());
	//Exposure compensator which tries to remove exposure related artifacts by adjusting image block intensities  
	//stitcher.setExposureCompensator(new detail::detail::BlocksGainCompensator()); 

	//Image Blenders
	//Blender which uses multi-band blending algorithm 
	//stitcher.setBlender(new detail::MultiBandBlender(try_use_gpu));//默认的是这个
	//Simple blender which mixes images at its borders
	stitcher.setBlender(new detail::FeatherBlender());//这个简单，耗时少

	//柱面？球面OR平面？默认为球面
	PlaneWarper*  cw = new PlaneWarper();
	//SphericalWarper*  cw = new SphericalWarper();
	//CylindricalWarper*  cw = new CylindricalWarper();
	stitcher.setWarper(cw);

	//Stitcher::Status status = stitcher.estimateTransform(imgs);
	//if (status != Stitcher::OK)
	//{

	//}
	//status = stitcher.composePanorama(pano);
	//if (status != Stitcher::OK)
	//{

	//}

}

void TestDlg::OnBnClickedUsesize()
{
	// TODO: 在此添加控件通知处理程序代码
	bool bEnable=((CButton*)GetDlgItem(IDC_UseSize))->GetCheck();
	GetDlgItem(IDC_EDIT1)->EnableWindow(bEnable);
}


//src是小图，dst是大图。src是搜索的模板图，dst是要被查找的大图
//位置是，src这边的方向，dst相对与src的方向，  先打开的图像是src，后打开的是dst
void TestDlg::OnBnClickedNewcombinebtn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	_dr=(Dir)iCurRation;
	_ad.m_rows=_ad.m_cols=iCutSize;
	_ad.TempleSize=iTempleSize;//模板大小
	_ad.m_overlap=_ad.m_rows-_ad.TempleSize;//200;

	imgFiles.clear();
	imgFiles = DoSelectFiles(_T("*.bmp"), 
		OFN_ENABLESIZING   |OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY,
		_T("image files (*.bmp; *.png; *.jpg) |*.bmp; *.png; *.jpg; *.jpeg| All Files (*.*) |*.*||"),
		_T("SelectImg"),_T("Imgs\\Left"));

	vector<Mat> imgs;
	vector<Mat> iComBineMgs;

	if (imgFiles.size()<2)
		return;
	for (int i=0;i<imgFiles.size();i++)
	{
		CString str=(CString)imgFiles[i];
		std::string tempName=(LPCSTR)CStringA(str);
		const char *tmp=tempName.c_str();
		Mat mImg = cvLoadImage(tmp,0);

		if (_ad.m_cols<mImg.cols&&_ad.m_rows<mImg.rows)
		{
			int iY=mImg.rows/2-_ad.m_rows/2;
			int iX=mImg.cols/2-_ad.m_cols/2;
			iComBineMgs.push_back(mImg);
			Mat frame=mImg(Rect(iX,iY,_ad.m_cols,_ad.m_rows));  //测试  合成图/8-7以及8-8
			Mat frame2=cv::Mat(_ad.m_rows,_ad.m_cols,CV_8U);
			frame.copyTo(frame2);
			imgs.push_back(frame2);
		}
		else
		{
			AfxMessageBox(L"图像过小！");
			return;
		}
	}

	for (int i=0;i<imgs.size()-1;i++)
	{
		DWORD  dwGTCBegin=0.0f;  
		dwGTCBegin = GetTickCount();

		Rect mSrcRect=Rect(0,0,_ad.m_overlap/4,_ad.m_overlap/4);
		Rect mDstRect=Rect(0,0,_ad.m_overlap,_ad.m_overlap);
		CalSearchRect(mSrcRect,mDstRect);
		Mat mSrcframe=imgs[i](mSrcRect);//cv::Mat(mSrcRect.height,mSrcRect.width,CV_8U);
		Mat mDstframe=imgs[i+1](mDstRect);//cv::Mat(mDstRect.height,mDstRect.width,CV_8U);
		cv::Mat mMatchValue=cv::Mat(mDstframe.rows,mDstframe.cols, CV_32F);
		matchTemplate(mDstframe,mSrcframe,mMatchValue,CV_TM_SQDIFF);
		double dMax=0,dMin=0;
		cv::Point minPt=Point(0,0),maxPt=Point(0,0);
		cv::minMaxLoc(mMatchValue,&dMin,&dMax,&minPt,&maxPt);

		int shiftX=0,shiftY=0;
		Mat mSrcframe2=Mat(mSrcframe.rows,mSrcframe.cols,mSrcframe.type());
		mSrcframe.copyTo(mSrcframe2);
		Mat mDstframe2=Mat(mDstframe.rows,mDstframe.cols,mDstframe.type());
		mDstframe.copyTo(mDstframe2);

		//cv::flip(mSrcframe2,mSrcframe2,0);
		//cv::flip(mDstframe2,mDstframe2,0);
		//LPBYTE mSrcframeData=(LPBYTE)malloc(mSrcRect.width*mSrcRect.height*mSrcframe2.channels());
		//memcpy(mSrcframeData,mSrcframe2.data,mSrcRect.width*mSrcRect.height*mSrcframe2.channels());
		//LPBYTE mDstframeData=(LPBYTE)malloc(mDstRect.width*mDstRect.height*mDstframe2.channels());
		//memcpy(mDstframeData,mDstframe2.data,mDstRect.width*mDstRect.height*mDstframe2.channels());
		//HSaveBmp(mSrcframeData,mSrcRect.height,mSrcRect.height,8,L".\\Test\\1old.bmp");
		//HSaveBmp(mDstframeData,mDstRect.height,mDstRect.height,8,L".\\Test\\2old.bmp");
		//double mV=Match_8bits(mSrcframeData,mSrcRect.width,mSrcRect.height,
		//	mDstframeData,mDstRect.width,mDstRect.height,shiftX,shiftY,1);
		//delete(mSrcframeData);delete(mDstframeData);
		double mV=Match_8bits(mSrcframe2.data,mSrcRect.width,mSrcRect.height,
			mDstframe2.data,mDstRect.width,mDstRect.height,shiftX,shiftY,1);
		shiftX=shiftX-_ad.m_overlap/4;
		shiftY=shiftY-_ad.m_overlap/4;
		//cv::flip(mSrcframe2,mSrcframe2,0);
		//cv::flip(mDstframe2,mDstframe2,0);
		//cv::rectangle(mDstframe2,Rect(shiftX,shiftY,mSrcRect.width,mSrcRect.height),Scalar(255,0,0));
		//cv::namedWindow("SrcImg_Match_8bits");
		//cv::imshow("SrcImg_Match_8bits",mSrcframe2);
		//cv::namedWindow("DstImg_Match_8bits");
		//cv::imshow("DstImg_Match_8bits",mDstframe2);

		//cv::rectangle(mDstframe,Rect(minPt.x,minPt.y,mSrcRect.width,mSrcRect.height),Scalar(0,0,255));
		//cv::namedWindow("SrcImg");
		//cv::imshow("SrcImg",mSrcframe);
		//cv::namedWindow("DstImg");
		//cv::imshow("DstImg",mDstframe);
		//cv::rectangle(imgs[0],mSrcRect,Scalar(0,0,255));
		//cv::rectangle(imgs[1],mDstRect,Scalar(0,0,255));
		//cv::namedWindow("Img0");
		//cv::imshow("Img0",imgs[0]);
		//cv::namedWindow("Img1");
		//cv::imshow("Img1",imgs[1]);

		int mSiftX=minPt.x-_ad.m_overlap/4;
		int mSiftY=minPt.y-_ad.m_overlap/4;
		float iMax=(_ad.m_overlap/4+0.0f)*0.8f;
		//if (mSiftX>=iMax||mSiftY>=iMax)
		//{
		//	AfxMessageBox(L"合成失败！");
		//}
		Mat mCombine;
		ComBineImg(iComBineMgs[i],iComBineMgs[i+1],mCombine,minPt);
		double mTimeUse=GetTickCount() - dwGTCBegin;
		//SetDlgItemInt(IDC_EDITTime,( GetTickCount() - dwGTCBegin));
		cv::namedWindow("FinalImg",1);
		cv::imshow("FinalImg",mCombine);
		CString strShow;
		strShow.Format(L"%03d-Shift：（%d，%d）；Match_8bits：（%d，%d）；MatchV:%.6f;Time:%.1f",iCalCount,mSiftX,mSiftY,shiftX,shiftY,mV,mTimeUse);
		mResultList.AddString(strShow);
		iCalCount++;
	}
}

void TestDlg::CutImg(Mat src,Mat& dst,CRect mV)
{
	Mat frame=src(Rect(mV.left,mV.top,mV.Width(),mV.Height()));
	frame.copyTo(dst);
}

void TestDlg::ComBineImg(Mat src,Mat dst,Mat& mCombine,Point mPt)
{
	Mat mframe1,mframe2;
	int mSiftX=_ad.m_overlap/4-mPt.x;
	int mSiftY=_ad.m_overlap/4-mPt.y;
	int iSrcX=src.cols/2-_ad.TempleSize/2;
	int iSrcY=src.rows/2-_ad.TempleSize/2;
	switch(_dr)
	{
		case left:			
			mCombine=Mat(_ad.m_rows,2*_ad.m_cols,src.type());
			mframe1=src(Rect(iSrcX,iSrcY,_ad.TempleSize,_ad.TempleSize));
			mframe2=mCombine(Rect(_ad.TempleSize,0,_ad.TempleSize,_ad.TempleSize));		
			mframe1.copyTo(mframe2);
			mframe1=dst(Rect(iSrcX-mSiftX,iSrcY-mSiftY,_ad.TempleSize,_ad.TempleSize));
			mframe2=mCombine(Rect(0,0,_ad.TempleSize,_ad.TempleSize));
			mframe1.copyTo(mframe2);
			break;
		case right:		
			mCombine=Mat(_ad.m_rows,2*_ad.m_cols,src.type());
			mframe1=src(Rect(iSrcX,iSrcY,_ad.TempleSize,_ad.TempleSize));
			mframe2=mCombine(Rect(0,0,_ad.TempleSize,_ad.TempleSize));
			mframe1.copyTo(mframe2);
			mframe1=dst(Rect(iSrcX-mSiftX,iSrcY-mSiftY,_ad.TempleSize,_ad.TempleSize));
			mframe2=mCombine(Rect(_ad.TempleSize,0,_ad.TempleSize,_ad.TempleSize));
			mframe1.copyTo(mframe2);
			break;
		case top:
			mCombine=Mat(2*_ad.m_rows,_ad.m_cols,src.type());
			mframe1=src(Rect(iSrcX,iSrcY,_ad.TempleSize,_ad.TempleSize));
			mframe2=mCombine(Rect(0,_ad.TempleSize,_ad.TempleSize,_ad.TempleSize));
			mframe1.copyTo(mframe2);
			mframe1=dst(Rect(iSrcX-mSiftX,iSrcY-mSiftY,_ad.TempleSize,_ad.TempleSize));		
			mframe2=mCombine(Rect(0,0,_ad.TempleSize,_ad.TempleSize));
			mframe1.copyTo(mframe2);

			break;
		case bottom:
			mCombine=Mat(2*_ad.m_rows,_ad.m_cols,src.type());
			mframe1=src(Rect(iSrcX,iSrcY,_ad.TempleSize,_ad.TempleSize));
			mframe2=mCombine(Rect(0,0,_ad.TempleSize,_ad.TempleSize));
			mframe1.copyTo(mframe2);
			mframe1=dst(Rect(iSrcX-mSiftX,iSrcY-mSiftY,_ad.TempleSize,_ad.TempleSize));
			mframe2=mCombine(Rect(0,_ad.TempleSize,_ad.TempleSize,_ad.TempleSize));
			mframe1.copyTo(mframe2);
			break;
	}
}

void TestDlg::CalSearchRect(Rect& srcRect,Rect& dstRect)
{
	switch(_dr)
	{
		case left:			
			srcRect.y=_ad.m_rows/2-_ad.m_overlap/4;
			dstRect.y=_ad.m_rows/2-_ad.m_overlap/2;
			dstRect.x=_ad.m_cols-_ad.m_overlap;
			srcRect.x=_ad.m_overlap/4;	
			break;
		case right:		
			srcRect.y=_ad.m_rows/2-_ad.m_overlap/4;
			dstRect.y=_ad.m_rows/2-_ad.m_overlap/2;
			dstRect.x=0;
			srcRect.x=_ad.m_cols-_ad.m_overlap/2-_ad.m_overlap/4;	
			break;
		case top:
			srcRect.x=_ad.m_cols/2-_ad.m_overlap/4;
			dstRect.x=_ad.m_cols/2-_ad.m_overlap/2;		
			srcRect.y=_ad.m_overlap/4;
			dstRect.y=_ad.m_rows-_ad.m_overlap;
			break;
		case bottom:
			srcRect.x=_ad.m_cols/2-_ad.m_overlap/4;
			dstRect.x=_ad.m_cols/2-_ad.m_overlap/2;
			srcRect.y=_ad.m_rows-_ad.m_overlap*3/4;
			dstRect.y=0;
			break;
	}

	//long dx,dy;//模板在源图像重叠区域的相对位置
	//dx = rect.left - ovlp.left;				
	//dy = rect.top - ovlp.top;
	//mRECT searchRect;
	//searchRect.left = dx + ovlp1.left - _ad.m_toleranceH;
	//searchRect.right = searchRect.left + (rect.right - rect.left + 1) + _ad.m_toleranceH*2;
	//searchRect.top = dy + ovlp1.top - _ad.m_toleranceV;
	//searchRect.bottom = searchRect.top + (rect.bottom - rect.top + 1) + _ad.m_toleranceV*2;
	//if(searchRect.left<0)searchRect.left = 0;
	//if(searchRect.right>=_ad.m_thdr.width)searchRect.right = _ad.m_thdr.width-1;
	//if(searchRect.top<0)searchRect.top = 0;
	//if(searchRect.bottom>=_ad.m_thdr.height)searchRect.bottom = _ad.m_thdr.height-1;
	//rect = searchRect;
}

/**
	\brief 8位灰度图像匹配。8 bits image matching.
	\param pTmplBuf [in]模版图像缓冲.Pointer to the template image buffer.
	\param tw [in]模版宽度.Template width.
	\param th [in]模版高度.Template height.
	\param pSearchBuf [in]搜索图像缓冲.Pointer to the searching image buffer.
	\param sw [in]搜索图像宽度.Searching image width.
	\param sh [in]搜索图像高度.Searching image height.
	\param x [out]模版在搜索图像上匹配度最高的位置。X-coor of matched position.
	\param y [out]模版在搜索图像上匹配度最高的位置。Y-corr of matched position.
	\param step [in]搜索步长（像素）.Searching steps in pixel.
	\return Matching value.匹配度因子。越大说明匹配程度越好。
	*/
double TestDlg::Match_8bits(const unsigned char* pTmplBuf,int tw,int th,
		const unsigned char* pSearchBuf,int sw,int sh,int &x,int &y,int step)
	{
		HSaveBmp((LPBYTE)pTmplBuf,tw,th,8,L".\\Test\\1.bmp");
		HSaveBmp((LPBYTE)pSearchBuf,sw,sh,8,L".\\Test\\2.bmp");

		//Calculate the total energy of the tmplate buffer
		const unsigned char* pTmp = pTmplBuf,*pTmp2=0;
		long tmplScanLineSize = step* tw;
		double tmplEnergy = 0;

		for(int i=0;i<th;i+=step)//取模板pTmplBuf里面的一部分
		{
			pTmp2 = pTmp;
			for(int j=0;j<tw;j+=step)
			{
				tmplEnergy += ((double)*pTmp2)**pTmp2;
				pTmp2 += step;
			}
			pTmp += tmplScanLineSize;
		}
		tmplEnergy = sqrt(tmplEnergy);
		//Match
		x = y = 0;
		double maxEv = -1.;
		long searchScanLineSize = step* sw;
		for(int i=0;i<sh-th;i+=step)
		{
			for(int j=0;j<sw-tw;j+=step)
			{
				//Calculate the matching value of current matching position
				const unsigned char* pSTmp ,* pSTmp2;
				pSTmp = pSearchBuf + i*sw + j;
				pTmp = pTmplBuf;
				double searchEnergy = 0.;
				double relativeEnergy = 0.;
				searchEnergy = 0.;
				relativeEnergy = 0.;
				for(int k = 0;k<th;k+=step)
				{
					pSTmp2 = pSTmp;
					pTmp2 = pTmp;
					for(int g = 0;g<tw;g+=step)
					{
						searchEnergy+=((double)*pSTmp2)**pSTmp2;
						relativeEnergy+=((double)*pSTmp2)**pTmp2;
						pSTmp2 += step;
						pTmp2 += step;
					}
					pSTmp += searchScanLineSize;
					pTmp += tmplScanLineSize;
				}
				double tmpEv = relativeEnergy/(sqrt(searchEnergy)*tmplEnergy);
				if(tmpEv>maxEv)
				{
					maxEv = tmpEv;
					x = j;
					y = i;
				}
			}            
		}
		return maxEv;
	}


void TestDlg::OnBnClickedNewcombinebtn2()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	_dr=(Dir)iCurRation;
	_ad.m_rows=_ad.m_cols=iCutSize;
	_ad.TempleSize=iTempleSize;//模板大小
	_ad.m_overlap=_ad.m_rows-_ad.TempleSize;//200;
	imgFiles.clear();
	imgFiles = DoSelectFiles(_T("*.bmp"), 
		OFN_ENABLESIZING   |OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY,
		_T("image files (*.bmp; *.png; *.jpg) |*.bmp; *.png; *.jpg; *.jpeg| All Files (*.*) |*.*||"),
		_T("SelectImg"),_T("Imgs\\Left"));
	vector<Mat> imgs;
	vector<Mat> iComBineMgs;
	if (imgFiles.size()<2)
		return;
	for (int i=0;i<imgFiles.size();i++)
	{
		CString str=(CString)imgFiles[i];
		std::string tempName=(LPCSTR)CStringA(str);
		const char *tmp=tempName.c_str();
		Mat mImg = cvLoadImage(tmp);

		if (_ad.m_cols<mImg.cols&&_ad.m_rows<mImg.rows)
		{
			int iY=mImg.rows/2-_ad.m_rows/2;
			int iX=mImg.cols/2-_ad.m_cols/2;
			iComBineMgs.push_back(mImg);
			Mat frame=mImg(Rect(iX,iY,_ad.m_cols,_ad.m_rows));  //测试  合成图/8-7以及8-8
			Mat frame2=cv::Mat(_ad.m_rows,_ad.m_cols,CV_8U);
			frame.copyTo(frame2);
			imgs.push_back(frame2);

		}
		else
		{
			AfxMessageBox(L"图像过小！");
			return;
		}
	}

	//第一、二副图
	Rect mSrcRect=Rect(0,0,_ad.m_overlap/4,_ad.m_overlap/4);
	Rect mDstRect=Rect(0,0,_ad.m_overlap,_ad.m_overlap);
	CalSearchRect(mSrcRect,mDstRect);
	Mat mSrcframe=imgs[0](mSrcRect);
	Mat mDstframe=imgs[1](mDstRect);
	cv::Mat mMatchValue=cv::Mat(mDstframe.rows,mDstframe.cols, CV_32F);
	matchTemplate(mDstframe,mSrcframe,mMatchValue,CV_TM_SQDIFF);
	double dMax=0,dMin=0;
	cv::Point minPt=Point(0,0),maxPt=Point(0,0);
	cv::minMaxLoc(mMatchValue,&dMin,&dMax,&minPt,&maxPt);

	int mSiftX=_ad.m_overlap/4-minPt.x;
	int mSiftY=_ad.m_overlap/4-minPt.y;

	//第三副图
	int iY=iComBineMgs[0].rows/2-_ad.m_rows/2;
	int iX=iComBineMgs[0].cols/2-_ad.m_cols/2;
	iX-=mSiftX;iY-=mSiftY;
	Mat imgs2=iComBineMgs[1](Rect(iX,iY,_ad.m_cols,_ad.m_rows)); 
	mSrcframe=imgs2(mSrcRect);
	mDstframe=imgs[2](mDstRect);
	matchTemplate(mDstframe,mSrcframe,mMatchValue,CV_TM_SQDIFF);
	cv::minMaxLoc(mMatchValue,&dMin,&dMax,&minPt,&maxPt);
	int mSiftX2=_ad.m_overlap/4-minPt.x;
	int mSiftY2=_ad.m_overlap/4-minPt.y;


	//合成
	Mat mCombine=Mat(_ad.m_rows,3*_ad.m_cols,iComBineMgs[0].type());
	Mat mframe1,mframe2;
	int iSrcX=iComBineMgs[0].cols/2-_ad.TempleSize/2;
	int iSrcY=iComBineMgs[0].rows/2-_ad.TempleSize/2;

	mframe1=iComBineMgs[0](Rect(iSrcX,iSrcY,_ad.TempleSize,_ad.TempleSize));
	mframe2=mCombine(Rect(0,0,_ad.TempleSize,_ad.TempleSize));
	mframe1.copyTo(mframe2);
	mframe1=iComBineMgs[1](Rect(iSrcX-mSiftX,iSrcY-mSiftY,_ad.TempleSize,_ad.TempleSize));
	mframe2=mCombine(Rect(_ad.TempleSize,0,_ad.TempleSize,_ad.TempleSize));
	mframe1.copyTo(mframe2);
	mframe1=iComBineMgs[2](Rect(iSrcX-mSiftX2,iSrcY-mSiftY2,_ad.TempleSize,_ad.TempleSize));
	mframe2=mCombine(Rect(_ad.TempleSize*2,0,_ad.TempleSize,_ad.TempleSize));
	mframe1.copyTo(mframe2);

	cv::namedWindow("FinalImg2",0);
	cv::imshow("FinalImg2",mCombine);
}


void TestDlg::OnLbnDblclkList2()
{
	// TODO: 在此添加控件通知处理程序代码
	mResultList.ResetContent();
	iCalCount=0;
}


void TestDlg::OnBnClickedReadimgbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	imgFiles.clear();
	imgFiles = DoSelectFiles(_T("*.bmp"), 
		OFN_ENABLESIZING   |OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY,
		_T("image files (*.bmp; *.png; *.jpg) |*.bmp; *.png; *.jpg; *.jpeg| All Files (*.*) |*.*||"),
		_T("SelectImg"),_T("Imgs\\Left"));

	vector<Mat> imgs;
	for (int i=0;i<imgFiles.size();i++)
	{
		CString str=(CString)imgFiles[i];
		std::string tempName=(LPCSTR)CStringA(str);
		const char *tmp=tempName.c_str();
		Mat mImg = cvLoadImage(tmp,0);
		imgs.push_back(mImg);
	}
	if (imgs.size()<=0)
		return;

	string strValHeader=".\\ImgValue.txt";
	FILE * fp=fopen((const char*)strValHeader.c_str(),"w");
	strValHeader="以下是图像的数据！\r\n";
	const char* mN=strValHeader.c_str();
	fwrite(mN,1,strlen(mN),fp);
	CString strWrite=L"";
	std::string tempName;
	int height=imgs[0].rows,width=imgs[0].cols;
	for (int i=0;i<height;i++)
	{
		for (int j=0;j<width;j++)
		{
			int fV=imgs[0].at<byte>(i,j);
			strWrite.Format(L"%d，",fV);
			if (j==width-1)
				strWrite += L"  \r\n下一行：";
			tempName=(LPCSTR)CStringA(strWrite);
			mN=tempName.c_str();
			fwrite(mN,1,strlen(mN),fp);
		}
	}
	fclose(fp);

	int iBuffSize=imgs[0].rows*imgs[0].cols*imgs[0].channels();
	LPBYTE m_RenderBuffer=(LPBYTE)malloc(iBuffSize);
	memcpy(m_RenderBuffer,imgs[0].data,iBuffSize);

	{
		string strValHeader=".\\ImgValueBuff.txt";
		FILE * fp=fopen((const char*)strValHeader.c_str(),"w");
		strValHeader="以下是图像的数据！\r\n";
		const char* mN=strValHeader.c_str();
		fwrite(mN,1,strlen(mN),fp);
		CString strWrite=L"";
		std::string tempName;
		int height=imgs[0].rows,width=imgs[0].cols;
		for (int i=0;i<height;i++)
		{
			for (int j=0;j<width;j++)
			{
				int fV=*m_RenderBuffer;
				strWrite.Format(L"%d，",fV);
				if (j==width-1)
					strWrite += L"  \r\n下一行：";
				tempName=(LPCSTR)CStringA(strWrite);
				mN=tempName.c_str();
				fwrite(mN,1,strlen(mN),fp);
				m_RenderBuffer++;
			}
		}
		fclose(fp);
	}
}
