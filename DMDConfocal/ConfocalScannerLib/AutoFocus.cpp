#include "StdAfx.h"
#include "AutoFocus.h"


CAutoFocus::CAutoFocus(void)
{
	m_ConfocalCore=0;
	m_CaptureVideo.Vbuffer=0;
	m_CaptureVideo.Vsize=0;
	m_hMoveEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_hCaptureEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_ScanThread=0;
	bFocusOver=false;
	bWaitMode=false;
}

CAutoFocus::~CAutoFocus(void)
{
}

bool CAutoFocus::InitPlugin(LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	m_AxisZ=0;
	if(m_ConfocalCore)
	{
		if(m_ConfocalCore->GetGearBox())
			m_AxisZ=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Z);//初始化载物台
		if (m_AxisZ)
			m_AxisZ->GetSubject()->Attach(this);
		m_CurVideoDevice=m_ConfocalCore->GetCurVideo();//初始化相机
		mAutoProcess=m_ConfocalCore->GetCoreProcess(PROCESS_ImgQuality);
	}
	return true;
}

bool CAutoFocus::UnInitPlugin()
{
	bFocusOver=false;
	//if (m_CaptureVideo.Vbuffer)
	//	delete(m_CaptureVideo.Vbuffer);
	return true;
}

bool CAutoFocus::Stop()
{
	bFocusOver=false;
	return true;
}

int CAutoFocus::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if (pSubject==m_AxisZ->GetSubject())
	{
		if(ID==NOTIFY_AXIS_MOVED)//||NOTIFY_AXIS_POSITION)//
		{
			SetEvent(m_hMoveEvent);
		}
	}
	return 1;
}

bool CAutoFocus::Set(void* WParas)
{
	if (WParas!=0)
	{
		mAutoFocusParas=(AutoFocusParas*)WParas;
		mAutoProcess->InitProcess(WParas);
		return true;
	}
	return false;
}

bool CAutoFocus::Start(void* vParas)
{
	if (m_ScanThread!=0)
	{
		if (WaitForSingleObject(m_ScanThread,300)==WAIT_TIMEOUT)
		{
			AfxMessageBox(L"The AutoFocus thread havn't finished!");
			SetEvent(m_hMoveEvent);
			SetEvent(m_hCaptureEvent);
			return false;
		}
	}

	bWaitMode=false;
	if (vParas!=0)
	{
		bWaitMode=*(bool*)vParas;
		WaitStart();
		return true;
	}

	m_CaptureVideo.VnCount=0;//表示当前编号
	iControl=0;
	iCon5Direct=0;
	m_ConfocalCore->GetRenderChain()->push_back(this);//加入渲染

	ResetEvent(m_hMoveEvent);
	ResetEvent(m_hCaptureEvent);
	bFocusOver=true;

	m_CurVideoDevice->SetTrigerMode(TRIGER_SOFT);
	m_ScanThread = CreateThread(NULL, 0, AutoFocusProc, this, 0, 0);//这个开线程
	HCorePanel* mDlg=m_ConfocalCore->GetMsgPanel(MSG_TIME);
	mDlg->InitPanel(0);
	this->Attach(mDlg);
	m_ConfocalCore->GetDockablePanel(DOCKPANEL_STAGECTL)
		->GetCWnd()->PostMessage(0x118);
	((CDialogEx*)mDlg->GetCWnd())->DoModal();
	this->Detach(mDlg);
	bFocusOver=false;//退出线程
	WaitForSingleObject(m_ScanThread,2000);
	m_ConfocalCore->GetRenderChain()->remove(this);
	mAutoProcess->UnInitProcess();

	return true;
}

void CAutoFocus::WaitStart()
{
	m_CaptureVideo.VnCount=0;//表示当前编号
	iControl=0;
	iCon5Direct=0;
	m_ConfocalCore->GetRenderChain()->push_back(this);//加入渲染
	ResetEvent(m_hMoveEvent);
	ResetEvent(m_hCaptureEvent);
	bFocusOver=true;
	m_ScanThread = CreateThread(NULL, 0, AutoFocusProc, this, 0, 0);//这个开线程
	HCorePanel* mDlg=m_ConfocalCore->GetMsgPanel(MSG_TIME);
	this->Attach(mDlg);
	WaitForSingleObject(m_ScanThread,INFINITE);//等待线程结束
	this->Detach(mDlg);
	bFocusOver=false;//退出线程
	mAutoProcess->UnInitProcess();
}

