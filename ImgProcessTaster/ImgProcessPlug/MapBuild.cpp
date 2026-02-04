#include "StdAfx.h"
#include "MapBuild.h"

CMapBuild::CMapBuild(void)
{
	m_ROIData.Vbuffer=0;
	m_ThumbData.Vbuffer=0;
	m_MapData.Vbuffer=0;
	m_MapNarrow.Vbuffer=0;
}

CMapBuild::~CMapBuild(void)
{
}

bool CMapBuild::InitProcess(LPVOID p_param)
{
	if (p_param!=0)
	{
		mMapBuildParas=(MapBuildParas*)p_param;
		ResetParas();
		if (mMapBuildParas->bMapSave||mMapBuildParas->bROIImgSave)
		{
			strSaveFile=mMapBuildParas->MapSavePath+L"\\"+HGetTimeString();
			//CreateDirectory(strSaveFile,NULL);
			strROIPath=strSaveFile+L"\\ROIImg";
			strImgPath=strSaveFile+L"\\Img";
			if (mMapBuildParas->bROIImgSave)
			{
				mMapBuildParas->bROIImgSave=BuildDirectory(strROIPath);
				mMapBuildParas->bROIImgSave=BuildDirectory(strImgPath);
			}			
			if (mMapBuildParas->bMapSave)
				mMapBuildParas->bMapSave=BuildDirectory(strSaveFile);
			//CreateDirectory(strROIPath,NULL);
			//CreateDirectory(strImgPath,NULL);
		}
		return true;
	}	
	return false;
}

void CMapBuild::ResetParas()
{
	//计算移动的次数
	CSize mROISize;//ROI图的大小
	m_MoveStep=(float)mMapBuildParas->ImageROISize*mMapBuildParas->PixelSize;
	int m_XCount=abs(mMapBuildParas->EndPt.x-mMapBuildParas->StartPt.x)/m_MoveStep;//+1;
	int m_YCount=abs(mMapBuildParas->EndPt.y-mMapBuildParas->StartPt.y)/m_MoveStep;//+1;
	if (mMapBuildParas->StartPt.x+(m_XCount+1)*m_MoveStep<mMapBuildParas->MaxPt.x)
		m_XCount++;
	if (mMapBuildParas->StartPt.y+(m_YCount+1)*m_MoveStep<mMapBuildParas->MaxPt.y)
		m_YCount++;

	mMapBuildParas->RowCount=m_XCount;
	mMapBuildParas->ColCount=m_YCount;

	mROISize.cx=mMapBuildParas->ImageROISize;
	mROISize.cy=mMapBuildParas->ImageROISize;

	//计算出缩略图
	CSize mThumbSize;
	float mMapW,mMapH;//缩小地图的长宽路程，mm为单位

	mMapW=mMapBuildParas->MaxPt.x-mMapBuildParas->MinPt.x+0.0f;
	mMapH=mMapBuildParas->MaxPt.y-mMapBuildParas->MinPt.y+0.0f;//整个地图的长宽
	mThumbSize.cx=m_MoveStep/mMapW*mMapBuildParas->iMapSizeX;//缩略图的大小 
	if (mThumbSize.cx%4!=0)
		mThumbSize.cx=mThumbSize.cx-mThumbSize.cx%4+4;
	mThumbSize.cy=mThumbSize.cx*((mROISize.cy+0.0f)/(mROISize.cx+0.0f));

	//按照推算，应该改变终点位置！
	mMapBuildParas->EndPt.x=mMapBuildParas->StartPt.x+m_XCount*m_MoveStep;
	mMapBuildParas->EndPt.y=mMapBuildParas->StartPt.y+m_YCount*m_MoveStep;

	//推算按照比例，地图应该变成的宽
	//m_MapNarrow.Vwidth=mMapBuildParas->iMapSizeX;
	float mXTraPer=mMapBuildParas->EndPt.x-mMapBuildParas->StartPt.x;
	mXTraPer=mXTraPer/mMapW;
	m_MapNarrow.Vwidth=(m_XCount*mThumbSize.cx+0.0f)/mXTraPer;
	if (m_MapNarrow.Vwidth%4!=0)
	{
		m_MapNarrow.Vwidth=m_MapNarrow.Vwidth-m_MapNarrow.Vwidth%4+4;
	}
	//推算按照比例，地图应该变成的高
	float mYTraPer=mMapBuildParas->EndPt.y-mMapBuildParas->StartPt.y;
	mYTraPer=mYTraPer/mMapH;
	m_MapNarrow.Vheight=(m_YCount*mThumbSize.cy+0.0f)/mYTraPer+1;

	float mX=mMapBuildParas->StartPt.x;
	float mY=mMapBuildParas->StartPt.y;
	mThumStartPt.x=mX/mMapW*m_MapNarrow.Vwidth;
	//float mTestV=(mX+0.0f)/mMapW*m_MapNarrow.Vwidth;
	//mThumStartPt.x=mTestV;

	mThumStartPt.y=m_MapNarrow.Vheight-mY/mMapH*m_MapNarrow.Vheight-m_YCount*mThumbSize.cy;//缩略图的起Y方向起点
	//mTestV=(mY+0.0f)/mMapH*m_MapNarrow.Vheight;
	//mTestV=m_MapNarrow.Vheight-mTestV-m_YCount*mThumbSize.cy;
	//mThumStartPt.y=mTestV;

	//CString str;
	//str.Format(L"新终点：%0.2f,%0.2f,大小:%d X %d;Y的起点：%d",mMapBuildParas->EndPt.x,mMapBuildParas->EndPt.y,
	//	m_MapNarrow.Vwidth,m_MapNarrow.Vheight,mThumStartPt.y);
	//AfxMessageBox(str);

	//设置地图图像数据参数
	m_ROIData.Vwidth=mROISize.cx;
	m_ROIData.Vheight=mROISize.cy;
	m_ThumbData.Vwidth=mThumbSize.cx;
	m_ThumbData.Vheight=mThumbSize.cy;

	m_MapData.Vwidth=m_XCount*mROISize.cx;;//iMapWholeX;
	m_MapData.Vheight=m_YCount*mROISize.cy;//iMapWholeY;

	m_ROIData.Vbuffer=0;
	m_ThumbData.Vbuffer=0;
	m_MapData.Vbuffer=0;
	m_MapNarrow.Vbuffer=0;
	iShiftX=iShiftY=0;
	iPreShiftX=iPreShiftY=0;
	bNewProcess=true;
}

