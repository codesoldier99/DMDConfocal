
#include "StdAfx.h"
#include "DMoticCamera.h"
#include "ImgProcess.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef TEST_MONO
#define TEST_MONO
#endif

MucamVideo::MucamVideo(MUCam_Handle t_handle,CString t_name)
{
	m_hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	InitializeCriticalSection(&m_crtSec);
	m_nWaitTime = 10;
	m_VideoRender=0;
	m_AWB=false;
	bOpen=false;

	g_MuCameraParas.m_iCurHandle=t_handle;
	g_MuCameraParas.m_Format=MUCam_getFrameFormat(t_handle);
	g_MuCameraParas.m_CameraName=t_name;
	g_MuCameraParas.m_pBuffer=0;
	g_MuCameraParas.m_pWidth=0;
	g_MuCameraParas.m_pHeight=0;
	g_MuCameraParas.m_fExposure=1.7;
	g_MuCameraParas.m_Gain=1;
	width=0;
	height=0;
}

MucamVideo::~MucamVideo( void )
{
	CloseCamera();
	g_MuCameraParas.m_iCurHandle=NULL;
	m_VideoRender=0;
}

CString MucamVideo::GetBining( int pIndex )
{
	CString temp;
	temp.Format(_T("%d X %d"),g_MuCameraParas.m_pWidth[pIndex],
		g_MuCameraParas.m_pHeight[pIndex]);
	return temp;
}

void MucamVideo::SetBinning( int pIndex,CPoint Pt)
{
	if (pIndex==g_MuCameraParas.m_iBinningIndx)
		return;
	EnterCriticalSection(&m_crtSec);
	if(MUCam_setBinningIndex(g_MuCameraParas.m_iCurHandle, pIndex))
	{
		g_MuCameraParas.m_iBinningIndx= pIndex;
	}
	LeaveCriticalSection(&m_crtSec);
}

bool MucamVideo::OpenCamera()
{
	if (bOpen)
		return true;
	MUCam_Handle hCamera=g_MuCameraParas.m_iCurHandle;//=hCamera;
	if(MUCam_openCamera(hCamera))
	{
		bOpen=true;
		int binCount = MUCam_getBinningCount(hCamera);
		if(binCount <= 0)
		{
			CloseCamera();
			return false;
		}
		if(binCount != g_MuCameraParas.m_iBinningCount || 
			g_MuCameraParas.m_pWidth == NULL || g_MuCameraParas.m_pHeight == NULL)
		{
			g_MuCameraParas.m_iBinningCount = binCount;
			if(g_MuCameraParas.m_pWidth)delete[] g_MuCameraParas.m_pWidth;
			if(g_MuCameraParas.m_pHeight)delete[] g_MuCameraParas.m_pHeight;
			g_MuCameraParas.m_pWidth = new int[g_MuCameraParas.m_iBinningCount];
			g_MuCameraParas.m_pHeight = new int[g_MuCameraParas.m_iBinningCount];
		}	
		if(MUCam_getBinningList(hCamera, g_MuCameraParas.m_pWidth, g_MuCameraParas.m_pHeight))
		{

		}
		if(MUCam_setBinningIndex(hCamera, 0))
		{
			g_MuCameraParas.m_iBinningIndx= 0;
		}

		g_MuCameraParas.m_iBitDepth = 8;
		if(MUCam_setBitCount(hCamera, 8))
		{
			g_MuCameraParas.m_iBitDepth = 8;    
		}
		g_MuCameraParas.m_iColorChannel = 3;//RGB 3 channels
		MUCam_Format fmt = MUCam_getFrameFormat(hCamera);
		if(fmt == MUCAM_FORMAT_MONOCHROME)
		{
			g_MuCameraParas.m_iColorChannel = 1;//only 1 channel
		}
#ifdef TEST_MONO
    g_MuCameraParas.m_iColorChannel = 1;
#endif
		//@Exposure    
		MUCam_setExposure(hCamera, g_MuCameraParas.m_fExposure);
		int r, g, b;
		MUCam_setRGBOffset(hCamera, 0, 0, 0);
		MUCam_setRGBGainValue(hCamera, g_MuCameraParas.m_Gain+0.0f, 
			g_MuCameraParas.m_Gain+0.0f, g_MuCameraParas.m_Gain+0.0f, &r, &g, &b);
		//MUCam_setMirror(hCamera,true);//镜像他，很重要！

		Notify(this,Notify_CarlVideo_OpenCamera,0,&g_MuCameraParas.m_CameraName);
		return true;
	}
	return false;
}

