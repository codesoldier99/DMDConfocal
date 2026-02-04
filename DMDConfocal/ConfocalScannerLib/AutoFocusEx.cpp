#include "StdAfx.h"
#include "AutoFocusEx.h"

CAutoFocusEx::CAutoFocusEx(void)
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

CAutoFocusEx::~CAutoFocusEx(void)
{
}

bool CAutoFocusEx::InitPlugin(LPVOID p_Param, LPVOID p_Param2)
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

bool CAutoFocusEx::UnInitPlugin()
{
	bFocusOver=false;
	//if (m_CaptureVideo.Vbuffer)
	//	delete(m_CaptureVideo.Vbuffer);
	return true;
}

bool CAutoFocusEx::Stop()
{
	bFocusOver=false;
	return true;
}

int CAutoFocusEx::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
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

bool CAutoFocusEx::Set(void* WParas)
{
	if (WParas!=0)
	{
		mAutoFocusParas=(AutoFocusParas*)WParas;
		mAutoProcess->InitProcess(WParas);
		return true;
	}
	return false;
}

bool CAutoFocusEx::Start(void* vParas)
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
	iControl=1;
	m_ConfocalCore->GetRenderChain()->push_back(this);//加入渲染

	ResetEvent(m_hMoveEvent);
	ResetEvent(m_hCaptureEvent);
	bFocusOver=true;

	m_CurVideoDevice->SetTrigerMode(TRIGER_SOFT);
	m_ScanThread = CreateThread(NULL, 0, AutoFocusProcEx, this, 0, 0);//这个开线程
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

void CAutoFocusEx::WaitStart()
{
	m_CaptureVideo.VnCount=0;//表示当前编号
	iControl=0;
	m_ConfocalCore->GetRenderChain()->push_back(this);//加入渲染
	ResetEvent(m_hMoveEvent);
	ResetEvent(m_hCaptureEvent);
	bFocusOver=true;
	m_ScanThread = CreateThread(NULL, 0, AutoFocusProcEx, this, 0, 0);//这个开线程
	HCorePanel* mDlg=m_ConfocalCore->GetMsgPanel(MSG_TIME);
	this->Attach(mDlg);
	WaitForSingleObject(m_ScanThread,INFINITE);//等待线程结束
	this->Detach(mDlg);
	bFocusOver=false;//退出线程
	mAutoProcess->UnInitProcess();
}

DWORD WINAPI AutoFocusProcEx( LPVOID lp )
{
	((CAutoFocusEx*)lp)->ScanAuto();
	return 0;
}

