
#include "StdAfx.h"
#include "DcamVideo.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDcamVideo::CDcamVideo(int idx,CString strName)
{
	m_hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_hGrabberThread=0;
	InitializeCriticalSection(&m_crtSec);
	m_nWaitTime = 10;
	m_VideoRender=0;
	m_AWB=false;
	bOpen=false;
	bRun=false;
	bMirror=false;
	bFlip=false;

	g_MuCameraParas.m_iCurHandle=NULL;
	g_MuCameraParas.m_CameraName=strName;
	g_MuCameraParas.iSel=idx;
	g_MuCameraParas.m_pBuffer=0;
	g_MuCameraParas.m_pDcamBuffer=0;
	g_MuCameraParas.m_pWidth=0;
	g_MuCameraParas.m_pHeight=0;
	g_MuCameraParas.m_fExposure=10.0;
	g_MuCameraParas.m_Gain=1;
	width=0;
	height=0;

	mFlipMirrorParas.iCount=0;
	mFlipMirrorParas.iFlipDst=NULL;
	mFlipMirrorParas.iFlipSrc=NULL;
	mFlipMirrorParas.iMirrorSrc=NULL;
	mFlipMirrorParas.iMirrorDst=NULL;

	for (int i=0;i<3;i++)
		mBinPt[i].x=mBinPt[i].y=0;
}

CDcamVideo::~CDcamVideo( void )
{
	CloseCamera();
	dcam_uninit();
	m_VideoRender=0;
}

CString CDcamVideo::GetBining( int pIndex )
{
	CString temp;
	temp.Format(_T("%d X %d"),g_MuCameraParas.m_pWidth[pIndex],
		g_MuCameraParas.m_pHeight[pIndex]);
	return temp;
}

void CDcamVideo::SetBinningPt(int pIndex,CPoint Pt)
{
	mBinPt[pIndex]=Pt;
}

void CDcamVideo::SetBinning(int pIndex,CPoint Pt)
{
	if (pIndex==g_MuCameraParas.m_iBinningIndx)
		return;
	int iX=0,iY=0,iW=0,iH=0;
	if (pIndex==0)
	{
		iW=2048;
		iH=2048;
	}
	else if (pIndex==1)
	{
		if (mBinPt[pIndex].x>0&&mBinPt[pIndex].y>0)
		{
			iX=mBinPt[pIndex].x;iY=mBinPt[pIndex].y;
		}
		else
		{
			iX=512;iY=512;
			//iX=0;iY=0;
		}		
		iW=1024;
		iH=1024+4;
	}
	else if (pIndex==2)
	{
		if (mBinPt[pIndex].x>0&&mBinPt[pIndex].y>0)
		{
			iX=mBinPt[pIndex].x;iY=mBinPt[pIndex].y;
		}
		else
		{
			iX=768;iY=768;
			//iX=0;iY=0;
		}		
		iW=512;
		iH=512+4;
	}
	DCAM_PARAM_SUBARRAY	 param;
	memset( &param, 0, sizeof( param ) );
	param.hdr.cbSize	= sizeof( param );
	param.hdr.id	 = DCAM_IDPARAM_SUBARRAY;
	param.hdr.iFlag	 = dcamparam_subarray_hpos
		| dcamparam_subarray_vpos	| dcamparam_subarray_hsize| dcamparam_subarray_vsize;
	param.hpos	= iX;
	param.vpos	= iY;
	param.hsize	= iW;
	param.vsize	= iH;
	EnterCriticalSection(&m_crtSec);
	_DWORD status;
	dcam_getstatus(g_MuCameraParas.m_iCurHandle,&status);
	if(status==DCAM_STATUS_BUSY)
	{
		dcam_idle(g_MuCameraParas.m_iCurHandle);
		dcam_freeframe( g_MuCameraParas.m_iCurHandle );
	}
	bool bReturn=
	dcam_extended( g_MuCameraParas.m_iCurHandle, DCAM_IDMSG_SETPARAM, &param, sizeof( param ) );
	g_MuCameraParas.m_iBinningIndx=pIndex;
	dcam_getstatus(g_MuCameraParas.m_iCurHandle,&status);
	if(status!=DCAM_STATUS_BUSY) //return false;//不在状态
	{
		if (status==DCAM_STATUS_UNSTABLE)
			dcam_precapture( g_MuCameraParas.m_iCurHandle, DCAM_CAPTUREMODE_SEQUENCE);
		else if (status==DCAM_STATUS_STABLE)
			dcam_allocframe( g_MuCameraParas.m_iCurHandle, 3 );//这里暂时定3帧
		dcam_capture(g_MuCameraParas.m_iCurHandle);
	}
	LeaveCriticalSection(&m_crtSec);
}