DWORD WINAPI AutoFocusProc( LPVOID lp )
{
	((CAutoFocus*)lp)->ScanAuto();
	return 0;
}

void CAutoFocus::ScanAuto()
{
	InitValue();
	float mfV=0;
	double* mFocusV=new double;
	*mFocusV=0;
	Quality_TYPE mfCuQua=ImgQuality_CuQuality;
	Quality_TYPE mfXiQua=ImgQuality_XiQuality;
	while(bFocusOver)
	{
		switch(iControl)
		{
		case 0:
			mfV=2*mAutoFocusParas->mCuStep;
			for (int i=0;i<3;i++)
			{
				if (i==0)
					MoveZ(1,mfV);
				else
					MoveZ(-1,mfV);
				if (!Capture())//采集图像
				{
					bFocusOver=false;
					break;
				}
				//Capture();
				mAutoProcess->ProcessImg(&mfCuQua,mFocusV,&m_CaptureVideo);
				mPreDr.push_back(*mFocusV);
				mPreDrP.push_back(m_AxisZ->GetPosition());
			}
			if (!bFocusOver)
				break;
			Contrl0();
			MoveToZ(mPredw);
			break;
		case 1:
			if (!Capture())//采集图像
			{
				bFocusOver=false;
				break;
			}
			mAutoProcess->ProcessImg(&mfCuQua,mFocusV,&m_CaptureVideo);
			mCuDr.push_back(*mFocusV);
			mCuDrP.push_back(m_AxisZ->GetPosition());
			MoveZ(mPreDirection,mAutoFocusParas->mDrStep);//向上移动位置
			if (!Capture())//采集图像
			{
				bFocusOver=false;
				break;
			}
			mAutoProcess->ProcessImg(&mfCuQua,mFocusV,&m_CaptureVideo);
			mCuDr.push_back(*mFocusV);
			mCuDrP.push_back(m_AxisZ->GetPosition());
			if (!bFocusOver)
				break;
			Contrl1();
			break;
		case 2:
			if (!Capture())//采集图像
			{
				bFocusOver=false;
				break;
			}
			mAutoProcess->ProcessImg(&mfCuQua,mFocusV,&m_CaptureVideo);
			mCuContinuous.push_back(*mFocusV);
			mCuContinuousP.push_back(m_AxisZ->GetPosition());
			for (int i=0;i<4;i++)
			{
				MoveZ(mDirectionDr,mAutoFocusParas->mCuStep);//移动位置
				if (!Capture())//采集图像
				{
					bFocusOver=false;
					break;
				}
				mAutoProcess->ProcessImg(&mfCuQua,mFocusV,&m_CaptureVideo);
				mCuContinuous.push_back(*mFocusV);
				mCuContinuousP.push_back(m_AxisZ->GetPosition());
			}
			if (!bFocusOver)
				break;
			Contrl2();
			MoveToZ(mRdw);
			break;
		case 3:
			for (int i=0;i<3;i++)
			{
				if (i==0)
					MoveZ(1,mAutoFocusParas->mCuStep);
				else
					MoveZ(-1,mAutoFocusParas->mCuStep);	
				if (!Capture())//采集图像
				{
					bFocusOver=false;
					break;
				}
				mAutoProcess->ProcessImg(&mfCuQua,mFocusV,&m_CaptureVideo);
				mCuTriangle.push_back(*mFocusV);
				mCuTriangleP.push_back(m_AxisZ->GetPosition());
			}
			if (!bFocusOver)
				break;
			Contrl3();
			MoveToZ(mRough);
			break;
		case 4:
			break;
		case 5:
			if (!MoveContrl5(iCon5Direct))
			{
				bFocusOver=false;
				break;
			}
			if (!bFocusOver)
				break;
			Contrl5();
			MoveToZ(mXdw);
			break;
		case 6:
			for (int i=0;i<3;i++)
			{
				if (i==0)
					MoveZ(1,mAutoFocusParas->mXiStep);
				else
					MoveZ(-1,mAutoFocusParas->mXiStep);			
				if (!Capture())//采集图像
				{
					bFocusOver=false;
					break;
				}
				mAutoProcess->ProcessImg(&mfXiQua,mFocusV,&m_CaptureVideo);
				mXiTriangle.push_back(*mFocusV);
				mXiTriangleP.push_back(m_AxisZ->GetPosition());
			}
			if (!bFocusOver)
				break;
			Contrl6();
			MoveToZ(mFocus);
			break;
		case 7:
			break;
		default:
			bFocusOver=false;
			break;
		}	
	}
	if (mAutoFocusParas->bRecord)
	{
		m_CaptureVideo.VnCount=99;
		Capture();
		SaveFocusValue();
	}
	m_ConfocalCore->GetRenderChain()->remove(this);
	if (!bWaitMode)
	{
		m_CurVideoDevice->SetTrigerMode(TRIGER_INTERAL);
		Notify(this,NOTIFY_MSGVIEW_END,0,0);
	}
}

