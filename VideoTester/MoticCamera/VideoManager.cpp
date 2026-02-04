#include "StdAfx.h"
#include "VideoManager.h"
//#include "MoticUCam.h"
#include "DMoticCamera.h"


CVideoManager::CVideoManager(void)
{

}


CVideoManager::~CVideoManager(void)
{
	m_CameraDevice.clear();
}

void CVideoManager::SearchCameras()
{
	//Step1 Search Motic camera
	//如果有需要，也可以写成配置文件方式加载某些相机

	if (m_CameraDevice.size()>0)
	{
		return;
	}

	MUCam_Handle hCamera = MUCam_findCamera();//Find Cameras
	CString strName = TEXT("");
	int camIndex=0;//相机编号

	while(hCamera)
	{
		
		switch(MUCam_getType(hCamera))
		{     
		case MUCAM_TYPE_MC1001:
			strName = TEXT("MC1001");
			break;
		case  MUCAM_TYPE_MC2001:
			strName = TEXT("MC2001");
			break;
		case MUCAM_TYPE_MC3001:
			strName = TEXT("MC3001");
			break;
		case MUCAM_TYPE_MC2001B:
			strName = TEXT("MC2001B");
			break;
		case MUCAM_TYPE_MC1002:
			strName = TEXT("MC1002");
			break;
		case MUCAM_TYPE_MC2002:
			strName = TEXT("MC2002");
			break;
		case MUCAM_TYPE_MA205:
			strName = TEXT("MA205");
			break;
		case MUCAM_TYPE_MA285:
			strName = TEXT("MA285");
			break;
		case MUCAM_TYPE_MA252:
			strName = TEXT("MA252");
			break;	 
		case MUCAM_TYPE_MC5001:
			strName = TEXT("MC5001");
			break;
		case MUCAM_TYPE_MC3111:
			strName = TEXT("MC3111");
			break;
		case MUCAM_TYPE_MC3222:
			strName = TEXT("MC3222");
			break;	  
		case MUCAM_TYPE_MA282:
			strName = TEXT("MA282");
			break;
		case MUCAM_TYPE_VISION_3001:
			strName = TEXT("MC3001");
			break;
		case MUCAM_TYPE_VISION_3002:
			strName = TEXT("MC3002");
			break;
		default:
			strName = TEXT("Unknow Device");
			break;
		}

		SVideoDevice t_videodevice;

		t_videodevice.s_CameraHandle=hCamera;
		t_videodevice.s_CameraName=strName;
		t_videodevice.s_Isopen=false;
		t_videodevice.s_VideoDevice=0;

		m_CameraDevice.push_back(t_videodevice);
		
		hCamera = MUCam_findCamera();//find the next camera
	}

}

HVideoDevice* CVideoManager::OpenCamera( int pIndex )
{
	if (pIndex>=m_CameraDevice.size())
		return 0;
	if(m_CameraDevice[pIndex].s_Isopen)
		return m_CameraDevice[pIndex].s_VideoDevice;
	if (!m_CameraDevice[pIndex].s_VideoDevice)
	{
		m_CameraDevice[pIndex].s_VideoDevice=new MucamVideo(
			m_CameraDevice[pIndex].s_CameraHandle,m_CameraDevice[pIndex].s_CameraName);
		m_CameraDevice[pIndex].s_VideoDevice->Attach(this);
	}
	if (!m_CameraDevice[pIndex].s_Isopen)
	{
		m_CameraDevice[pIndex].s_VideoDevice->OpenCamera();
	}
	return m_CameraDevice[pIndex].s_VideoDevice;
}

HVideoDevice* CVideoManager::OpenCamera( CString pName )
{
	for (int i=0;i<m_CameraDevice.size();i++)
	{
		if(pName==GetCameraName(i))
			return(OpenCamera(i));
	}

	return 0;
}

void CVideoManager::CloseCamera( int pIndex )
{
	if(pIndex<m_CameraDevice.size())
	{
		if(m_CameraDevice[pIndex].s_Isopen)
		{
			m_CameraDevice[pIndex].s_VideoDevice->Pause();
			m_CameraDevice[pIndex].s_VideoDevice->CloseCamera();
			delete(m_CameraDevice[pIndex].s_VideoDevice);
			m_CameraDevice[pIndex].s_VideoDevice=NULL;
		}
		m_CameraDevice[pIndex].s_Isopen=false;
	}
}

int CVideoManager::OnSubjectNotified(IPtnSubject * pSubject, int ID,long wParam,
	void* pParam,float fParam,void* mParam)
{
	for (int i=0;i<m_CameraDevice.size();i++)
	{
		if (m_CameraDevice[i].s_VideoDevice==(HVideoDevice*)pSubject)
		{
			switch(ID)
			{
				case Notify_CarlVideo_OpenCamera:
					m_CameraDevice[i].s_Isopen=true;
					break;
				case Notify_CarlVideo_CloseCamera:
					m_CameraDevice[i].s_Isopen=false;
					break;
			}
			break;
		}
	}
	Notify(this,ID,wParam,pParam,fParam,mParam);
	return 0;
}
