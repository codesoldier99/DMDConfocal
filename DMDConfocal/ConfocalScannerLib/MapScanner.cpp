#include "StdAfx.h"
#include "MapScanner.h"


CMapScanner::CMapScanner(void)
{
	m_ConfocalCore=0;
	m_CaptureVideo.Vbuffer=0;
	m_CaptureVideo.Vsize=0;

	m_hMoveEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_hCaptureEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_ScanThread=0;

  m_AxisX = 0;
  m_AxisY = 0;
  m_AxisZ = 0;
}

CMapScanner::~CMapScanner(void)
{
	if (m_CaptureVideo.Vbuffer)
		delete(m_CaptureVideo.Vbuffer);
}

bool CMapScanner::InitPlugin(LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	//初始化载物台
	if(m_ConfocalCore)
	{
    if (m_ConfocalCore->GetGearBox())
    {
		  m_AxisX=m_ConfocalCore->GetGearBox()->GetAxis(AXIS_X);
    }
		if (m_AxisX)
			m_AxisX->GetSubject()->Attach(this);
    if (m_ConfocalCore->GetGearBox())
    {
      m_AxisY = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Y);
    }
		if (m_AxisY)
			m_AxisY->GetSubject()->Attach(this);
    if (m_ConfocalCore->GetGearBox())
    {
      m_AxisZ = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Z);
    }
		if (m_AxisZ)
			m_AxisZ->GetSubject()->Attach(this);
	}
	m_Conf=m_ConfocalCore->GetConfigure();
	m_CurVideoDevice=m_ConfocalCore->GetCurVideo();
	mMapProcess=m_ConfocalCore->GetCoreProcess(PROCESS_MapBuild);
  if (mMapProcess)
  {
	  mMapProcess->Attach(this);
  }
	//下面来一个测试的
	//m_MapMerge.InitPlugin(m_ConfocalCore);
	return true;
}

bool CMapScanner::Stop()
{
	bScan=false;
	return true;
}

bool CMapScanner::Set(void* WParas)
{
	if (WParas==0)
		return false;

	mMapScanParas=(MapBuildParas*)WParas;
	mMapProcess->InitProcess(WParas);//已经修改了起始点
	m_MoveStep=(float)mMapScanParas->ImageROISize*mMapScanParas->PixelSize;
	//计算移动的次数
	m_XCount=mMapScanParas->RowCount;
	m_YCount=mMapScanParas->ColCount;
	m_MoveCount=m_XCount*m_YCount;
    //清零已经移动的次数
	return true;
}

bool CMapScanner::Start(void* vParas)
{
	if (m_ScanThread!=0)
	{
		if (WaitForSingleObject(m_ScanThread,300)==WAIT_TIMEOUT)
		{
			AfxMessageBox(L"The last thread havn't finished!");
			SetEvent(m_hMoveEvent);
			SetEvent(m_hCaptureEvent);
			return false;
		}
	}

	m_PointNext.x=mMapScanParas->StartPt.x;
	m_PointNext.y=mMapScanParas->StartPt.y;
	m_ConfocalCore->GetRenderChain()->push_back(this);//加入渲染

	ResetEvent(m_hMoveEvent);
	ResetEvent(m_hCaptureEvent);
	bScan=true;
	m_CaptureVideo.VnCount=0;//清零
	m_CurVideoDevice->SetTrigerMode(TRIGER_SOFT);

	m_ScanThread = CreateThread(NULL, 0, ScanMapProc, this, 0, 0);//这个开线程

	HCorePanel* mDlg=m_ConfocalCore->GetMsgPanel(MSG_TIME);
	mDlg->InitPanel(0);
	this->Attach(mDlg);
	m_ConfocalCore->GetDockablePanel(DOCKPANEL_STAGECTL)
		->GetCWnd()->PostMessage(0x118);
	((CDialogEx*)mDlg->GetCWnd())->DoModal();
	this->Detach(mDlg);
	if (mMapScanParas->mScanType==Scan_Precision)
	{
		m_ConfocalCore->GetPlugin(PLUGIN_AutoFocus)->Stop();//停止调焦
	}
	bScan=false;//退出线程
	WaitForSingleObject(m_ScanThread,2000);
	m_ConfocalCore->GetRenderChain()->remove(this);
	mMapProcess->UnInitProcess();
	return true;
}