bool CAutoFocus::Capture()
{
	ResetEvent(m_hCaptureEvent);
	m_CurVideoDevice->TrigerVideoData();
	int iCount=0;
	strMsg=L"Status: Auto  Capturing!";
	Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
	while(WaitForSingleObject(m_hCaptureEvent,300)==WAIT_TIMEOUT)
	{
		iCount++;
		if (iCount<=5)
		{
			ResetEvent(m_hCaptureEvent);
			m_CurVideoDevice->TrigerVideoData();
		}	
		else
		{
			strMsg=L"Status:   Capturing Time Out!";
			Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
			Sleep(3000);
			return false;
		}		
	}
	m_CaptureVideo.VnCount++;
	if (mAutoFocusParas->bRecord)
	{
		CString str;
		str.Format(L"\\FX%04d.jpg",m_CaptureVideo.VnCount);
		mAutoProcess->SavePic(&m_CaptureVideo,strSaveFile+str);
	}
	return bFocusOver;
}

void CAutoFocus::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
{
	if(m_CaptureVideo.Vsize!=pHeader->Vsize)
	{
		if(m_CaptureVideo.Vbuffer)
			delete(m_CaptureVideo.Vbuffer);
		m_CaptureVideo.Vbuffer=(LPBYTE)malloc(pHeader->Vsize);
		m_CaptureVideo.Vwidth=pHeader->Vwidth;
		m_CaptureVideo.Vheight=pHeader->Vheight;
		m_CaptureVideo.Vsize=pHeader->Vsize;
		m_CaptureVideo.VwBit=pHeader->VwBit;
	}
	memcpy(m_CaptureVideo.Vbuffer,pBuffer,m_CaptureVideo.Vsize);
	SetEvent(m_hCaptureEvent);
}

