#include "StdAfx.h"
#include "VideoManager.h"
#include "TUCamVideo.h"

CVideoManager::CVideoManager(void)
{
  m_itApi.pstrConfigPath = "";
  m_itApi.uiCamCount = 0;

  // 获取当前地址
  CString strPath;
  GetCurrentDirectory(MAX_PATH, strPath.GetBuffer(MAX_PATH));

  m_itApi.pstrConfigPath = (LPSTR)(LPCTSTR)strPath;
  m_itApi.uiCamCount = 0;
}

CVideoManager::~CVideoManager(void)
{
  TUCAM_Api_Uninit();
	m_CameraDevice.clear();
}

void CVideoManager::SearchCameras()
{
	if (m_CameraDevice.size()>0)
	{
		return;
	}

  // 初始化SDK API环境
  TUCAMRET res = TUCAM_Api_Init(&m_itApi);
  if (res <= 0)
  {
    return;
  }
	long nCamera = m_itApi.uiCamCount;
	for (int i=0;i<nCamera;i++)
	{
		CString xtmp;

    TUCAM_OPEN m_opCam;
    m_opCam.uiIdxOpen = i;
    TUCAM_Dev_Open(&m_opCam);

    if (m_opCam.hIdxTUCam)
    {
      TUCAM_VALUE_INFO valInfo;
      valInfo.nID = TUIDI_CAMERA_MODEL;
      if (TUCAMRET_SUCCESS == TUCAM_Dev_GetInfo(m_opCam.hIdxTUCam, &valInfo))
      {
        xtmp = valInfo.pText;
      }
      TUCAM_Dev_Close(m_opCam.hIdxTUCam);
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
	if (pIndex>=(int)m_CameraDevice.size())
		return 0;
	if(m_CameraDevice[pIndex].s_Isopen)
		return m_CameraDevice[pIndex].s_VideoDevice;
	if (!m_CameraDevice[pIndex].s_VideoDevice)
	{
		m_CameraDevice[pIndex].s_VideoDevice=new TUCamVideo(m_CameraDevice[pIndex].s_CameraIndex,
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
	for (int i=0;i<(int)m_CameraDevice.size();i++)
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
	for (int i=0;i<(int)m_CameraDevice.size();i++)
	{
		if(pName==GetCameraName(i))
			return(OpenCamera(i));
	}
	return 0;
}

void CVideoManager::CloseCamera( int pIndex )
{
	if(pIndex<(int)m_CameraDevice.size())
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

