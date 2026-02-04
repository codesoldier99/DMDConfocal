#include "StdAfx.h"
#include "C3DScanner.h"
#include "HGloableFunction.h"

C3DScanner::C3DScanner(void)
{
	m_AxisZ=0;
	m_AxisPIZ=0;
	m_ConfocalCore=0;
	m_CaptureVideo.Vsize=0;//表示没有东西
	m_CaptureVideo.Vbuffer=0;
	m_hMoveEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_hCaptureEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_ScanThread=0;
	bScan=false;
	bSaveImg=false;
	bConfocalMode=false;
	strMsg=L"";
}

C3DScanner::~C3DScanner(void)
{

}

bool C3DScanner::InitPlugin(LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	//初始化载物台
	if(m_ConfocalCore)
	{
    if (m_ConfocalCore->GetGearBox())
    {
		  m_AxisZ=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Z);
    }
		if (m_AxisZ)
			m_AxisZ->GetSubject()->Attach(this);
    if (m_ConfocalCore->GetGearBox())
    {
		  m_AxisPIZ=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_PIZ);
    }
	}
	m_ConfigureFile=m_ConfocalCore->GetConfigure();//初始化其它参数
	m_CurVideoDevice=m_ConfocalCore->GetCurVideo();//获取相机
	m_ReBuildPro=m_ConfocalCore->GetCoreProcess(PROCESS_3DReBuild);
	if (m_ReBuildPro)
	{
		m_ReBuildPro->Attach(this);
	}
	return true;
}

bool C3DScanner::UnInitPlugin()
{
	bScan=false;
	return true;
}

bool C3DScanner::Stop()
{
	bScan=false;
	return true;
}

bool C3DScanner::Set(void* WParas)
{
	if (WParas==0)
		return false;
	ReBuildParas* mV=(ReBuildParas*)WParas;
	m_ReBuildPro->InitProcess(mV);
	int p_Count=(int)((mV->fDownPos-mV->fUpPos)/mV->fStepPos)+1;
	m_ScanStart=mV->fUpPos;
	m_ScanStep=mV->fStepPos;
	m_ScanStop=mV->fDownPos;
	m_ScanCount=p_Count;
	m_ScanSpeed=mV->fSpeed;
	bSaveImg=mV->bSaveImg;
	iSleep=mV->iSleep;
	bPIZ=mV->bPIZ;
	strSavsPath=m_ConfocalCore->GetConfigure()->GetString(L"3DRebuild",L"strPath",L"Confocal_Scaner");
	struct tm *local;
	time_t t;
	t=time(NULL);
	local=localtime(&t);
	if (bSaveImg)
	{
		CString strMV;
		strMV.Format(_T("\\%d%02d%02d\\%d-%d-%d"),local->tm_year+1900,local->tm_mon+1,local->tm_mday,
			local->tm_hour,local->tm_min,local->tm_sec);
		strSavsPath=strSavsPath+strMV;//+L"";
		BuildDirectory(strSavsPath);
		//CreateDirectory(strSavsPath,NULL);
	}
	return true;
}

bool C3DScanner::Start(void* vParas)//Start(bool bConfocal)
{
	if (vParas==0)
		return false;
	bool bConfocal=*(bool*)vParas;
	m_ScanCountRec=0;
	m_Axis_Next=m_ScanStart;	
	m_CaptureVideo.VcaptureTime=0;//表示当前编号
	m_CaptureVideo.VnCount=m_ScanCount;//表示共有多少张
	bConfocalMode=bConfocal;
	if (!bConfocal)
	{
		m_CurVideoDevice->SetTrigerMode(TRIGER_SOFT);
		m_ConfocalCore->GetRenderChain()->push_back(this);//加入渲染
	}
	else
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->push_back(this);
	ResetEvent(m_hMoveEvent);
	ResetEvent(m_hCaptureEvent);
	if (!bPIZ)
		m_ScanThread = CreateThread(NULL, 0, ScanProc, this, 0, 0);//这个开线程
	else
		m_ScanThread = CreateThread(NULL, 0, ScanPIZProc, this, 0, 0);
	bScan=true;
	//HINSTANCE exe_hInstance = GetModuleHandle(NULL);
	//HINSTANCE dll_hInstance = GetModuleHandle(L"ConfocalUILib.dll");
	//AfxSetResourceHandle(dll_hInstance); //切换状态
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
	//AfxSetResourceHandle(exe_hInstance); //恢复状态
	bScan=false;//退出线程
}

void C3DScanner::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
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
	if (!pBuffer)
		return;
	memcpy(m_CaptureVideo.Vbuffer,pBuffer,m_CaptureVideo.Vsize);

	SetEvent(m_hCaptureEvent);
}

int C3DScanner::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if (pSubject==m_ConfocalCore->GetCoreProcess(PROCESS_3DReBuild))
	{
		LPBYTE pBuffer=(LPBYTE)pParam;
		if(ID==NOTIFY_RESULT_HEIGHT)
		{
			HSaveBmp(pBuffer,wParam,wParam,8, HGetStartPath() + L"\\EpiResult\\ResultHeight.bmp");
		}
		else if(ID==NOTIFY_RESULT_PIC)
		{
			HSaveBmp(pBuffer,wParam,wParam,fParam, HGetStartPath() + L"\\EpiResult\\ResultPicture.bmp");
		}
		else if (ID==NOTIFY_RESULT_3D)
		{
			Notify(this,PANEL_VIEW_3DCMOPLETE,0,0);
		}
	}
	else if (pSubject==m_AxisZ->GetSubject())
	{
		if(ID==NOTIFY_AXIS_MOVED||ID==NOTIFY_AXIS_POSITION)
			SetEvent(m_hMoveEvent);
	}

	return 0;
}