bool CMapBuild::UnInitProcess()
{
	if (m_MapData.Vbuffer)
	{
		delete(m_MapData.Vbuffer);
		m_MapData.Vbuffer=0;
	}
	if (m_ROIData.Vbuffer)
	{
		delete(m_ROIData.Vbuffer);
		m_ROIData.Vbuffer=0;
	}
	if (m_ThumbData.Vbuffer)
	{
		delete(m_ThumbData.Vbuffer);
		m_ThumbData.Vbuffer=0;
	}
	if (m_MapNarrow.Vbuffer)
	{
		delete(m_MapNarrow.Vbuffer);
		m_MapNarrow.Vbuffer=0;
	}
	return true;
}

bool CMapBuild::ProcessImg(void* InputPara,void* OutPutPara,HVideoHeader* pHeader)
{
	//Setp1 找原点
	if (InputPara==0||InputPara==NULL)
		return false;
	int iCount=*(int*)OutPutPara;
	CPoint* Pt=(CPoint*)InputPara;
	
	//GetMatchShift(pHeader,*Pt);//计算拼接位置

	//step2 获取ROI
	m_ROIData.VwBit=pHeader->VwBit;
	m_ROIData.Vsize=m_ROIData.Vwidth*m_ROIData.Vheight*m_ROIData.VwBit/8;
	if (mMapBuildParas->mScanType==Scan_ZStack)
		GetImgROI2(pHeader,&m_ROIData,*Pt);
	else 
		GetImgROI(pHeader,&m_ROIData);

	//step3 获取缩略图
	m_ThumbData.VwBit=pHeader->VwBit;
	m_ThumbData.Vsize=m_ThumbData.Vheight*m_ThumbData.Vwidth*m_ThumbData.VwBit/8;
	GetImgThumb(&m_ROIData,&m_ThumbData);

	if (mMapBuildParas->bMapSave)
	{
		//setp4 复制大地图数据
		CPoint tPoint=GetROIMapPoint(Pt->x,Pt->y);
		m_MapData.VwBit=pHeader->VwBit;
		if (!CopyROIImg(&m_ROIData,&m_MapData,tPoint))
			return false;
	}
	
	//step5 组合缩略图
	CPoint tPoint=GetMapPoint(Pt->x,Pt->y);
	m_MapNarrow.VwBit=pHeader->VwBit;
	if (!CopyROIImg(&m_ThumbData,&m_MapNarrow,tPoint))
		return false;

	int mV=m_MoveStep*1000;//方框ROI的实际长度
	Notify(this,NOTIFY_MAPROISIZE_FLUSH,mV,&m_MapNarrow);

	//判断是否存储缩图
	if (mMapBuildParas->bROIImgSave)
	{
		CString str;
		str.Format(L"\\Img%d-%d.jpg",mMapBuildParas->ColCount-1-Pt->y,Pt->x);
		str=strImgPath+str;
		SaveImage(pHeader,str,FLIP_VERTICAL);
		str.Format(L"\\ROI%d-%d.jpg",mMapBuildParas->ColCount-1-Pt->y,Pt->x);
		str=strROIPath+str;
		SaveImage(&m_ROIData,str,FLIP_VERTICAL);
	}	

	//如果合成结束
	if (pHeader->VnCount>=iCount)
	{
		SaveImage(&m_MapNarrow,L".\\EpiResult\\NaviMap.bmp",FLIP_VERTICAL);
		delete(m_MapNarrow.Vbuffer);m_MapNarrow.Vbuffer=0;
		CString str;
		if (mMapBuildParas->bMapSave)
		{
			str=strSaveFile+L"\\MapImg.jpg";
			SaveImage(&m_MapData,str,FLIP_VERTICAL);
			delete(m_MapData.Vbuffer);m_MapData.Vbuffer=0;
			Notify(this,NOTIFY_MAP_END,0,&str,0,&strSaveFile);
		}	
		UnInitProcess();		
	}

	delete(m_ROIData.Vbuffer);m_ROIData.Vbuffer=0;
	delete(m_ThumbData.Vbuffer);m_ThumbData.Vbuffer=0;
	return true;
}