void CAutoFocus::Contrl0()
{
	double mMax=-1;
	int mIdx=0;
	GetMax(mPreDr,&mMax,&mIdx);
	mPredw=mPreDrP[mIdx];//回到最大值点
	mPreDirection=mIdx==0?1:-1;//记录方向
	if (fabs(mPredw)<0.001)//定位为零，一定反向
	{
		mPreDirection=1;
		iControl=1;
		return;
	}	
	if (mIdx==1)//构成三角形
		iControl=3;
	else
		iControl=1;
}
void CAutoFocus::Contrl1()
{
	double mMax=-1;
	int mIdx=0;
	GetMax(mCuDr,&mMax,&mIdx);
	mDirectionDr=mIdx==0?-mPreDirection:mPreDirection;
	iControl=2;
}
void CAutoFocus::Contrl2()
{
	double mMax=-1;
	int mIdx=0;
	GetMax(mCuContinuous,&mMax,&mIdx);
	if (mIdx!=0&&mIdx!=4)//构成三角形
	{
		/*mRdw=GetTriangleValue(mCuContinuous[mIdx-1],mCuContinuous[mIdx],
			mCuContinuous[mIdx+1],mDirectionDr,mCuStep)+
			-mDirectionDr*mCuStep*(mCuContinuous.size()-2-mIdx);*/
		mRdw=GetTriangleValue2(mCuContinuous[mIdx-1],mCuContinuous[mIdx],
			mCuContinuous[mIdx+1],mDirectionDr,mAutoFocusParas->mCuStep)+mCuContinuousP[mIdx];
		iControl=5;
	}
	else
	{
		//mRdw=-1*mDirectionDr*mCuStep*(4-mIdx);
		mRdw=mCuContinuousP[mIdx];
		iControl=3;
	}
}
void CAutoFocus::Contrl3()
{
	int iSize=mCuTriangle.size();
	vector<double> mVs;
	vector<float> mVsP;
	for (double idx=iSize-3;idx<iSize;idx++)
	{
		mVs.push_back(mCuTriangle[idx]);
		mVsP.push_back(mCuTriangleP[idx]);
	}
	double mMax=-1;
	int mIdx=0;
	GetMax(mVs,&mMax,&mIdx);
	if (mIdx!=0&&mIdx!=2)
	{
		//mRough=GetTriangleValue(mVs[mIdx-1],mVs[mIdx],
		//	mVs[mIdx+1],-1,mCuStep);

		mRough=GetTriangleValue2(mVs[mIdx-1],mVs[mIdx],
			mVs[mIdx+1],-1,mAutoFocusParas->mCuStep)+mVsP[mIdx];
		iControl=6;//退出
	}
	else if (iSize>=6)
	{
		vector<double> mVs2;
		vector<float> mVsP2;
		for (double idx=iSize-6;idx<iSize;idx++)
		{
			mVs2.push_back(mCuTriangle[idx]);
			mVsP2.push_back(mCuTriangleP[idx]);
		}
		int mIdx2=0;
		GetMax(mVs2,&mMax,&mIdx2);
		mRough=mVsP2[mIdx2];//回到最大值位置
		if (mIdx2!=0&&mIdx2!=5)
			iControl=5;//进入五
		else if (iSize>=45)//太多次了，也退出
			iControl=5;//进入五
	}
	else
	{
		//mRough=mCuStep*(2-mIdx);//返回到最大值位置
		mRough=mVsP[mIdx];//返回到最大值位置
	}
}
void CAutoFocus::Contrl4()
{

}
bool CAutoFocus::MoveContrl5(int Con5Direct)
{
	double* mFocusV=new double;
	*mFocusV=0;
	Quality_TYPE mfXiQua=ImgQuality_XiQuality;
	if (Con5Direct==0)//第一次进入
	{
		MoveZ(1,3*mAutoFocusParas->mXiStep);//先向下
		if (!Capture())//采集图像
		{
			bFocusOver=false;
			return false;
		}
		mAutoProcess->ProcessImg(&mfXiQua,mFocusV,&m_CaptureVideo);
		mXiContinuous1.push_back(*mFocusV);
		mXiContinuous1P.push_back(m_AxisZ->GetPosition());
		for (int i=0;i<6;i++)
		{
			MoveZ(-1,mAutoFocusParas->mXiStep);//移动位置
			if (!Capture())//采集图像
			{
				bFocusOver=false;
				return false;
			}
			mAutoProcess->ProcessImg(&mfXiQua,mFocusV,&m_CaptureVideo);
			mXiContinuous1.push_back(*mFocusV);
			mXiContinuous1P.push_back(m_AxisZ->GetPosition());
		}
	}
	else
	{
		if (!Capture())//采集图像
		{
			bFocusOver=false;
			return false;
		}
		mAutoProcess->ProcessImg(&mfXiQua,mFocusV,&m_CaptureVideo);
		mXiContinuous2.push_back(*mFocusV);
		mXiContinuous2P.push_back(m_AxisZ->GetPosition());
		for (int i=0;i<4;i++)
		{
			MoveZ(Con5Direct,mAutoFocusParas->mXiStep);//移动位置
			if (!Capture())//采集图像
			{
				bFocusOver=false;
				return false;
			}
			mAutoProcess->ProcessImg(&mfXiQua,mFocusV,&m_CaptureVideo);
			mXiContinuous2.push_back(*mFocusV);
			mXiContinuous2P.push_back(m_AxisZ->GetPosition());
		}
	}
}
void CAutoFocus::Contrl5()
{
	if (iCon5Direct==0)//第一次进入
	{
		double mMax=-1;
		int mIdx=0;
		GetMax(mXiContinuous1,&mMax,&mIdx);
		if (mIdx==0)
		{
			iCon5Direct=1;//向下走
			//mXdw=6*mXiStep;//回到第二位

			mXdw=mXiContinuous1P[1];//回到第二位
		}	
		else if (mIdx>=6)
		{
			iCon5Direct=-1;//向下走
			//mXdw=mXiStep;//回到倒数第二位

			mXdw=mXiContinuous1P[mXiContinuous1P.size()-2];//回到第二位
		}			
		else //构成三角形
		{
			//mXdw=GetTriangleValue(mXiContinuous1[mIdx-1],mXiContinuous1[mIdx],
			//	mXiContinuous1[mIdx+1],-1,mXiStep)+mXiStep*(mXiContinuous1.size()-2-mIdx);

			mXdw=GetTriangleValue2(mXiContinuous1[mIdx-1],mXiContinuous1[mIdx],
				mXiContinuous1[mIdx+1],-1,mAutoFocusParas->mXiStep)+mXiContinuous1P[mIdx];
			iControl=-1;
		}
	}
	else//第二次进入
	{
		double mMax=-1;
		int mIdx=0;
		GetMax(mXiContinuous2,&mMax,&mIdx);
		if (mIdx==0)
		{
			//mXdw=-4*mXiStep*iCon5Direct;//回到第二位
			mXdw=mXiContinuous2P[1];//回到第二位
			iControl=6;
		}	
		else if (mIdx>=4)
		{
			//mXdw=0;//不动，当前位置为最大
			mXdw=mXiContinuous2P[mXiContinuous2P.size()-1];//不动，当前位置为最大
			iControl=6;
		}			
		else //构成三角形
		{
			//mXdw=GetTriangleValue(mXiContinuous2[mIdx-1],mXiContinuous2[mIdx],
			//	mXiContinuous2[mIdx+1],iCon5Direct,mXiStep)+
			//	-iCon5Direct*mXiStep*(mXiContinuous2.size()-2-mIdx);

			mXdw=GetTriangleValue2(mXiContinuous2[mIdx-1],mXiContinuous2[mIdx],
				mXiContinuous2[mIdx+1],iCon5Direct,mAutoFocusParas->mXiStep)+mXiContinuous2P[mIdx];
			iControl=-1;
		}
	}
}
void CAutoFocus::Contrl6()
{
	int iSize=mXiTriangle.size();
	vector<double> mVs;
	//for (int idx=iSize-3;idx<iSize;idx++)
	//	mVs.push_back(mXiTriangle[idx]);
	vector<float> mVsP;
	for (double idx=iSize-3;idx<iSize;idx++)
	{
		mVs.push_back(mXiTriangle[idx]);
		mVsP.push_back(mXiTriangleP[idx]);
	}
	double mMax=-1;
	int mIdx=0;
	GetMax(mVs,&mMax,&mIdx);
	if (mIdx!=0&&mIdx!=2)
	{
		//mFocus=GetTriangleValue(mVs[mIdx-1],mVs[mIdx],
		//	mVs[mIdx+1],-1,mXiStep);

		mFocus=GetTriangleValue2(mVs[mIdx-1],mVs[mIdx],
			mVs[mIdx+1],-1,mAutoFocusParas->mXiStep)+mVsP[mIdx];
		iControl=-1;//退出
	}
	else if (iSize>=9)
	{
		//mFocus=mXiStep*(2-mIdx);//返回到最大值位置
		GetMax(mXiTriangle,&mMax,&mIdx);
		mFocus=mXiTriangleP[mIdx];
		iControl=-1;//退出
	}
	else
	{
		//mFocus=mXiStep*(2-mIdx);//返回到最大值位置
		mFocus=mVsP[mIdx];
	}
}
void CAutoFocus::Contrl7()
{

}
void CAutoFocus::GetMax(vector<double> mValues,double *Max,int *Idx)
{
	*Max=-1;
	for (int i=0;i<mValues.size();i++)
	{
		if (mValues[i]>*Max)
		{
			*Max=mValues[i];
			*Idx=i;
		}
	}
}
float CAutoFocus::GetTriangleValue(double mV1,double mV2,double mV3,int iDr,float mStep)
{
	float mReturnV=-1*iDr*mStep;//与原iDr方向反向
	if (mV2==mV1)//分母等值的话
		return mReturnV;
	float fD1=mV2-mV3,fD2=mV2-mV1;
	float mV=0;	
	int mD=fD1>fD2?-1:1;
	//int mD=fD1>fD2?1:-1;
	if (fD1>fD2)
	{
		mD=-1;//后者大于前者，反方向
		mV=(fD1-fD2)/mV2;
	}
	else
	{
		mD=1;
		mV=(fD2-fD1)/mV2;
	}
	if (fabs(mV)<1&&fabs(mV)>0)
		mReturnV+=mStep*mV*mD*iDr;
	return mReturnV;
}
float CAutoFocus::GetTriangleValue2(double mV1,double mV2,double mV3,int iDr,float mStep)
{
	return 0;
	float mReturnV=0;//与原iDr方向反向
	if (mV2==mV1)//分母等值的话
		return mReturnV;
	float mDv=mV3>mV1?mV3:mV1;
	if (mDv/mV2<0.6)//超过百分之60才做调整
		return mReturnV;
	float fD1=mV2-mV3,fD2=mV2-mV1;
	float mV=0;	
	int mD=fD1>fD2?-1:1;
	//int mD=fD1>fD2?1:-1;
	if (fD1>fD2)
	{
		mD=-1;//后者大于前者，反方向
		//mV=(fD1-fD2)/mV2;
		mV=fD2/mV2;
	}
	else
	{
		mD=1;
		//mV=(fD2-fD1)/mV2;
		mV=fD1/mV2;
	}
	if (fabs(mV)<1&&fabs(mV)>0)
		mReturnV+=mStep*mV*mD*iDr;
	return mReturnV;
}