bool MucamVideo::Run()
{
	//启动图像抓取线程
	if (!bOpen)
		return false;
	if (WaitForSingleObject(m_hGrabberThread, 10)==WAIT_TIMEOUT)
		return true;
	ResetEvent(m_hEvent);
	m_hGrabberThread = CreateThread(NULL, 0, GrabVideoDataProc, this, 0, 0);
	if (WaitForSingleObject(m_hGrabberThread, 100)==WAIT_TIMEOUT)
		return true;	
	return false;
}

bool MucamVideo::Pause()
{
	if (!bOpen)
		return true;
	SetEvent(m_hEvent);
	if (WaitForSingleObject(m_hGrabberThread, 300)==WAIT_TIMEOUT)
		return false;
	return true;
}

bool MucamVideo::CloseCamera()
{
	bOpen=false;
	Notify(this,Notify_CarlVideo_CloseCamera,0,&g_MuCameraParas.m_CameraName);
	SetEvent(m_hEvent);
	if (WaitForSingleObject(m_hGrabberThread, 1000)==WAIT_TIMEOUT)
	{
		//return false;
	}
  if (g_MuCameraParas.m_iCurHandle)
  {
    MUCam_closeCamera(g_MuCameraParas.m_iCurHandle);
    g_MuCameraParas.m_iCurHandle = 0;
  }
	return true;
}

bool MucamVideo::GetEnable(HVideoPID pPID)
{
	switch(pPID)
	{    
		case VIDEO_PID_WhitBalance:
			return true;
			break;
		case VIDEO_PID_AutoExp:
			return false;
			break;
		default:
			return false;
			break;
	}
	return false;
}


void MucamVideo::SetValue( HVideoPID pPID,float pValue )
{
	EnterCriticalSection(&m_crtSec);
	switch(pPID)
	{     
	case VIDEO_PID_RGAIN:

		break;
	case VIDEO_PID_GGAIN:

		break;
	case VIDEO_PID_BGAIN:

		break;
	case VIDEO_PID_GAIN:
		int r, g, b;
		MUCam_setRGBOffset(g_MuCameraParas.m_iCurHandle, 0, 0, 0);
		MUCam_setRGBGainValue(g_MuCameraParas.m_iCurHandle, pValue+0.0f, pValue+0.0f, pValue+0.0f, &r, &g, &b);
		break;
	}
	LeaveCriticalSection(&m_crtSec);
}

float MucamVideo::GetValue( HVideoPID pPID )
{
	EnterCriticalSection(&m_crtSec);
	float iV=0.0f,iV2=0.0f;
	switch(pPID)
	{    
		case VIDEO_PID_EXMAX:
			MUCam_getExposureRange(g_MuCameraParas.m_iCurHandle,&iV,&iV2);
			break;
		case VIDEO_PID_EXMIN:
			MUCam_getExposureRange(g_MuCameraParas.m_iCurHandle,&iV2,&iV);
			break;
		case VIDEO_PID_RGAIN:

			break;
		case VIDEO_PID_GGAIN:

			break;
		case VIDEO_PID_BGAIN:

			break;
		case VIDEO_PID_GAIN:

			break;
	}
	return iV;
	LeaveCriticalSection(&m_crtSec);
}

void MucamVideo::SetExposure( float pExpos )
{
	g_MuCameraParas.m_fExposure=pExpos;
	EnterCriticalSection(&m_crtSec);
	MUCam_setExposure(g_MuCameraParas.m_iCurHandle, g_MuCameraParas.m_fExposure);
	LeaveCriticalSection(&m_crtSec);
	Notify(this,Notify_CarlVideo_ExpTime,0,0,g_MuCameraParas.m_fExposure);
}

void MucamVideo::AutoAWB(bool pAuto)
{
	m_AWB=pAuto;
}