CPoint CMapBuild::GetROIMapPoint( int pXCount,int pYCount)
{
	CPoint tRes;
	tRes.x=pXCount*m_ROIData.Vwidth;//mMapBuildParas->iMapSizeX;
	tRes.y=pYCount*m_ROIData.Vheight;//iMapSizeY-pYCount*iMapSizeY-mThumSizeY;//图像数据是从下往上，从做到右的？
	return tRes;
}

CPoint CMapBuild::GetMapPoint( int pXCount,int pYCount)
{
	CPoint tRes;
	tRes.x=pXCount*m_ThumbData.Vwidth+mThumStartPt.x;
	tRes.y=pYCount*m_ThumbData.Vheight+mThumStartPt.y;
	return tRes;
}

void CMapBuild::GetImgROI2( HVideoHeader* pScr,HVideoHeader* pDest,CPoint pt)
{
	pDest->Vbuffer=(LPBYTE)malloc(pDest->Vsize);//建大小
	//以下是原图
	int width=pScr->Vwidth;
	int height=pScr->Vheight;
	int index=int(pScr->VcaptureTime);//这里暂用capture time代替编号
	int wBit=pScr->VwBit/8;//这里图的位数
	cv::Mat mCurImg;
	cv::Mat mGrayImg;
	if (wBit==3)
	{
		mCurImg=cv::Mat(height,width,CV_8UC3,pScr->Vbuffer);
		cv::flip(mCurImg,mCurImg,0);//上下颠倒
		cvtColor(mCurImg,mGrayImg,CV_RGB2GRAY);
	}		
	else if(wBit==1)
	{
		mCurImg=cv::Mat(height,width,CV_8U,pScr->Vbuffer);
		cv::flip(mCurImg,mCurImg,0);//上下颠倒
		mGrayImg=mCurImg;//cv::Mat(height,width,CV_8U);
		//mCurImg.copyTo(mGrayImg);
	}
	//来判断是不是第一次
	int iY=mGrayImg.rows/2-mMapBuildParas->m_rows/2;
	int iX=mGrayImg.cols/2-mMapBuildParas->m_cols/2;
	if (mPrePt.y!=pt.y)//判断方向！
	{
		_dr=Dir::bottom;
		//iPreShiftX=0;
		//iPreShiftY=0;
	}		
	else if (mPrePt.x<pt.x)
		_dr=Dir::right;
	else if (mPrePt.x>pt.x)
		_dr=Dir::left;

	iX-=iPreShiftX;
	iY-=iPreShiftY;
	Mat frame=mGrayImg(Rect(iX,iY,mMapBuildParas->m_cols,mMapBuildParas->m_rows));
	if (!bNewProcess)//不是第一次，做匹配
	{	

		Rect mSrcRect=Rect(0,0,mMapBuildParas->m_overlap/4,mMapBuildParas->m_overlap/4);
		Rect mDstRect=Rect(0,0,mMapBuildParas->m_overlap,mMapBuildParas->m_overlap);
		CalSearchRect(mSrcRect,mDstRect);
		Mat mSrcframe=mPreImg(mSrcRect);
		Mat mDstframe=frame(mDstRect);
		cv::Mat mMatchValue=cv::Mat(mDstframe.rows,mDstframe.cols, CV_32F);
		matchTemplate(mDstframe,mSrcframe,mMatchValue,CV_TM_SQDIFF);
		double dMax=0,dMin=0;
		cv::Point minPt=Point(0,0),maxPt=Point(0,0);
		cv::minMaxLoc(mMatchValue,&dMin,&dMax,&minPt,&maxPt);
		iShiftX=mMapBuildParas->m_overlap/4-minPt.x;
		iShiftY=mMapBuildParas->m_overlap/4-minPt.y;
		float iMax=(mMapBuildParas->m_overlap/4+0.0f)*0.8f;
		//bool bR=true;
		//if (bR)
		//{
		frame=mGrayImg(Rect(iX-iShiftX,iY-iShiftY,mMapBuildParas->m_cols,mMapBuildParas->m_rows));
		//}
		frame.copyTo(mPreImg);

		//frame=mGrayImg(Rect(iX,iY,mMapBuildParas->m_cols,mMapBuildParas->m_rows));
		//cv::rectangle(frame,mDstRect,Scalar(0,0,255),5);//还没重选就应该绘制上
		//cv::rectangle(mDstframe,Rect(minPt.x,minPt.y,mSrcRect.width,mSrcRect.height),Scalar(0,0,255),5);//还没重选就应该绘制上
		//cv::rectangle(frame,mSrcRect,Scalar(0,0,255),5);//绘上 用于下副图的Src模板		
		mPrePt=pt;	
	}	
	else//第一次，复制保存图
	{	
		//Rect mSrcRect=Rect(0,0,mMapBuildParas->m_overlap/4,mMapBuildParas->m_overlap/4);
		//Rect mDstRect=Rect(0,0,mMapBuildParas->m_overlap,mMapBuildParas->m_overlap);
		//CalSearchRect(mSrcRect,mDstRect);
		mPreImg=cv::Mat(mMapBuildParas->m_rows,mMapBuildParas->m_cols,CV_8U);
		//cv::rectangle(mPreImg,mSrcRect,Scalar(255,0,0));

		frame.copyTo(mPreImg);
		mPrePt=pt;
		bNewProcess=false;
	}
	//截取图像！
	long StartX=(pScr->Vwidth-pDest->Vwidth)/2;
	long StartY=(pScr->Vheight-pDest->Vheight)/2;
	StartX-=iPreShiftX;
	StartY-=iPreShiftY;
	StartX-=iShiftX;
	StartY-=iShiftY;
	cv::Mat mDstImg=cv::Mat(pDest->Vheight,pDest->Vwidth,mCurImg.type(),pDest->Vbuffer);
	mCurImg(Rect(StartX,StartY,pDest->Vheight,pDest->Vwidth)).copyTo(mDstImg);
	iPreShiftX=iShiftX;
	iPreShiftY=iShiftY;
	cv::flip(mCurImg,mCurImg,0);//上下返回颠倒
	cv::flip(mDstImg,mDstImg,0);//上下返回颠倒
}