bool CDcamVideo::OpenCamera()//cameraidx误解
{
	if (bOpen)
		return true;
	//进这里的函数是外部的manager测试过有没有被占用的哦，所以。。。
	if(!dcam_open(&g_MuCameraParas.m_iCurHandle,g_MuCameraParas.iSel)) return false;
	//step1 设分辨率
	int binCount =3;//roi暂时不用
	if(binCount != g_MuCameraParas.m_iBinningCount || 
		g_MuCameraParas.m_pWidth == NULL || g_MuCameraParas.m_pHeight == NULL)
	{
		g_MuCameraParas.m_iBinningCount = binCount;//设总数
		if(g_MuCameraParas.m_pWidth)delete[] g_MuCameraParas.m_pWidth;
		if(g_MuCameraParas.m_pHeight)delete[] g_MuCameraParas.m_pHeight;
		g_MuCameraParas.m_pWidth = new int[g_MuCameraParas.m_iBinningCount];
		g_MuCameraParas.m_pHeight = new int[g_MuCameraParas.m_iBinningCount];
	}//删除空间	
	if(1)//手动设置
	{
		g_MuCameraParas.m_pWidth[0]=2048;
		g_MuCameraParas.m_pHeight[0]=2048;

		g_MuCameraParas.m_pWidth[1]=1024;
		g_MuCameraParas.m_pHeight[1]=1024;

		g_MuCameraParas.m_pWidth[2]=512;
		g_MuCameraParas.m_pHeight[2]=512;

	}
	g_MuCameraParas.m_iBinningIndx= 0;//记入参数
	//Step2 设置数据格式
	g_MuCameraParas.m_iBitDepth = 8;
	g_MuCameraParas.m_iColorChannel = 1;//RGB 3 channels
	//step3 打开相机
	Notify(this,Notify_CarlVideo_OpenCamera,0,&g_MuCameraParas.m_CameraName);
	bOpen=true;

	//DWORD mV;
	////对于DWORD，比如针对DCAM_QUERYCAPABILITY_DATATYPE，如果支持16位和8位的话，它的值会是DCAM_DATATYPE_UINT8|DCAM_DATATYPE_UINT16
	////DCAM_DATATYPE_UINT8是0x00000001；DCAM_DATATYPE_UINT16是0x00000002，都支持的话，就会是0x00000003
	//dcam_getcapability(g_MuCameraParas.m_iCurHandle,&mV,DCAM_QUERYCAPABILITY_DATATYPE);
	//int k=1;
	return true;
}

bool CDcamVideo::Run()
{
	if (!bOpen)
		return false;
	if (WaitForSingleObject(m_hGrabberThread, 10)==WAIT_TIMEOUT)
		return true;
	//ResetEvent(m_hEvent);
	SetEvent(m_hEvent);
	bRun=true;
	m_hGrabberThread = CreateThread(NULL, 0, DCamDataProc, this, 0, 0);//step5 启动线程
	return true;
}

bool CDcamVideo::Pause()
{
	if (!bOpen)
		return true;
	//SetEvent(m_hEvent);
	ResetEvent(m_hEvent);
	bRun=false;
	WaitForSingleObject(m_hGrabberThread, 100);
	return true;
}

bool CDcamVideo::CloseCamera()
{
	bOpen=false;
	if(!g_MuCameraParas.m_iCurHandle)return false;
	//SetEvent(m_hEvent);
	ResetEvent(m_hEvent);
	bRun=false;
	WaitForSingleObject(m_hGrabberThread, INFINITE);
	dcam_close(g_MuCameraParas.m_iCurHandle);
	g_MuCameraParas.m_iCurHandle=NULL;
	Notify(this,Notify_CarlVideo_CloseCamera,0,&g_MuCameraParas.m_CameraName);
	return true;

}