int CMapScanner::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if(pSubject==m_AxisX->GetSubject())
	{
		if(ID==NOTIFY_AXIS_MOVED)//NOTIFY_AXIS_POSITION)//
		{
			SetEvent(m_hMoveEvent);
		}

	}else if (pSubject==m_AxisY->GetSubject())
	{
		if(ID==NOTIFY_AXIS_MOVED)//NOTIFY_AXIS_POSITION)//
		{
			SetEvent(m_hMoveEvent);
		}
	}
	else if (pSubject==m_AxisZ->GetSubject())
	{

	}
	else if (ID==NOTIFY_MAP_END)
	{
		CString str=*(CString*)pParam;
		CString strPath=*(CString*)mParam;
		/*CString strSystemDir;
		::GetSystemDirectory(strSystemDir.GetBuffer(256), 256);//取得System目录路径
		strSystemDir.ReleaseBuffer();
		CString strRundll;
		strRundll = strSystemDir + L"\\rundll32.exe ";
		CString strParm;
		strParm.Format(L"%s\\shimgvw.dll,ImageView_Fullscreen %s", strSystemDir, str);
		ShellExecute(NULL,L"Open",strRundll,strParm,NULL,SW_SHOWNORMAL);*/
		ShellExecute( NULL, _T("open"), _T("explorer.exe"), strPath, NULL, SW_SHOWNORMAL);  

		CString strCopySrc=m_Conf->GetModulePath()+L"\\Plug\\Zoomify Converter.exe";
		CString strCopyDst=strPath+L"\\Zoomify Converter.exe";
		CopyFile(strCopySrc,strCopyDst,TRUE);
		strCopySrc=m_Conf->GetModulePath()+L"\\Plug\\NaviMap.htm";
		strCopyDst=strPath+L"\\NaviMap.htm";
		CopyFile(strCopySrc,strCopyDst,TRUE);
		strCopySrc=m_Conf->GetModulePath()+L"\\Plug\\ZoomifyAnnotationViewer-orig.swf";
		strCopyDst=strPath+L"\\ZoomifyAnnotationViewer-orig.swf";
		CopyFile(strCopySrc,strCopyDst,TRUE);
	}
	return 1;
}

DWORD WINAPI ScanMapProc( LPVOID lp )
{
	((CMapScanner*)lp)->ScanMap();
	return 0;
}