bool CMapBuild::GetMatchShift(HVideoHeader* pHeader,CPoint pt)
{
	int width=pHeader->Vwidth;
	int height=pHeader->Vheight;
	int index=int(pHeader->VcaptureTime);//这里暂用capture time代替编号
	int wBit=pHeader->VwBit/8;//这里图的位数
	cv::Mat mImg;
	if (wBit==3)
	{
		cv::Mat mOldImg=cv::Mat(height,width,CV_8UC3,pHeader->Vbuffer);
		cvtColor(mOldImg,mImg,CV_RGB2GRAY);
	}		
	else if(wBit==1)
		mImg=cv::Mat(height,width,CV_8U,pHeader->Vbuffer);

	cv::flip(mImg,mImg,0);//上下颠倒

	//判断方向！
	if (mPrePt.y>pt.y)
	{
		_dr=Dir::bottom;
		iPreShiftX=0;
	}		
	else if (mPrePt.x<pt.x)
		_dr=Dir::right;
	else if (mPrePt.x>pt.x)
		_dr=Dir::left;

	int iY=mImg.rows/2-mMapBuildParas->m_rows/2;
	int iX=mImg.cols/2-mMapBuildParas->m_cols/2;	
	iX-=iPreShiftX;
	iY-=iPreShiftY;
	Mat frame=mImg(Rect(iX,iY,mMapBuildParas->m_cols,mMapBuildParas->m_rows));

	if (bNewProcess)//第一次
	{
		mPreImg=cv::Mat(mMapBuildParas->m_rows,mMapBuildParas->m_cols,CV_8U);
		frame.copyTo(mPreImg);
		mPrePt=pt;
		bNewProcess=false;

		cv::flip(mImg,mImg,0);//返回颠倒
		return true;
	}

	Rect mSrcRect=Rect(0,0,mMapBuildParas->m_overlap/4,mMapBuildParas->m_overlap/4);
	Rect mDstRect=Rect(0,0,mMapBuildParas->m_overlap,mMapBuildParas->m_overlap);
	CalSearchRect(mSrcRect,mDstRect);
	Mat mSrcframe=mPreImg(mSrcRect);
	Mat mDstframe=frame(mDstRect);
	cv::Mat mMatchValue=cv::Mat(mDstframe.rows,mDstframe.cols, CV_32F);
	matchTemplate(mDstframe,mSrcframe,mMatchValue,CV_TM_SQDIFF);
	double dMax=0,dMin=0;
	cv::Point minPt=Point(0,0),maxPt=Point(0,0);
	cv::minMaxLoc(mMatchValue,&dMin,&dMax,&minPt,&maxPt);
	iShiftX=mMapBuildParas->m_overlap/4-minPt.x;
	iShiftY=mMapBuildParas->m_overlap/4-minPt.y;
	float iMax=(mMapBuildParas->m_overlap/4+0.0f)*0.8f;
	bool bR=true;

	/*if (abs(iShiftX)>=iMax||abs(iShiftY)>=iMax)
	{
	iShiftX=iShiftY=0;
	bR=false;
	}	
	else
	bR=true;*/

	//CString str;
	//str.Format(L"\\SrcImg%d-%d.jpg",pt.y,pt.x);
	//str=strImgPath+str;
	//std::string tempName=(LPCSTR)CStringA(str);
	//const char* mN=tempName.c_str();
	//imwrite(mN,mSrcframe);

	//str.Format(L"\\DstImg%d-%d.jpg",pt.y,pt.x);
	//str=strImgPath+str;
	//tempName=(LPCSTR)CStringA(str);
	//mN=tempName.c_str();
	//imwrite(mN,mDstframe);

	//str.Format(L"\\PreImg%d-%d.jpg",pt.y,pt.x);
	//str=strImgPath+str;
	//tempName=(LPCSTR)CStringA(str);
	//mN=tempName.c_str();
	//imwrite(mN,frame);
	if (bR)
	{
		frame=mImg(Rect(iX-iShiftX,iY-iShiftY,mMapBuildParas->m_cols,mMapBuildParas->m_rows));
	}
	frame.copyTo(mPreImg);
	mPrePt=pt;

	//str.Format(L"\\PreImg%d-%dNew.jpg",pt.y,pt.x);
	//str=strImgPath+str;
	//tempName=(LPCSTR)CStringA(str);
	//mN=tempName.c_str();
	//imwrite(mN,mPreImg);
	cv::flip(mImg,mImg,0);//返回颠倒
	return bR;
}