bool CDcamVideo::GetEnable(HVideoPID pPID)
{
	switch(pPID)
	{    
	case VIDEO_PID_WhitBalance:
		return false;
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

void CDcamVideo::SetValue( HVideoPID pPID,float pValue )
{
	EnterCriticalSection(&m_crtSec);
	if(!g_MuCameraParas.m_iCurHandle)
		return;	
	switch(pPID)
	{     
	case VIDEO_PID_RGAIN:

		break;
	case VIDEO_PID_GGAIN:

		break;
	case VIDEO_PID_BGAIN:

		break;
	case VIDEO_PID_GAIN:

		break;
	case VIDEO_PID_TRIGER:
		Sleep(100);
		dcam_settriggermode(g_MuCameraParas.m_iCurHandle,pValue);
		break;
	}
	LeaveCriticalSection(&m_crtSec);
}

float CDcamVideo::GetValue( HVideoPID pPID )
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

		break;

	}
	LeaveCriticalSection(&m_crtSec);
	return 0;
}

void CDcamVideo::SetExposure( float pExpos )
{
	if(!g_MuCameraParas.m_iCurHandle)
		return;
	g_MuCameraParas.m_fExposure=pExpos;
	pExpos/=1000;
	EnterCriticalSection(&m_crtSec);
	dcam_setexposuretime(g_MuCameraParas.m_iCurHandle,(double)pExpos);//由于这个函数设置的是以秒为单位的！
	LeaveCriticalSection(&m_crtSec);
	Notify(this,Notify_CarlVideo_ExpTime,0,0,pExpos*1000);
}

void CDcamVideo::AutoAWB(bool pAuto)
{
	m_AWB=pAuto;
}

HVideoHeader CDcamVideo::GetImgHeader()
{
	HVideoHeader t_result;
	t_result.Vheight=g_MuCameraParas.m_pHeight[g_MuCameraParas.m_iBinningIndx];
	t_result.Vwidth=g_MuCameraParas.m_pWidth[g_MuCameraParas.m_iBinningIndx];
	t_result.VwBit=g_MuCameraParas.m_iColorChannel;
	t_result.Vuser=(long)g_MuCameraParas.m_iCurHandle;//标识相机
	t_result.Vsize=t_result.Vwidth*t_result.Vheight*t_result.VwBit;
	t_result.VwBit *= 8;// 16;
	return t_result;
}

bool CDcamVideo::GrabImage( HVideoHeader* pHeader,LPBYTE pBuffer )
{
	pHeader->Vheight=g_MuCameraParas.m_pHeight[g_MuCameraParas.m_iBinningIndx];
	pHeader->Vwidth=g_MuCameraParas.m_pWidth[g_MuCameraParas.m_iBinningIndx];
	pHeader->VwBit = g_MuCameraParas.m_iColorChannel * 8;// 16;
	pHeader->Vuser=(long)g_MuCameraParas.m_iCurHandle;//标识相机
	if(GrabBuffer(width,height))
	{
		pHeader->Vsize=width*height*pHeader->VwBit/8;
		//if()pBuffer指令是否空未定
		memcpy(pBuffer,g_MuCameraParas.m_pBuffer,pHeader->Vsize);
		return true;
	}
	else
	{
		pHeader->Vsize=width*height*pHeader->VwBit/8;
		//if()pBuffer指令是否空未定
		memcpy(pBuffer,g_MuCameraParas.m_pBuffer,pHeader->Vsize);
		return false;
	}
}

void CDcamVideo::SetRender( HVideoRender* pRender )
{
	EnterCriticalSection(&m_crtSec);
	m_VideoRender=pRender;
	LeaveCriticalSection(&m_crtSec);
}

