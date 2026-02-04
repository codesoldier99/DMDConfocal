#pragma once
#include "CarlVideo.h"
#include <vector>
#include "MoticUCam.h"

typedef struct 
{
	MUCam_Handle s_CameraHandle;
	CString s_CameraName;
	HVideoDevice* s_VideoDevice;
	bool s_Isopen;
}SVideoDevice;


class CVideoManager : public HVideoManager
{
public:
	CVideoManager(void);
	~CVideoManager(void);
	void SearchCameras();
	int GetCameraCount() {return m_CameraDevice.size();};
	CString GetCameraName(int pIndex){return m_CameraDevice[pIndex].s_CameraName;};
	HVideoDevice* GetCamerDevice(int pIndex){return m_CameraDevice[pIndex].s_VideoDevice;};
	HVideoDevice* OpenCamera(int pIndex);
	virtual HVideoDevice* OpenCamera(CString pName);
	void CloseCamera(int pIndex);
	virtual int OnSubjectNotified(IPtnSubject * pSubject, int ID,long wParam = 0,
		void* pParam = 0,float fParam=0.0f,void* mParam=0);
private:
	std::vector<SVideoDevice>  m_CameraDevice;
};