void CMapBuild::GetImgROI( HVideoHeader* pScr,HVideoHeader* pDest )
{
	pDest->Vbuffer=(LPBYTE)malloc(pDest->Vsize);//建大小
	int height=pDest->Vheight;
	LPBYTE tScr=pScr->Vbuffer;
	LPBYTE tDest=pDest->Vbuffer;
	long StartX=(pScr->Vwidth-pDest->Vwidth)*pScr->VwBit/16;//目标中的启始位置
	long StartY=(pScr->Vheight-pDest->Vheight)/2;
	int SCpSize=pDest->Vwidth*pDest->VwBit/8;
	int SScrSize=pScr->Vwidth*pScr->VwBit/8;
	tScr+=StartX+StartY*pScr->Vwidth;//目标
	for (int j=0;j<height;j++)//一行一行的复制
	{
		memcpy(tDest,tScr,SCpSize);
		tDest+=SCpSize;
		tScr+=SScrSize;
	}
}

void CMapBuild::GetImgThumb( HVideoHeader* pScr,HVideoHeader* pDest )
{
	pDest->Vbuffer=(LPBYTE)malloc(pDest->Vsize);//建大小
	cv::Mat frame;
	cv::Mat mImg;
	int wBit=pScr->VwBit/8;//这里图的位数
	if (wBit==3)
	{
		frame=cv::Mat(pScr->Vheight,pScr->Vwidth,CV_8UC3,pScr->Vbuffer);
		mImg=cv::Mat(pDest->Vheight,pDest->Vwidth,CV_8UC3,pDest->Vbuffer);
	}		
	else if(wBit==1)
	{
		frame=cv::Mat(pScr->Vheight,pScr->Vwidth,CV_8U,pScr->Vbuffer);
		mImg=cv::Mat(pDest->Vheight,pDest->Vwidth,CV_8U,pDest->Vbuffer);
	}
	Size mSize=cv::Size(pDest->Vwidth,pDest->Vheight);
	cv::resize(frame,mImg,mSize);
	return;


	//int height=pScr->Vheight;
	//int width=pScr->Vwidth*pScr->VwBit/8;

	//int wbit=pDest->VwBit/8;//位数

	//int DestHeight=pDest->Vheight;
	//int DestWidth=pDest->Vwidth;

	//LPBYTE tScr=pScr->Vbuffer;
	//LPBYTE tDest=pDest->Vbuffer;

	//float tSkipX=(float)(pScr->Vwidth)/(float)(pDest->Vwidth);
	//float tSkipY=(float)(pScr->Vheight)/(float)(pDest->Vheight);

	//float tCountX=0,tCountY=0;

	//for (int i=0;i<DestHeight;i++)
	//{
	//	tCountX=0;
	//	for(int j=0;j<DestWidth;j++)
	//	{

	//		int index=(int)(tCountX)*wbit;
	//		*tDest=tScr[index];
	//		if(wbit>1)
	//		{
	//			tDest[1]=tScr[index+1];
	//			tDest[2]=tScr[index+2];
	//			tDest+=3;
	//		}
	//		else
	//			tDest++;
	//		tCountX+=tSkipX;//下一个目标
	//	}
	//	tScr+=(width*(int)tSkipY);//下i一行
	//}
}