void CMapScanner::ScanMap()
{
	m_CaptureVideo.VnCount=0;//这个代表地图中的第几斟
	m_XtCount=0;
	m_YtCount=0;
	m_MoveTCount=0;
	int tx=0,ty=0;
	while(bScan)
	{
		if(m_XtCount==0&&m_YtCount==0)//第一次
		{
			float m_PosX=m_AxisX->GetPosition();
			float m_PosY=m_AxisY->GetPosition();
			if (abs(m_PosY-m_PointNext.y)>0.005)
			{
				if (!MoveAxis(m_AxisY,m_PointNext.y,L"Status:  Y Moving"))
				{
					bScan=false;
					break;
				}
			}
			if (abs(m_PosX-m_PointNext.x)>0.005)
			{
				if (!MoveAxis(m_AxisX,m_PointNext.x,L"Status:  X Moving"))
				{
					bScan=false;
					break;
				}	
			}
		}
		else if(m_MoveTCount%m_XCount==0)
		{
			if (!MoveAxis(m_AxisY,m_PointNext.y,L"Status:  Y Moving"))
			{
				bScan=false;
				break;
			}
			if (m_PointNext.y!=mMapScanParas->StartPt.y)
				m_YtCount++;
			m_XtCount=0;
		}
		else
		//if (abs(m_PosX-m_PointNext.x)>0.005)
		{
			if (!MoveAxis(m_AxisX,m_PointNext.x,L"Status:  X Moving"))
			{
				bScan=false;
				break;
			}		
		}

		if(mMapScanParas->iSleepTime>0)
			Sleep(mMapScanParas->iSleepTime);

		if (!Capture())
		{
			bScan=false;
			break;
		}

		m_MoveTCount++;
		m_XtCount++;	

		tx=m_MoveTCount%m_XCount;
		ty=m_MoveTCount/m_XCount;
		if((ty%2)!=0)
			tx=m_XCount-tx-1;
		else 
			int k=1;
		m_PointNext.x=(float)tx*m_MoveStep+mMapScanParas->StartPt.x;
		m_PointNext.y=(float)ty*m_MoveStep+mMapScanParas->StartPt.y;

		strMsg.Format(L"Rows:%d/%d,Cols:%d/%d,Total:%d/%d",m_XtCount,m_XCount,m_YtCount,m_YCount,m_MoveTCount,m_MoveCount);
		Notify(this,NOTIFY_MSGVIEW_Info,0,&strMsg);

		if(m_MoveTCount>=m_MoveCount)//全部都移完了
		{ 					
			m_AxisX->MoveTo(mMapScanParas->StartPt.x);
			m_AxisY->MoveTo(mMapScanParas->StartPt.y);//回原位
			m_ConfocalCore->GetRenderChain()->remove(this);
			//下面存数据................
			//HVideoHeader* t=m_MapMerge.GetMap();
			//HSaveBmp(t->Vbuffer,t->Vwidth,t->Vheight,t->VwBit,L".\\Configure\\CofocalMap.bmp");
			bScan=false;
			break;//推出线程
		}
	}
	m_CurVideoDevice->SetTrigerMode(TRIGER_INTERAL);
	Notify(this,NOTIFY_MSGVIEW_END,0,0);
}

bool CMapScanner::MoveAxis(HAxis *m_Axis,float mPos,CString strOut)
{
	int iTime=7000;
	Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strOut);
	ResetEvent(m_hMoveEvent);
	if (mPos<0)
		m_Axis->Rest();
	else
		m_Axis->MoveTo(mPos,mMapScanParas->iScanSpeed);
	if (WaitForSingleObject(m_hMoveEvent,iTime)==WAIT_TIMEOUT)
	{
		CString strMsg=strOut+L"  TIMEOUT!";
		Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
		Sleep(3000);
		return false;
	}
	return true;
}

void CMapScanner::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
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

bool CMapScanner::Capture()
{
	//先进行调焦
	if (mMapScanParas->mScanType==Scan_Precision)
	{
		m_ConfocalCore->GetRenderChain()->remove(this);
		bool bV=true;
		//m_ConfocalCore->GetPlugin(PLUGIN_AutoFocus)->Start(&bV);

		m_ConfocalCore->GetPlugin(PLUGIN_AutoFocusEx)->Start(&bV);

		if (!bScan)//已经退出了
			return false;
		m_ConfocalCore->GetRenderChain()->push_back(this);

		//m_CurVideoDevice->SetExposure(m_CurVideoDevice->GetExposure());
		m_CurVideoDevice->SetTrigerMode(TRIGER_INTERAL);
		m_CurVideoDevice->SetTrigerMode(TRIGER_SOFT);
		/*m_CurVideoDevice->Run();
		Sleep(500);
		m_CurVideoDevice->Pause();*/
	}
	ResetEvent(m_hCaptureEvent);
	strMsg=L"Status:   Capturing!";
	Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
	m_CurVideoDevice->TrigerVideoData();
	int iCount=0;
	while(WaitForSingleObject(m_hCaptureEvent,1300)==WAIT_TIMEOUT)
	{
		iCount++;
		if (iCount<=3)
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
	//图像处理部分
	CPoint pt;//=new CPoint(m_XtCount,m_YtCount);
	if ((m_MoveTCount/m_XCount)%2==0)
		pt.x=m_XtCount;
	else
		pt.x=m_XCount-m_XtCount-1;
	pt.y=m_YCount-1-m_YtCount;
	bool bR=mMapProcess->ProcessImg(&pt,&m_MoveCount,&m_CaptureVideo);
	return bR;//mMapProcess->ProcessImg(&pt,&m_MoveCount,&m_CaptureVideo);
}
