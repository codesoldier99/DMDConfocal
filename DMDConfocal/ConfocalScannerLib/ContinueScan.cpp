#include "StdAfx.h"
#include "ContinueScan.h"


ContinueScan::ContinueScan(void)
{
	m_CaptureVideo.Vsize=0;//表示没有东西
	m_CaptureVideo.Vbuffer=0;
	m_hCaptureEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	iSaveCount=0;
}


ContinueScan::~ContinueScan(void)
{
}

bool ContinueScan::InitPlugin(LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	if(m_ConfocalCore)
	{
		m_CurVideoDevice=m_ConfocalCore->GetCurVideo();//获取相机
		m_ReBuildPro=m_ConfocalCore->GetCoreProcess(PROCESS_3DReBuild);
		return true;
	}
	return false;
}

bool ContinueScan::UnInitPlugin()
{
	bScan=false;
	return true;
}

bool ContinueScan::Stop()
{
	bScan=false;
	return true;
}

bool ContinueScan::Set(void* WParas)
{
	if (WParas==0)
		return false;
	//bool bConfocal=*(bool*)vParas;
	iSaveCount=*(int*)WParas;
	strSavsPath=m_ConfocalCore->GetConfigure()->GetString(L"3DRebuild",L"strPath",L"Confocal_Scaner");
	struct tm *local;
	time_t t;
	t=time(NULL);
	local=localtime(&t);
	CString strMV;
	strMV.Format(_T("\\%d%02d%02dContinue\\%02d-%02d-%02d"),local->tm_year+1900,local->tm_mon+1,local->tm_mday,
		local->tm_hour,local->tm_min,local->tm_sec);
	strSavsPath=strSavsPath+strMV;
	BuildDirectory(strSavsPath);
	//CreateDirectory(strSavsPath,NULL);
	return true;
}

bool ContinueScan::Start(void* vParas)//Start(bool bConfocal)
{
	m_CaptureVideo.VcaptureTime=0;//表示当前编号
	bConfocalMode=false;//lbConfocal;
	if (!bConfocalMode)
	{
		m_CurVideoDevice->SetTrigerMode(TRIGER_SOFT);
		m_ConfocalCore->GetRenderChain()->push_back(this);//加入渲染
	}
	else
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->push_back(this);
	ResetEvent(m_hCaptureEvent);
	m_ScanThread = CreateThread(NULL, 0, SaveScanProc, this, 0, 0);//这个开线程
	bScan=true;
	HCorePanel* mDlg=m_ConfocalCore->GetMsgPanel(MSG_TIME);
	mDlg->InitPanel(0);
	this->Attach(mDlg);
	m_ConfocalCore->GetDockablePanel(DOCKPANEL_STAGECTL)
		->GetCWnd()->PostMessage(0x118);
	((CDialogEx*)mDlg->GetCWnd())->DoModal();//->ShowWindow(SW_SHOW);
	this->Detach(mDlg);
	WaitForSingleObject(m_ScanThread,2000);
	m_ConfocalCore->GetRenderChain()->remove(this);//删除视频
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->remove(this);
	bScan=false;//退出线程
	return true;
}

void ContinueScan::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
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

DWORD WINAPI SaveScanProc( LPVOID lp )
{
	((ContinueScan*)lp)->DoSaveScan();
	return 0;
}

void ContinueScan::DoSaveScan()
{
	int iCurrentCount=0;
	while(bScan)
	{
		if (iCurrentCount<=iSaveCount)
		{
			strMsg.Format(L"Capturing Image:%d/%d",iCurrentCount,iSaveCount);
			Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
			if (!Capture())//抓图+存图
			{
				bScan=false;
				break;
			}
		}
		else 
		{
			bScan=false;
			break;
		}
		iCurrentCount++;
	}
	m_ConfocalCore->GetRenderChain()->remove(this);//删除视频
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->remove(this);
	if (!bConfocalMode)
	{
		m_CurVideoDevice->SetTrigerMode(TRIGER_INTERAL);
	}
	if (iCurrentCount>=iSaveCount)//代表成功完成了重建
		Notify(this,NOTIFY_MSGVIEW_END,0,0);
}

bool ContinueScan::Capture()
{
	if (!bConfocalMode)
		m_CurVideoDevice->TrigerVideoData();
	int iCount=0;
	while(WaitForSingleObject(m_hCaptureEvent,1300)==WAIT_TIMEOUT)
	{
		iCount++;
		if (iCount<=3)
		{
			if (!bConfocalMode)
				m_CurVideoDevice->TrigerVideoData();
		}	
		else
		{
			strMsg=L"Status:   Capturing Time Out!";
			Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
			return false;
		}		
	}
	ResetEvent(m_hCaptureEvent);
	m_CaptureVideo.VcaptureTime++;//编号加一
	CString t_Name;
	t_Name.Format(L"%s\\FX%04d.jpg",strSavsPath,m_CaptureVideo.VcaptureTime);
	m_ReBuildPro->SavePic(&m_CaptureVideo,t_Name);
	return true;
}

bool ContinueScan::BuildDirectory(CString strPath)
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