bool CMapBuild::CopyROIImg( HVideoHeader* pScr,HVideoHeader* pDest,CPoint tPoint)
{
	//CPoint tPoint=GetROIMapPoint(pt.x,pt.y);
	if(!pDest->Vbuffer)
	{
		int mK=pDest->VwBit/8;
		pDest->Vsize=pDest->Vwidth*pDest->Vheight*mK;
		pDest->Vbuffer=(LPBYTE)malloc(pDest->Vsize);
		if (!pDest->Vbuffer)
		{
			AfxMessageBox(L"The Map size is too big！");
			return false;
		}
		memset(pDest->Vbuffer,0,pDest->Vsize);
	}

	int wByte=pDest->VwBit/8;
	int LoopY=pScr->Vheight;
	int ThumbWidthSize=pScr->Vwidth*wByte;//复制的每行内存大小
	int MapWidthSize=pDest->Vwidth*wByte;
	long tPosBuffer=tPoint.x*wByte+tPoint.y*MapWidthSize;//地图数据Buff的起点

	LPBYTE lpDst=&(pDest->Vbuffer[tPosBuffer]);
	LPBYTE lpSrc=pScr->Vbuffer;
	for(int i=0;i<LoopY;i++)
	{
		memcpy(lpDst,lpSrc,ThumbWidthSize);
		lpDst+=MapWidthSize;
		lpSrc+=ThumbWidthSize;
	}
	return true;
}