void CAutoFocus::InitValue()
{
	mPreDr.clear();
	mPreDrP.clear();
	mCuDr.clear();
	mCuContinuous.clear();
	mCuTriangle.clear();
	mXiContinuous1.clear();
	mXiContinuous2.clear();
	mXiTriangle.clear();
	mCuDrP.clear();
	mCuContinuousP.clear();
	mCuTriangleP.clear();
	mXiContinuous1P.clear();
	mXiContinuous2P.clear();
	mXiTriangleP.clear();

	if (mAutoFocusParas->bRecord)
	{
		CreateDirectory(mAutoFocusParas->strSavePath,NULL);
		strSaveFile=mAutoFocusParas->strSavePath+L"\\"+GetTimeString();
		mAutoFocusParas->bRecord=BuildDirectory(strSaveFile);
		//CreateDirectory(strSaveFile,NULL);
	}
}

bool CAutoFocus::MoveZ(int iDirection,float fValue)
{
	if (m_AxisZ->GetPosition()+iDirection*fValue<=0)
	{
		return true;
	}
	int iTime=10000;
	strMsg.Format(L"Info: Moving %0.2f",fValue*iDirection);
	Notify(this,NOTIFY_MSGVIEW_Info,0,&strMsg);
	strMsg=L"Status:  Z Moving";
	Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
	ResetEvent(m_hMoveEvent);
	m_AxisZ->MoveRef(iDirection*fValue,mAutoFocusParas->iZSpeed);
	if (WaitForSingleObject(m_hMoveEvent,iTime)==WAIT_TIMEOUT)
	{
		strMsg=strMsg+L"  TIMEOUT!";
		Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
		Sleep(3000);
		return false;
	}
	Sleep(mAutoFocusParas->iSleepTime);//等待震荡时间
	return true;
}