void CAutoFocusEx::ScanAuto()
{
	InitValue();
	float mfV=0;
	double* mFocusV=new double;
	*mFocusV=0;
	Quality_TYPE mfCuQua=ImgQuality_CuQuality;
	Quality_TYPE mfXiQua=ImgQuality_XiQuality;
	int mReMove=0;
	while(bFocusOver)
	{
		switch(iControl)
		{
			case 0://细调
				mReMove=mAutoFocusParas->ScanCount/2;
				mfV=mAutoFocusParas->mXiStep;
				MoveZ(1,(mReMove+0.0f)*mfV);//第一步先向下移动
				iDirect=-1;
				while(iControl==0)
				{
					for (int i=0;i<mAutoFocusParas->ScanCount;i++)
					{				
						if (!Capture())//采集图像
						{
							bFocusOver=false;
							break;
						}	
						mAutoProcess->ProcessImg(&mfXiQua,mFocusV,&m_CaptureVideo);
						mXiValue.push_back(*mFocusV);
						mXiPt.push_back(m_AxisZ->GetPosition());
						if (i!=mAutoFocusParas->ScanCount-1)//不是最后一次才移动
							MoveZ(iDirect,mfV);
					}
					if (!bFocusOver)
						break;
					Contrl0();
					MoveToZ(mXidw[mXidw.size()-1]);
				}							
				break;
			case 1:
				if (mAutoFocusParas->PreCount<=0)
				{
					iControl=0;
					break;
				}
				mReMove=mAutoFocusParas->PreCount/2;
				mfV=mAutoFocusParas->mCuStep;
				MoveZ(1,(mReMove+0.0f)*mfV);//第一步先向下移动
				iDirect=-1;
				while(iControl==1)
				{
					for (int i=0;i<mAutoFocusParas->PreCount;i++)
					{
						if (!Capture())//采集图像
						{
							bFocusOver=false;
							break;
						}
						mAutoProcess->ProcessImg(&mfCuQua,mFocusV,&m_CaptureVideo);
						mCuValue.push_back(*mFocusV);
						mCuPt.push_back(m_AxisZ->GetPosition());	
						if (i!=mAutoFocusParas->PreCount-1)//最后一次不移动
							MoveZ(iDirect,mfV);
					}
					if (!bFocusOver)
						break;
					Contrl1();
					MoveToZ(mCudw[mCudw.size()-1]);
				}		
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

void CAutoFocusEx::ScanAuto2()
{
	InitValue();
	float mfV=mAutoFocusParas->mXiStep;
	double* mFocusV=new double;
	*mFocusV=0;
	Quality_TYPE mfCuQua=ImgQuality_CuQuality;
	Quality_TYPE mfXiQua=ImgQuality_XiQuality;
	int mReMove=mAutoFocusParas->ScanCount/2;
	MoveZ(1,(mReMove+0.0f)*mfV);//第一步先向下移动
	iDirect=-1;
	while(bFocusOver)
	{
		for (int i=0;i<mAutoFocusParas->ScanCount;i++)
		{				
			if (!Capture())//采集图像
			{
				bFocusOver=false;
				break;
			}	
			mAutoProcess->ProcessImg(&mfXiQua,mFocusV,&m_CaptureVideo);
			mFocusValue.push_back(*mFocusV);
			mFocusPt.push_back(m_AxisZ->GetPosition());
			if (i!=mAutoFocusParas->ScanCount-1)
				MoveZ(iDirect,mfV);//向上移动
		}
		if (!Contrl())
		{
			bFocusOver=false;
			MoveToZ(mFocusdw[mFocusdw.size()-1]);
			break;
		}
		MoveToZ(mFocusdw[mFocusdw.size()-1]);
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

bool CAutoFocusEx::Contrl()
{
	if (mFocusValue[mFocusValue.size()-1]<mFocusValue[0])//反向
	{
		iDirect=-1*iDirect;
		std::reverse(mFocusValue.begin(),mFocusValue.end());
		std::reverse(mFocusPt.begin(),mFocusPt.end());
		Quality_TYPE mfQua=ImgQuality_XiInvert;
		mAutoProcess->ProcessImg(&mfQua,0,0);//反向数据记录
	}
	double mMax=-1;
	int mIdx=0;
	GetMax(mFocusValue,&mMax,&mIdx,0);
	mFocusdw.push_back(mFocusPt[mIdx]+mAutoFocusParas->mXiStep*iDirect);//移动到另外的下一个位置
	if (mIdx!=mFocusValue.size()-1)//如果最大值不在最后一个位置，则结束
	{
		Quality_TYPE mfQua=ImgQuality_XiEnd;
		double mXiIdx=0.0;
		mAutoProcess->ProcessImg(&mfQua,&mXiIdx,NULL);
		int mGetIdx=mXiIdx;
		mXiIdx=mXiIdx-mGetIdx;
		if (mGetIdx>mFocusPt.size()||mGetIdx<0||abs(mGetIdx-mIdx)>1)
			mGetIdx=mIdx;
		mFocusdw.push_back(mFocusPt[mGetIdx]+mXiIdx*mAutoFocusParas->mXiStep*iDirect);
		return false;
	}		
	else
		return true;
}

void CAutoFocusEx::Contrl0()
{
	if (mXiValue[mXiValue.size()-1]<mXiValue[0])//反向
	{
		iDirect=-1*iDirect;
		std::reverse(mXiValue.begin(),mXiValue.end());
		std::reverse(mXiPt.begin(),mXiPt.end());
		Quality_TYPE mfQua=ImgQuality_XiInvert;
		mAutoProcess->ProcessImg(&mfQua,0,0);//反向数据记录
	}
	double mMax=-1;
	int mIdx=0;
	GetMax(mXiValue,&mMax,&mIdx,0);
	mXidw.push_back(mXiPt[mIdx]+mAutoFocusParas->mXiStep*iDirect);//移动到另外的下一个位置
	if (mIdx!=mXiValue.size()-1)//如果最大值不在最后一个位置，则结束
	{
		Quality_TYPE mfQua=ImgQuality_XiEnd;
		double mXiIdx=0.0;
		mAutoProcess->ProcessImg(&mfQua,&mXiIdx,NULL);
		int mGetIdx=mXiIdx;
		mXiIdx=mXiIdx-mGetIdx;
		if (mGetIdx>mXiPt.size()||mGetIdx<0||abs(mGetIdx-mIdx)>1)
			mGetIdx=mIdx;
		mXidw.push_back(mXiPt[mGetIdx]+mXiIdx*mAutoFocusParas->mXiStep*iDirect);
		iControl=3;
	}		
	else
		iControl=0;

	//double mMax=-1;
	//int mIdx=0;
	//int iStart=mXiValue.size()-mAutoFocusParas->ScanCount;
	//GetMax(mXiValue,&mMax,&mIdx,iStart);
	//mXidw.push_back(mXiPt[mIdx]);//记录最大值点
	//if (mIdx==iStart||mIdx==mXiValue.size()-1)//没构成三角形
	//	iControl=1;//粗调
	//else
	//	iControl=3;//结束
}

void CAutoFocusEx::Contrl1()
{
	if (mCuValue[mCuValue.size()-1]<mCuValue[0])//反向
	{
		iDirect=-1*iDirect;
		std::reverse(mCuValue.begin(),mCuValue.end());
		std::reverse(mCuPt.begin(),mCuPt.end());
		Quality_TYPE mfQua=ImgQuality_CuInvert;
		mAutoProcess->ProcessImg(&mfQua,0,0);//反向数据记录
	}
	double mMax=-1;
	int mIdx=0;
	GetMax(mCuValue,&mMax,&mIdx,0);
	mCudw.push_back(mCuPt[mIdx]+mAutoFocusParas->mCuStep*iDirect);//移动到另外的下一个位置
	if (mIdx!=mCuValue.size()-1)//如果最大值不在最后一个位置，则结束
	{
		Quality_TYPE mfQua=ImgQuality_CuEnd;
		double mCuIdx=0.0;
		mAutoProcess->ProcessImg(&mfQua,&mCuIdx,NULL);
		int mGetIdx=mCuIdx;
		mCuIdx=mCuIdx-mGetIdx;
		if (mGetIdx>mCuPt.size()||mGetIdx<0||abs(mGetIdx-mIdx)>1)
			mGetIdx=mIdx;
		mCudw.push_back(mCuPt[mGetIdx]+mCuIdx*mAutoFocusParas->mCuStep*iDirect);
		iControl=0;
	}		
	else
		iControl=1;

	//double mMax=-1;
	//int mIdx=0;
	//int iStart=mXiValue.size()-mAutoFocusParas->ScanCount;
	//GetMax(mCuValue,&mMax,&mIdx,iStart);
	//mCudw.push_back(mCuPt[mIdx]);

	//if (mIdx==iStart||mIdx==mCuValue.size()-1)//没构成三角形
	//	iControl=1;//粗调
	//else
	//	iControl=0;//返回细调
}

bool CAutoFocusEx::Capture()
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

void CAutoFocusEx::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
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

void CAutoFocusEx::InitValue()
{
	mXiValue.clear();
	mXiPt.clear();
	mXidw.clear();

	mCuValue.clear();
	mCuPt.clear();
	mCudw.clear();

	mFocusValue.clear();
	mFocusPt.clear();
	mFocusdw.clear();

	if (mAutoFocusParas->bRecord)
	{
		CreateDirectory(mAutoFocusParas->strSavePath,NULL);
		strSaveFile=mAutoFocusParas->strSavePath+L"\\"+GetTimeString();
		mAutoFocusParas->bRecord=BuildDirectory(strSaveFile);
		//CreateDirectory(strSaveFile,NULL);
	}
}

bool CAutoFocusEx::MoveZ(int iDirection,float fValue)
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

bool CAutoFocusEx::MoveToZ(float fValue)
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

void CAutoFocusEx::GetMax(vector<double> mValues,double *Max,int *Idx,int iStart)
{
	*Max=-1;
	for (int i=iStart;i<mValues.size();i++)
	{
		if (mValues[i]>*Max)
		{
			*Max=mValues[i];
			*Idx=i;
		}
	}
}

//获取时间字符串
CString CAutoFocusEx::GetTimeString()
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

void CAutoFocusEx::SaveFocusValue()
{
	CStdioFile file;
	file.Open(strSaveFile+L"\\Value.txt",CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite); 
	char* old_locale = _strdup(setlocale(LC_CTYPE,NULL) );
	setlocale( LC_CTYPE, "chs" );//设定区域为中文

	CString strVal=_T("以下是调焦过程的数据：\r\n");
	file.WriteString(strVal);  
	strVal.Format(L"粗步长：%.4f\r\n",mAutoFocusParas->mCuStep);
	file.WriteString(strVal); 
	strVal.Format(L"细步长：%.4f\r\n",mAutoFocusParas->mXiStep);
	file.WriteString(strVal); 
	strVal.Format(L"粗尺度：%.1f\r\n",mAutoFocusParas->mCuScale);
	file.WriteString(strVal); 
	strVal.Format(L"细尺度：%.1f\r\n",mAutoFocusParas->mXiScale);
	file.WriteString(strVal); 
	strVal.Format(L"最大位置：%.1f\r\n",mAutoFocusParas->UpLimit);
	file.WriteString(strVal); 
	strVal.Format(L"最小位置：%.1f\r\n",mAutoFocusParas->DownLimit);
	file.WriteString(strVal); 
	strVal.Format(L"扫描数：%d\r\n",mAutoFocusParas->ScanCount);
	file.WriteString(strVal); 

	int iSaveCount=1;
	/*for (int i=0;i<mFocusdw.size();i++)
	{
		strVal.Format(L"第%d次细定位：%.4f\r\n",i+1,mFocusdw[i]);
		file.WriteString(strVal); 
	}
	for (int i=0;i<mFocusValue.size();i++)
	{
		strVal.Format(L"FX%04d，%.4f——>%.4f\r\n",iSaveCount,mFocusValue[i],mFocusPt[i]);
		file.WriteString(strVal); 
		if (iSaveCount%mAutoFocusParas->ScanCount==0)
			file.WriteString(L"\r\n"); 
		iSaveCount++;
	}*/

	for (int i=0;i<mCudw.size();i++)
	{
		strVal.Format(L"第%d次粗定位：%.4f\r\n",i+1,mCudw[i]);
		file.WriteString(strVal); 
	}
	for (int i=0;i<mCuValue.size();i++)
	{
		strVal.Format(L"FX%04d，%.4f——>%.4f\r\n",iSaveCount,mCuValue[i],mCuPt[i]);
		file.WriteString(strVal); 
		if (iSaveCount%mAutoFocusParas->PreCount==0)
			file.WriteString(L"\r\n"); 
		iSaveCount++;
	}

	for (int i=0;i<mXidw.size();i++)
	{
		strVal.Format(L"第%d次细定位：%.4f\r\n",i+1,mXidw[i]);
		file.WriteString(strVal); 
	}
	for (int i=0;i<mXiValue.size();i++)
	{
		strVal.Format(L"FX%04d，%.4f——>%.4f\r\n",iSaveCount,mXiValue[i],mXiPt[i]);
		file.WriteString(strVal); 
		if ((i+1)%mAutoFocusParas->ScanCount==0)
			file.WriteString(L"\r\n"); 
		iSaveCount++;
	}

	setlocale( LC_CTYPE, old_locale );
	free(old_locale);//还原区域向前设定设定
	file.Close();

}

//检查输入的目录是否存在，如不存在，则创建新目录
bool CAutoFocusEx::BuildDirectory(CString strPath)
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