//检查输入的目录是否存在，如不存在，则创建新目录
bool CMapBuild::BuildDirectory(CString strPath)
{
	CString strSubPath;
	CString strInfo;
	int nCount = 0; 
	int nIndex = 0;

	//查找字符"\\"的个数
	do
	{
		nIndex = strPath.Find(L"\\",nIndex) + 1;
		nCount++;
	}while( (nIndex-1) != -1);
	nIndex = 0;
	//检查，并创建目录
	while( (nCount-1) >= 0)
	{
		nIndex = strPath.Find(L"\\",nIndex) + 1;
		if( (nIndex - 1) == -1)
		{
			strSubPath = strPath;
		}
		else
			strSubPath = strPath.Left(nIndex);
		if(!PathFileExists(strSubPath))
		{
			if(!::CreateDirectory(strSubPath,NULL))
			{
				strInfo = L"Build Directory";
				strInfo += strSubPath;
				strInfo += L" Fail!";
				AfxMessageBox(strInfo,MB_OK);
				return FALSE;
			}
		}
		nCount--;
	};
	return TRUE;
}

void CMapBuild::CalSearchRect(Rect& srcRect,Rect& dstRect)
{
	switch(_dr)
	{
	case Dir::left:			
		srcRect.y=mMapBuildParas->m_rows/2-mMapBuildParas->m_overlap/4;
		dstRect.y=mMapBuildParas->m_rows/2-mMapBuildParas->m_overlap/2;
		dstRect.x=mMapBuildParas->m_cols-mMapBuildParas->m_overlap;
		srcRect.x=mMapBuildParas->m_overlap/4;	
		break;
	case Dir::right:		
		srcRect.y=mMapBuildParas->m_rows/2-mMapBuildParas->m_overlap/4;
		dstRect.y=mMapBuildParas->m_rows/2-mMapBuildParas->m_overlap/2;
		dstRect.x=0;
		srcRect.x=mMapBuildParas->m_cols-mMapBuildParas->m_overlap/2-mMapBuildParas->m_overlap/4;	
		break;
	case Dir::top:
		srcRect.x=mMapBuildParas->m_cols/2-mMapBuildParas->m_overlap/4;
		dstRect.x=mMapBuildParas->m_cols/2-mMapBuildParas->m_overlap/2;		
		srcRect.y=mMapBuildParas->m_overlap/4;
		dstRect.y=mMapBuildParas->m_rows-mMapBuildParas->m_overlap;
		break;
	case Dir::bottom:
		srcRect.x=mMapBuildParas->m_cols/2-mMapBuildParas->m_overlap/4;
		dstRect.x=mMapBuildParas->m_cols/2-mMapBuildParas->m_overlap/2;
		srcRect.y=mMapBuildParas->m_rows-mMapBuildParas->m_overlap*3/4;
		dstRect.y=0;
		break;
	}
}