bool MucamVideo::SetFlip(bool pFlip)
{
	if (!bOpen)
		return false;
	EnterCriticalSection(&m_crtSec);
	MUCam_setFlip(g_MuCameraParas.m_iCurHandle,pFlip);
	LeaveCriticalSection(&m_crtSec);
	Notify(this,Notify_CarlVideo_Flip,0,&pFlip);
	return true;
}

bool MucamVideo::SetMirror(bool pMirror)
{
	if (!bOpen)
		return false;
	EnterCriticalSection(&m_crtSec);
	MUCam_setMirror(g_MuCameraParas.m_iCurHandle,pMirror);
	LeaveCriticalSection(&m_crtSec);
	Notify(this,Notify_CarlVideo_Mirror,0,&pMirror);
	return true;
}

HVideoHeader MucamVideo::GetImgHeader()
{
	HVideoHeader t_result;
	t_result.Vheight=g_MuCameraParas.m_pHeight[g_MuCameraParas.m_iBinningIndx];
	t_result.Vwidth=g_MuCameraParas.m_pWidth[g_MuCameraParas.m_iBinningIndx];
#ifdef TEST_MONO
  t_result.VwBit = 8;
#else
  t_result.VwBit = g_MuCameraParas.m_iColorChannel;
#endif
	t_result.Vuser=(long)g_MuCameraParas.m_iCurHandle;//标识相机
	return t_result;
}

bool MucamVideo::GrabImage( HVideoHeader* pHeader,LPBYTE pBuffer )
{
	if (!bOpen)
		return false;

	if(m_AWB)//自动白平衡
	{
		if (g_MuCameraParas.m_Format!=MUCAM_FORMAT_MONOCHROME)
		{
			double red=0.0,green=0.0,blue=0.0;
			int r,g,b;
			MUCam_setRGBGainValue(g_MuCameraParas.m_iCurHandle,1.0f,1.0f,1.0f,&r, &g, &b);
			Sleep(500);
			if (GrabBuffer(width,height))
			{
				if(Cam_AutoAWB(g_MuCameraParas.m_pBuffer,width,height,red,green,blue))
				{
					MUCam_setRGBGainValue(g_MuCameraParas.m_iCurHandle, red, 
						green,blue, &r, &g, &b);
				}
				m_AWB=false;
			}
			Sleep(300);
		}
	}

	//EnterCriticalSection(&m_crtSec);
	bool bReturn=false;
	pHeader->Vheight=g_MuCameraParas.m_pHeight[g_MuCameraParas.m_iBinningIndx];
	pHeader->Vwidth=g_MuCameraParas.m_pWidth[g_MuCameraParas.m_iBinningIndx];
	pHeader->VwBit=g_MuCameraParas.m_iColorChannel;
	pHeader->Vuser=(long)g_MuCameraParas.m_iCurHandle;//标识相机
	int iSize=pHeader->Vwidth*pHeader->Vheight*pHeader->VwBit;
	pHeader->VwBit*=8;
	if (pHeader->Vsize!=iSize)//重新申请空间
	{
		pHeader->Vsize=iSize;
		if (pHeader->Vbuffer)
			delete pHeader->Vbuffer;
		pHeader->Vbuffer=(LPBYTE)malloc(pHeader->Vsize);
	}
	if(GrabBuffer(width,height))
	{
		//pHeader->Vsize=width*height*pHeader->VwBit;
		//memcpy(pBuffer,g_MuCameraParas.m_pBuffer,pHeader->Vsize);
		memcpy(pHeader->Vbuffer,g_MuCameraParas.m_pBuffer,pHeader->Vsize);
		bReturn=true;
	}
	else
		bReturn=false;
	//LeaveCriticalSection(&m_crtSec);
	return bReturn;
}