bool CAutoFocus::MoveToZ(float fValue)
{
	if (m_AxisZ->GetPosition()==fValue)
		return true;
	int iTime=10000;
	strMsg=L"Status:  Z Moving To";
	Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
	ResetEvent(m_hMoveEvent);
	m_AxisZ->MoveTo(fValue,mAutoFocusParas->iZSpeed);
	if (WaitForSingleObject(m_hMoveEvent,iTime)==WAIT_TIMEOUT)
	{
		strMsg=strMsg+L"  TIMEOUT!";
		Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
		Sleep(3000);
		return false;
	}
	Sleep(mAutoFocusParas->iSleepTime);//等待20ms震荡时间
	return true;
}


//获取时间字符串
CString CAutoFocus::GetTimeString()
{
	struct tm *local;
	time_t t;
	t=time(NULL);
	local=localtime(&t);
	CString str;
	//str.Format(_T("%d%02d%02d-%d-%d-%d"),local->tm_year+1900,local->tm_mon+1,local->tm_mday,
	//	local->tm_hour,local->tm_min,local->tm_sec);
	str.Format(_T("%d%02d%02d\\%dh-%dm-%ds"),local->tm_year+1900,local->tm_mon+1,local->tm_mday,
		local->tm_hour,local->tm_min,local->tm_sec);
	return str;
}