void CDcamVideo::SetTrigerMode( TrigerType pMode )
{
	EnterCriticalSection(&m_crtSec);
	_DWORD status;
	dcam_getstatus(g_MuCameraParas.m_iCurHandle,&status);
	if(status==DCAM_STATUS_BUSY)
		dcam_idle(g_MuCameraParas.m_iCurHandle);
	switch(pMode)
	{
		case TRIGER_INTERAL:
			dcam_settriggermode(g_MuCameraParas.m_iCurHandle,DCAM_TRIGMODE_INTERNAL);
			if (!bRun)
				this->Run();
			break;
		case TRIGER_SOFT:
			//dcam_settriggermode(g_MuCameraParas.m_iCurHandle,DCAM_TRIGMODE_SOFTWARE);
			if (bRun)
				this->Pause();
			break;
		case TRIGER_OUT:
			dcam_settriggermode(g_MuCameraParas.m_iCurHandle,DCAM_TRIGMODE_EDGE);
			dcam_settriggerpolarity(g_MuCameraParas.m_iCurHandle, DCAM_TRIGPOL_POSITIVE);
			if (!bRun)
				this->Run();
			break;
	}
	dcam_getstatus(g_MuCameraParas.m_iCurHandle,&status);
	LeaveCriticalSection(&m_crtSec);
	Notify(this,Notify_CarlVideo_Triger,0,&pMode);
}

void CDcamVideo::TrigerVideoData()
{

	if (g_MuCameraParas.m_iCurHandle)
	{
		if (GrabBuffer(width,height))//抓图成功
		{
			DWORD t_now=::GetTickCount();
			DWORD dutime=t_now-(DWORD)m_VideoFormate.VcaptureTime;
			m_VideoFormate.VcaptureTime=t_now;
			m_VideoFormate.VFps=1000.0/dutime;
			RenderVideoData();//渲染
		}
	}
}

void CDcamVideo::GrabVideoData()//抓图进程 
{
	int failTimes=0;
	m_nWaitTime=3;
	Notify(this,Notify_CarlVideo_RunCamera,0,&g_MuCameraParas.m_CameraName);
	//while(WaitForSingleObject(m_hEvent,m_nWaitTime) != WAIT_OBJECT_0 &&bOpen)
	while(bRun&&bOpen)
	{
		if( WaitForSingleObject( m_hEvent, 1000 ) == WAIT_TIMEOUT )
			break;
		if (GrabBuffer(width,height))//抓图成功
		{
			failTimes=0;
			DWORD t_now=::GetTickCount();
			DWORD dutime=t_now-(DWORD)m_VideoFormate.VcaptureTime;
			m_VideoFormate.VcaptureTime=t_now;
			m_VideoFormate.VFps=1000.0/dutime;
			//////////////复制渲染//////////////
			RenderVideoData();
			//////////////复制渲染//////////////
		}
		else
		{
			if(failTimes++>20)//不知咋的，进失败的次数非常多
			{
				failTimes=0;
			}
		}
	}
	if (bOpen)
		Notify(this,Notify_CarlVideo_PauseCamera,0,&g_MuCameraParas.m_CameraName);


}

DWORD WINAPI DCamDataProc(LPVOID lp)
{
	((CDcamVideo*)lp)->GrabVideoData();
	return 0;
}

void CDcamVideo::UpdateBufferSize(int width, int height)
{
	if (g_MuCameraParas.m_pBuffer)
		delete(g_MuCameraParas.m_pBuffer);
	if(g_MuCameraParas.m_pDcamBuffer)
		delete(g_MuCameraParas.m_pDcamBuffer);
	g_MuCameraParas.m_pBuffer=(LPBYTE)malloc(width*height);
	g_MuCameraParas.m_pDcamBuffer=(unsigned short*)malloc(width*height*2);

	int m_PixelBytes=1;
	if (mFlipMirrorParas.iMirrorDst)
		delete mFlipMirrorParas.iMirrorDst;
	if (mFlipMirrorParas.iMirrorSrc)
		delete mFlipMirrorParas.iMirrorSrc;
	if (mFlipMirrorParas.iFlipDst)
		delete mFlipMirrorParas.iFlipDst;
	if (mFlipMirrorParas.iFlipSrc)
		delete mFlipMirrorParas.iFlipSrc;
	mFlipMirrorParas.iCount=height*width*m_PixelBytes;
	mFlipMirrorParas.iMirrorDst=new int[mFlipMirrorParas.iCount];
	mFlipMirrorParas.iMirrorSrc=new int[mFlipMirrorParas.iCount];
	mFlipMirrorParas.iFlipSrc=new int[mFlipMirrorParas.iCount];
	mFlipMirrorParas.iFlipDst=new int[mFlipMirrorParas.iCount];
	int iCount=0;
	for (int j=height-1;j>=0;j--)
	{
		for (int i=0;i<width;i++)
		{
			for (int k=0;k<m_PixelBytes;k++)
			{		
				*(mFlipMirrorParas.iMirrorSrc+iCount)=(width*j+i)*m_PixelBytes+k;
				*(mFlipMirrorParas.iMirrorDst+iCount)=(width*(j+1)-i-1)*m_PixelBytes+k;
				*(mFlipMirrorParas.iFlipSrc+iCount)=(width*j+i)*m_PixelBytes+k,
				*(mFlipMirrorParas.iFlipDst+iCount)=(width*(height-j-1)+i)*m_PixelBytes+k;
				iCount++;
			}
		}
	}
}