DWORD WINAPI ScanProc( LPVOID lp )
{
	((C3DScanner*)lp)->ScanImage();
	return 0;
}

void C3DScanner::ScanImage()
{
	m_Axis_Next=m_ScanStart;//起始位置
	//int iFirst=0;
	while(bScan)
	{
		m_Axis_Pos=m_AxisZ->GetPosition();//得到当前位置
		if (abs(m_Axis_Pos-m_Axis_Next)>0.001)
		{
			m_AxisZ->MoveTo(m_Axis_Next,m_ScanSpeed);//移动
			strMsg=L"Status:   Moving!";
			Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
			if (WaitForSingleObject(m_hMoveEvent,10000)==WAIT_TIMEOUT)//等待载物台移动到位
			{
				strMsg=L"Status:   Moving Time Out!";
				Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
				bScan=false;
				break;
			}
			strMsg.Format(L"Process: %d/%d ",m_ScanCountRec+1,m_ScanCount);
			Notify(this,NOTIFY_MSGVIEW_Info,0,&strMsg);
			ResetEvent(m_hMoveEvent);
		}
		Sleep(iSleep);//等待震荡时间
		if (!Capture())//抓图+融合
		{
			bScan=false;
			break;
		}
		m_Axis_Next+=m_ScanStep;//下一位
		m_ScanCountRec++;
		if (m_ScanCountRec>=m_ScanCount)
		{
			bScan=false;
			break;
		}	
	}
	m_AxisZ->MoveTo((m_ScanStart+m_ScanStop)/2);//回中间位置
	m_ConfocalCore->GetRenderChain()->remove(this);//删除视频
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->remove(this);
	//m_ReBuildPro->InitProcess();//清理内存？！？
	if (!bConfocalMode)
	{
		m_CurVideoDevice->SetTrigerMode(TRIGER_INTERAL);
	}
	if (m_ScanCountRec>=m_ScanCount)//代表成功完成了重建
		Notify(this,NOTIFY_MSGVIEW_END,0,0);
}

DWORD WINAPI ScanPIZProc( LPVOID lp )
{
	((C3DScanner*)lp)->ScanPIZ3D();
	return 0;
}

void C3DScanner::ScanPIZ3D()
{
	m_Axis_Next=m_ScanStart;//起始位置
	while(bScan)
	{
		//m_Axis_Pos=m_AxisPIZ->GetPosition();//得到当前位置
		//if (abs(m_Axis_Pos-m_Axis_Next)>0.001)
		//{
		//	m_AxisPIZ->MoveTo(m_Axis_Next,m_ScanSpeed);//移动
		//	strMsg=L"Status:   Moving!";
		//	Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
		//}
		if (m_Axis_Next==m_ScanStart)
			m_AxisPIZ->MoveTo(m_Axis_Next);
		else
			m_AxisPIZ->MoveRef(m_ScanStep);
		strMsg=L"Status:   Moving!";
		Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);

		strMsg.Format(L"Process: %d/%d ",m_ScanCountRec+1,m_ScanCount);
		Notify(this,NOTIFY_MSGVIEW_Info,0,&strMsg);

		Sleep(iSleep);//等待震荡时间
		if (!Capture())//抓图+融合
		{
			bScan=false;
			break;
		}
		Notify(this,NOTIFY_PIZChange);//通知更新PI电机位置

		m_Axis_Next+=m_ScanStep;//下一位
		m_ScanCountRec++;
		if (m_ScanCountRec>=m_ScanCount)
		{
			bScan=false;
			break;
		}	
	}
	m_AxisPIZ->MoveTo((m_ScanStart+m_ScanStop)/2);//回中间位置
	m_ConfocalCore->GetRenderChain()->remove(this);//删除视频
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->remove(this);
	//m_ReBuildPro->InitProcess();//重新初始化？释放资源？
	if (!bConfocalMode)
	{
		m_CurVideoDevice->SetTrigerMode(TRIGER_INTERAL);
	}
	if (m_ScanCountRec>=m_ScanCount)//代表成功完成了重建
		Notify(this,NOTIFY_MSGVIEW_END,0,0);
}

bool C3DScanner::Capture()
{
	if (!bConfocalMode)
		m_CurVideoDevice->TrigerVideoData();
	int iCount=0;
	strMsg=L"Status:   Capturing!";
	Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
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
			//AfxMessageBox(L"图像采集超时！");
			strMsg=L"Status:   Capturing Time Out!";
			Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
			return false;
			//break;
		}		
	}
	ResetEvent(m_hCaptureEvent);
	strMsg=L"Status:   Processing!";
	Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
	m_ReBuildPro->GetVideoRender()->Renderer(&m_CaptureVideo,m_CaptureVideo.Vbuffer);//输出到3d库中
	m_CaptureVideo.VcaptureTime++;//编号加一
	if (!bSaveImg)
		return true;
	CString t_Name;

	//strV.Format(L"PI Pos: %g mm\n",m_AxisPIZ->GetPosition());

	if (bPIZ)
	{
		float mV=m_AxisPIZ->GetPosition();
		mV=mV*1000;
		t_Name.Format(L"%s\\FX%04d-%.3fum.jpg",strSavsPath,m_CaptureVideo.VcaptureTime,mV);
	}	
	else
		t_Name.Format(L"%s\\FX%04d.jpg",strSavsPath,m_CaptureVideo.VcaptureTime);
	m_ReBuildPro->SavePic(&m_CaptureVideo,t_Name);
	return true;
}

bool C3DScanner::BuildDirectory(CString strPath)
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
