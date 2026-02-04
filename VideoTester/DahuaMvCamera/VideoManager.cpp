#include "StdAfx.h"
#include "VideoManager.h"
#include "GenICam/System.h"
#include "MvCamera.h"

using namespace Dahua;

CVideoManager::CVideoManager(void)
{
}


CVideoManager::~CVideoManager(void)
{
	for (int i= 0;i< m_CameraDevice.size();i++)
	{
		if(m_CameraDevice[i].s_VideoDevice)
			m_CameraDevice[i].s_VideoDevice->CloseCamera();
	}
	m_CameraDevice.clear();
}

void CVideoManager::SearchCameras()
{
	if (m_CameraDevice.size()>0)
	{
		return;
	}

	Infra::TVector<GenICam::ICameraPtr> vecCameras;
	GenICam::CSystem::getInstance().discovery(vecCameras);

	size_t iShowCameraNum = vecCameras.size();
	for (size_t iIndex = 0; iIndex < iShowCameraNum; ++iIndex)
	{
		//std::string midName = ((GenICam::ICameraPtr)vecCameras[iIndex])->getName();
		std::string midKey = ((GenICam::ICameraPtr)vecCameras[iIndex])->getKey();
		//int midV = ((GenICam::ICameraPtr)vecCameras[iIndex])->getType();
		//CString strMid1(midName.c_str());
		CString strMid2(midKey.c_str());
		if (strMid2.GetLength() > 5)
			strMid2 = strMid2.Right(5);
		CString strName = L"";
		//strName.Format(L"%s-%s-%d", strMid1, strMid2, iIndex);
		strName.Format(L"Dh%s-%d", strMid2, iIndex);

		SVideoDevice mV;
		mV.s_CameraName  = strName;
		mV.s_CameraIndex = iIndex;//记录当前相机编号
		mV.s_Isopen      = false;
		mV.s_VideoDevice = new CMvCamera(vecCameras[iIndex], iIndex, strName);
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
		return NULL;
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