bool CDcamVideo::GrabBuffer(int& pwidth,int& pheight)
{
	EnterCriticalSection(&m_crtSec);
	if (pwidth!=g_MuCameraParas.m_pWidth[g_MuCameraParas.m_iBinningIndx]&&
		pheight!=g_MuCameraParas.m_pHeight[g_MuCameraParas.m_iBinningIndx])
	{
		pwidth=g_MuCameraParas.m_pWidth[g_MuCameraParas.m_iBinningIndx];
		pheight=g_MuCameraParas.m_pHeight[g_MuCameraParas.m_iBinningIndx];
		UpdateBufferSize(pwidth,pheight);
		g_MuCameraParas.m_DcamSize=pwidth*pheight*2;//固定16位大小
	}

	_DWORD	status;
	HDCAM hCamera=g_MuCameraParas.m_iCurHandle;
	dcam_getstatus( hCamera, &status );
	if(status!=DCAM_STATUS_BUSY) //return false;//不在状态
	{
		if (status==DCAM_STATUS_UNSTABLE)
			dcam_precapture( hCamera, DCAM_CAPTUREMODE_SEQUENCE);
		else if (status==DCAM_STATUS_STABLE)
			dcam_allocframe( hCamera, 3 );//这里暂时定3帧
		dcam_capture(hCamera);
	}
	else
	{
		//查询状态
		long	newestFrameIndex, totalFrameCount;
		if( dcam_gettransferinfo( hCamera, &newestFrameIndex, &totalFrameCount ) )
		{
			if( g_MuCameraParas.m_NowFrameIndex	!= newestFrameIndex
				|| g_MuCameraParas.m_TotalFrame	!= totalFrameCount )
			{
				g_MuCameraParas.m_NowFrameIndex	= newestFrameIndex;
				g_MuCameraParas.m_TotalFrame	= totalFrameCount;
				long	rowbytes=2048;
				unsigned short* pFirstBuffer=g_MuCameraParas.m_pDcamBuffer;
				dcam_lockdata(hCamera, (void**)&(pFirstBuffer), &rowbytes, newestFrameIndex );//这个好好学，很好用！！
				memcpy(g_MuCameraParas.m_pDcamBuffer,pFirstBuffer,g_MuCameraParas.m_DcamSize);//pFirstBuffer改变了地址，而且不能被Delete
				dcam_unlockdata(hCamera);
				LeaveCriticalSection(&m_crtSec);
				return TRUE;
			}
		}
	}
	LeaveCriticalSection(&m_crtSec);
	return false;
}