void CAutoFocus::SaveFocusValue()
{
	CStdioFile file;
	file.Open(strSaveFile+L"\\Value.txt",CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite); 
	char* old_locale = _strdup(setlocale(LC_CTYPE,NULL) );
	setlocale( LC_CTYPE, "chs" );//设定区域为中文

	CString strVal=_T("以下是调焦过程的数据：\r\n");
	file.WriteString(strVal); 
	strVal.Format(L"方向步长：%.4f\r\n",mAutoFocusParas->mDrStep);
	file.WriteString(strVal); 
	strVal.Format(L"粗步长：%.4f\r\n",mAutoFocusParas->mCuStep);
	file.WriteString(strVal); 
	strVal.Format(L"细步长：%.4f\r\n",mAutoFocusParas->mXiStep);
	file.WriteString(strVal); 
	strVal.Format(L"粗尺度：%.1f\r\n",mAutoFocusParas->mCuScale);
	file.WriteString(strVal); 
	strVal.Format(L"细尺度：%.1f\r\n",mAutoFocusParas->mXiScale);
	file.WriteString(strVal); 

	strVal.Format(L"Control0预先判断：%d,%.4f\r\n",mPreDirection,mPredw);
	file.WriteString(strVal); 
	int iSaveCount=1;
	for (int i=0;i<mPreDr.size();i++)
	{
		strVal.Format(L"FX%04d，%.4f——>%.4f\r\n",iSaveCount,mPreDr[i],mPreDrP[i]);
		file.WriteString(strVal); 
		iSaveCount++;
	}

	strVal.Format(L"Control1方向判断：%d\r\n",mDirectionDr);
	file.WriteString(strVal); 
	for (int i=0;i<mCuDr.size();i++)
	{
		strVal.Format(L"FX%04d，%.4f——>%.4f\r\n",iSaveCount,mCuDr[i],mCuDrP[i]);
		file.WriteString(strVal); 
		iSaveCount++;
	}
	strVal.Format(L"Control2校准值：%.4f\r\n",mRdw);
	file.WriteString(strVal); 
	for (int i=0;i<mCuContinuous.size();i++)
	{
		strVal.Format(L"FX%04d，%.4f——>%.4f\r\n",iSaveCount,mCuContinuous[i],mCuContinuousP[i]);
		file.WriteString(strVal); 
		iSaveCount++;
	}
	strVal.Format(L"Control3校准值：%.4f\r\n",mRough);
	file.WriteString(strVal); 
	for (int i=0;i<mCuTriangle.size();i++)
	{
		strVal.Format(L"FX%04d，%.4f——>%.4f\r\n",iSaveCount,mCuTriangle[i],mCuTriangleP[i]);
		file.WriteString(strVal); 
		iSaveCount++;
	}
	strVal.Format(L"Control5校准值：%d,%.4f\r\n",iCon5Direct,mXdw);
	file.WriteString(strVal); 
	for (int i=0;i<mXiContinuous1.size();i++)
	{
		strVal.Format(L"FX%04d，%.4f——>%.4f\r\n",iSaveCount,mXiContinuous1[i],mXiContinuous1P[i]);
		file.WriteString(strVal); 
		iSaveCount++;
	}
	if (mXiContinuous2.size()>0)
	{
		strVal=L" 第二次：\r\n";
		file.WriteString(strVal); 
		for (int i=0;i<mXiContinuous2.size();i++)
		{
			strVal.Format(L"FX%04d，%.4f——>%.4f\r\n",iSaveCount,mXiContinuous2[i],mXiContinuous2P[i]);
			file.WriteString(strVal); 
			iSaveCount++;
		}
	}
	strVal.Format(L"Control6校准值：%.4f\r\n",mFocus);
	file.WriteString(strVal); 
	for (int i=0;i<mXiTriangle.size();i++)
	{
		strVal.Format(L"FX%04d，%.4f——>%.4f\r\n",iSaveCount,mXiTriangle[i],mXiTriangleP[i]);
		file.WriteString(strVal); 
		iSaveCount++;
	}


	setlocale( LC_CTYPE, old_locale );
	free(old_locale);//还原区域向前设定设定
	file.Close();

}

//检查输入的目录是否存在，如不存在，则创建新目录
bool CAutoFocus::BuildDirectory(CString strPath)
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

