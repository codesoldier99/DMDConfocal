#include "StdAfx.h"
#include "ColorSacnner.h"
#include "HGloableFunction.h"

CColorSacnner::CColorSacnner(void)
{
	m_CaptureVideo.Vsize=0;//表示没有东西
	m_CaptureVideo.Vbuffer=0;
	m_hCaptureEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
}


CColorSacnner::~CColorSacnner(void)
{
}

bool CColorSacnner::InitPlugin(LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	if (m_ConfocalCore!=0)
	{
		m_CurVideoDevice=m_ConfocalCore->GetCurVideo();//获取相机
		m_ColorBuildPro=m_ConfocalCore->GetCoreProcess(PROCESS_ColorBuild);
    if (m_ColorBuildPro)
    {
		  m_ColorBuildPro->InitProcess();
		  m_ColorBuildPro->Attach(this);
    }
	}
	return true;
}

bool CColorSacnner::UnInitPlugin()
{
	bScan=false;
	return true;
}

bool CColorSacnner::Stop()
{
	bScan=false;
	return true;
}

bool CColorSacnner::Start(void* vParas)
{
	m_CurVideoDevice->SetTrigerMode(TRIGER_SOFT);
	m_ConfocalCore->GetRenderChain()->push_back(this);//加入渲染

	m_ColorBuildPro->Attach(m_ConfocalCore->GetViewPanel(VIEWPANEL_COLOR));
	//this->Attach(m_ConfocalCore->GetViewPanel(VIEWPANEL_COLOR));
	ResetEvent(m_hCaptureEvent);
	m_ScanThread = CreateThread(NULL, 0, ColorProc, this, 0, 0);//这个开线程
	bScan=true;
	return true;
}

void CColorSacnner::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
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

int CColorSacnner::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if (pSubject==m_ConfocalCore->GetCoreProcess(PROCESS_ColorBuild))
	{
		/*HVideoHeader* hP=(HVideoHeader*)pParam;
		if(ID==NOTIFY_COLORBUILD_END)
		{
			HSaveBmp(hP->Vbuffer,hP->Vwidth,hP->Vheight,hP->VwBit,HGetStartPath() + L"\\EpiResult\\ResultColor.bmp");
		}*/
		//Notify(this,NOTIFY_COLORBUILD_END,0,hP);
	}
	//else if (pSubject==m_AxisZ->GetSubject())
	//{
	//	if(ID==NOTIFY_AXIS_MOVED||ID==NOTIFY_AXIS_POSITION)
	//		SetEvent(m_hMoveEvent);
	//}
	return 0;
}

DWORD WINAPI ColorProc( LPVOID lp )
{
	((CColorSacnner*)lp)->ScanImage();
	return 0;
}

void CColorSacnner::ScanImage()
{
	while(bScan)
	{
		AfxMessageBox(L"Switch the Red Channel!");
		if (!Capture())//抓图+传图
		{
			bScan=false;
			//AfxMessageBox(L"Capture Failed!");
			break;
		}

		AfxMessageBox(L"Switch the Green Channel!");
		if (!Capture())//抓图+传图
		{
			bScan=false;
			//AfxMessageBox(L"Capture Failed!");
			break;
		}

		AfxMessageBox(L"Switch the Blue Channel!");
		if (!Capture())//抓图+传图
		{
			bScan=false;
			//AfxMessageBox(L"Capture Failed!");
			break;
		}
		bScan=false;
		break;
	}
	m_ConfocalCore->GetRenderChain()->remove(this);//删除视频
	m_ColorBuildPro->InitProcess();

	m_CurVideoDevice->SetTrigerMode(TRIGER_INTERAL);
}

bool CColorSacnner::Capture()
{
	m_CurVideoDevice->TrigerVideoData();
	int iCount=0;

	while(WaitForSingleObject(m_hCaptureEvent,2000)==WAIT_TIMEOUT)
	{
		iCount++;
		if (iCount<=3)
		{
				m_CurVideoDevice->TrigerVideoData();
		}	
		else
		{
			return false;
		}		
	}
	//strMsg=L"Status:   Processing!";
	//Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
	m_ColorBuildPro->GetVideoRender()->Renderer(&m_CaptureVideo,m_CaptureVideo.Vbuffer);//输出到3d库中
	//m_CaptureVideo.VcaptureTime++;//编号加一

	return true;
}