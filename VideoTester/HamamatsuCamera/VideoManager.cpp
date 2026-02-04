#include "StdAfx.h"
#include "VideoManager.h"
#include "DcamVideo.h"


CVideoManager::CVideoManager(void)
{
}


CVideoManager::~CVideoManager(void)
{
	m_CameraDevice.clear();
}

void CVideoManager::SearchCameras()
{
	if (m_CameraDevice.size()>0)
	{
		return;
	}

	long nCamera;
	if (!dcam_init(NULL,&nCamera)) //不知咋的，这里初始化时间很长。。。
		return;
	for (int i=0;i<nCamera;i++)
	{
		CString xtmp;
		xtmp.Format(L"%s%d",L"CC11440-",i);

		HDCAM hdcam;
		dcam_open(&hdcam,i);
		if (hdcam!=NULL)
		{
			char cbModel[ 64 ];
			VERIFY( dcam_getstring( hdcam, DCAM_IDSTR_MODEL,cbModel,sizeof(cbModel)));
			xtmp= cbModel;
			dcam_close(hdcam);
		}

		SVideoDevice mV;
		mV.s_CameraName=xtmp;
		mV.s_CameraIndex=i;//记录当前相机编号
		mV.s_Isopen=false;
		mV.s_VideoDevice=0;

		m_CameraDevice.push_back(mV);
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
		m_CameraDevice[pIndex].s_VideoDevice=new CDcamVideo(m_CameraDevice[pIndex].s_CameraIndex,
			m_CameraDevice[pIndex].s_CameraName);
		m_CameraDevice[pIndex].s_VideoDevice->Attach(this);
	}
	if (!m_CameraDevice[pIndex].s_Isopen)
		m_CameraDevice[pIndex].s_VideoDevice->OpenCamera();
	return m_CameraDevice[pIndex].s_VideoDevice;
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
			m_CameraDevice[pIndex].s_VideoDevice->CloseCamera();
			delete(m_CameraDevice[pIndex].s_VideoDevice);
			m_CameraDevice[pIndex].s_VideoDevice=NULL;
		}
		m_CameraDevice[pIndex].s_Isopen=false;
	}

}