void CDcamVideo::RenderVideoData()
{
	m_VideoFormate.Vheight=height;
	m_VideoFormate.Vwidth=width;
	long t_Size=height*width;//dcam固定16转8位
	m_VideoFormate.VwBit = g_MuCameraParas.m_iColorChannel * 8;// 16;
	m_VideoFormate.Vsize=t_Size;
	m_VideoFormate.Vuser=(long)g_MuCameraParas.m_iCurHandle;//标识相机

	//memcpy(g_MuCameraParas.m_pBuffer,g_MuCameraParas.m_pDcamBuffer,t_Size);
	//下面转8位数据;
	unsigned char *p_scr=(unsigned char*)(g_MuCameraParas.m_pDcamBuffer)+1;
	unsigned char *p_Dest=g_MuCameraParas.m_pBuffer;
	for (long i=0;i<t_Size;i++)
	{
		*p_Dest=*p_scr;
		p_Dest++;
		p_scr+=2;
	}//这个要用5ms
	LPBYTE middleBuffer=NULL;
	if (bMirror||bFlip)
		middleBuffer=(LPBYTE)malloc(m_VideoFormate.Vsize);
		
	if (bMirror&&bFlip)
	{			
		MirrorConvert(g_MuCameraParas.m_pBuffer,middleBuffer,m_VideoFormate);
		FlipConvert(middleBuffer,g_MuCameraParas.m_pBuffer,m_VideoFormate);
		delete(middleBuffer);
	}
	else if (bMirror)
	{
		memcpy(middleBuffer,g_MuCameraParas.m_pBuffer,m_VideoFormate.Vsize);
		MirrorConvert(middleBuffer,g_MuCameraParas.m_pBuffer,m_VideoFormate);
		delete(middleBuffer);
	}	
	else if (bFlip)
	{
		memcpy(middleBuffer,g_MuCameraParas.m_pBuffer,m_VideoFormate.Vsize);
		FlipConvert(middleBuffer,g_MuCameraParas.m_pBuffer,m_VideoFormate);
		delete(middleBuffer);
	}
	if(m_VideoRender)
		m_VideoRender->Renderer(&m_VideoFormate,g_MuCameraParas.m_pBuffer);
}

bool CDcamVideo::SetFlip(bool pFlip)
{
	if (!bOpen)
		return false;
	EnterCriticalSection(&m_crtSec);
	bFlip=pFlip;
	LeaveCriticalSection(&m_crtSec);
	Notify(this,Notify_CarlVideo_Flip,0,&bFlip);
	return true;
}

bool CDcamVideo::SetMirror(bool pMirror)
{
	if (!bOpen)
		return false;
	EnterCriticalSection(&m_crtSec);
	bMirror=pMirror;
	LeaveCriticalSection(&m_crtSec);
	Notify(this,Notify_CarlVideo_Mirror,0,&pMirror);
	return true;
}

void CDcamVideo::MirrorConvert(LPBYTE Src,LPBYTE Dst,HVideoHeader mPara)
{
	if (mFlipMirrorParas.iCount>0)
	{
		for (int i=0;i<mFlipMirrorParas.iCount;i++)
			Dst[mFlipMirrorParas.iMirrorDst[i]]=Src[mFlipMirrorParas.iMirrorSrc[i]];
		return;
	}
	int with=mPara.Vwidth;
	int height=mPara.Vheight;
	int m_PixelBytes=mPara.VwBit/8;
	for (int j=height-1;j>=0;j--)
	{
		for (int i=0;i<with;i++)
		{
			for (int k=0;k<m_PixelBytes;k++)
			{
				int indexSrc=(with*j+i)*m_PixelBytes+k,
					indexDst=(with*(j+1)-i-1)*m_PixelBytes+k;
				Dst[indexDst]=Src[indexSrc];
			}
		}
	}
}

void CDcamVideo::FlipConvert(LPBYTE Src,LPBYTE Dst,HVideoHeader mPara)
{
	if (mFlipMirrorParas.iCount>0)
	{
		for (int i=0;i<mFlipMirrorParas.iCount;i++)
			Dst[mFlipMirrorParas.iFlipDst[i]]=Src[mFlipMirrorParas.iFlipSrc[i]];
		return;
	}
	int with=mPara.Vwidth;
	int height=mPara.Vheight;
	int m_PixelBytes=mPara.VwBit/8;
	for (int j=height-1;j>=0;j--)
	{
		for (int i=0;i<with;i++)
		{
			for (int k=0;k<m_PixelBytes;k++)
			{
				int indexSrc=(with*j+i)*m_PixelBytes+k,
					indexDst=(with*(height-j-1)+i)*m_PixelBytes+k;
				Dst[indexDst]=Src[indexSrc];
			}
		}
	}
}