void MucamVideo::SetTrigerMode( TrigerType pMode )
{
	if(!bOpen)
		return;
	//TRACE("SetTrigerMode开始进入 互锁\n");
	EnterCriticalSection(&m_crtSec);
	//TRACE("SetTrigerMode已经进入 互锁\n");
	switch(pMode)
	{
	case TRIGER_INTERAL:
		MUCam_setTriggerType(g_MuCameraParas.m_iCurHandle,MUCAM_TRIGGER_FREE);
		if (WaitForSingleObject(m_hGrabberThread, 10) != WAIT_TIMEOUT)
			this->Run();
		break;
	case TRIGER_SOFT:
		if (WaitForSingleObject(m_hGrabberThread, 10) == WAIT_TIMEOUT)
			this->Pause();
		MUCam_setTriggerType(g_MuCameraParas.m_iCurHandle,MUCAM_TRIGGER_SOFTWARE);
		break;
	case TRIGER_OUT:
		MUCam_setTriggerType(g_MuCameraParas.m_iCurHandle,MUCAM_TRIGGER_HARDWARE_FALL);
		if (WaitForSingleObject(m_hGrabberThread, 10) != WAIT_TIMEOUT)
			this->Run();
		break;
	}
	//MUCam_setExposure(g_MuCameraParas.m_iCurHandle, g_MuCameraParas.m_fExposure);//设置下曝光
	Sleep(300);
	LeaveCriticalSection(&m_crtSec);
	Notify(this,Notify_CarlVideo_Triger,0,&pMode);
}

void MucamVideo::SetRender( HVideoRender* pRender )
{
	EnterCriticalSection(&m_crtSec);
	if(pRender)
		m_VideoRender=pRender;
	LeaveCriticalSection(&m_crtSec);
}

void MucamVideo::TrigerVideoData()
{
	if (g_MuCameraParas.m_iCurHandle)
	{
		//EnterCriticalSection(&m_crtSec);
		if (GrabBuffer(width,height))//抓图成功
		{
			DWORD t_now=::GetTickCount();
			DWORD dutime=t_now-(DWORD)m_VideoFormate.VcaptureTime;
			m_VideoFormate.VcaptureTime=t_now;
			m_VideoFormate.VFps=1000.0/dutime;
			m_VideoFormate.Vheight=height;
			m_VideoFormate.Vwidth=width;
			m_VideoFormate.VwBit = g_MuCameraParas.m_iColorChannel;
			m_VideoFormate.Vuser=(long)g_MuCameraParas.m_iCurHandle;//标识相机
			m_VideoFormate.Vsize=width*height*m_VideoFormate.VwBit;
			m_VideoFormate.VwBit*=8;
			//获取当前时间
			if(m_VideoRender)
				m_VideoRender->Renderer(&m_VideoFormate,g_MuCameraParas.m_pBuffer);
		}
		//LeaveCriticalSection(&m_crtSec);
	}
}

void MucamVideo::GrabVideoData()
{
	long size;
	int failTimes=0;
	m_nWaitTime=1;
	Notify(this,Notify_CarlVideo_RunCamera,0,&g_MuCameraParas.m_CameraName);
	while(WaitForSingleObject(m_hEvent,m_nWaitTime) != WAIT_OBJECT_0 && bOpen)
	{
		//EnterCriticalSection(&m_crtSec);
		if (GrabBuffer(width,height))//抓图成功
		{
			//LeaveCriticalSection(&m_crtSec);
			DWORD t_now=::GetTickCount();
			DWORD dutime=t_now-(DWORD)m_VideoFormate.VcaptureTime;
			m_VideoFormate.VcaptureTime=t_now;
			m_VideoFormate.VFps=1000.0/dutime;
			m_VideoFormate.Vheight=g_MuCameraParas.m_pHeight[g_MuCameraParas.m_iBinningIndx];
			m_VideoFormate.Vwidth=g_MuCameraParas.m_pWidth[g_MuCameraParas.m_iBinningIndx];
			m_VideoFormate.VwBit = g_MuCameraParas.m_iColorChannel;
			m_VideoFormate.Vuser=(long)g_MuCameraParas.m_iCurHandle;//标识相机
			m_VideoFormate.Vsize=m_VideoFormate.Vheight*m_VideoFormate.Vwidth*m_VideoFormate.VwBit;
			m_VideoFormate.VwBit*=8;

			//获取当前时间
			if(m_VideoRender)
				m_VideoRender->Renderer(&m_VideoFormate,g_MuCameraParas.m_pBuffer);

			failTimes=0;
			if(m_AWB)//自动白平衡
			{
				if (g_MuCameraParas.m_Format!=MUCAM_FORMAT_MONOCHROME)
				{
					double red=0.0,green=0.0,blue=0.0;
					int r,g,b;
					MUCam_setRGBGainValue(g_MuCameraParas.m_iCurHandle,1.0f,1.0f,1.0f,&r, &g, &b);
					Sleep(500);
					if (GrabBuffer(width,height))
					{
						if(Cam_AutoAWB(g_MuCameraParas.m_pBuffer,width,height,red,green,blue))
						{
							MUCam_setRGBGainValue(g_MuCameraParas.m_iCurHandle, red, 
								green,blue, &r, &g, &b);
						}
						m_AWB=false;
					}
				}
			}
		}
		else
		{
			//LeaveCriticalSection(&m_crtSec);
			if(failTimes++>20)//不知咋的，进失败的次数非常多
			{
				failTimes=0;
				if (MUCam_isConnected(g_MuCameraParas.m_iCurHandle)==false)
				{
					CloseCamera();
					break;//退出
				}
			}
		}
	}
	//if (bOpen)
		Notify(this,Notify_CarlVideo_PauseCamera,0,&g_MuCameraParas.m_CameraName);
}

DWORD WINAPI GrabVideoDataProc(LPVOID lp)
{
	((MucamVideo*)lp)->GrabVideoData();
	return 0;
}

void MucamVideo::UpdateBufferSize(int width, int height)
{
	if (g_MuCameraParas.m_pBuffer)
	{
		delete(g_MuCameraParas.m_pBuffer);
	}
	g_MuCameraParas.m_pBuffer=(LPBYTE)malloc(width*height*
		g_MuCameraParas.m_iColorChannel);
}

bool MucamVideo::GrabBuffer(int& pwidth,int& pheight)
{
	//TRACE("采集线程 开始进入 互锁\n");
	EnterCriticalSection(&m_crtSec);

	//TRACE("采集线程 已经进入 互锁\n");
	if (pwidth!=g_MuCameraParas.m_pWidth[g_MuCameraParas.m_iBinningIndx]&&
		pheight!=g_MuCameraParas.m_pHeight[g_MuCameraParas.m_iBinningIndx])
	{
		pwidth=g_MuCameraParas.m_pWidth[g_MuCameraParas.m_iBinningIndx];
		pheight=g_MuCameraParas.m_pHeight[g_MuCameraParas.m_iBinningIndx];
		UpdateBufferSize(pwidth,pheight);
	}

#ifdef TEST_MONO
  int w = g_MuCameraParas.m_pWidth[g_MuCameraParas.m_iBinningIndx];
  int h = g_MuCameraParas.m_pHeight[g_MuCameraParas.m_iBinningIndx];
  BYTE* buf(0);
  if (g_MuCameraParas.m_Format != MUCAM_FORMAT_MONOCHROME)
  {
    buf = new BYTE[w * h * 3];
  }
  else
  {
    buf = g_MuCameraParas.m_pBuffer;
  }
  if (MUCam_getFrame(g_MuCameraParas.m_iCurHandle, buf, 0))
  {
    for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
      {
        g_MuCameraParas.m_pBuffer[i * w + j]
          = buf[(i * w + j) * 3 + 1];
      }
    }
    if (g_MuCameraParas.m_Format != MUCAM_FORMAT_MONOCHROME)
    {
      delete[]buf;
    }
    LeaveCriticalSection(&m_crtSec);
    //TRACE("采集线程 已经退出 互锁\n");
    return true;
  }
  else
  {
    if (g_MuCameraParas.m_Format != MUCAM_FORMAT_MONOCHROME)
    {
      delete []buf;
    }
  }
#else
	if(MUCam_getFrame(g_MuCameraParas.m_iCurHandle,g_MuCameraParas.m_pBuffer, 0))		
	{
		LeaveCriticalSection(&m_crtSec);
		//TRACE("采集线程 已经退出 互锁\n");
		return true;
	}
#endif
	LeaveCriticalSection(&m_crtSec);
	//TRACE("采集线程 已经退出 互锁\n");
	return false;
}


