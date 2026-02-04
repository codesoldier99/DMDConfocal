#ifndef __DAHUA_MULTICAMERAS_DEMO_MVDISPLAY_HELPER_H__
#define __DAHUA_MULTICAMERAS_DEMO_MVDISPLAY_HELPER_H__

#include "MvImageConvertHelper.h"
#include "Infra/Thread.h"
#include "Memory/SharedPtr.h"
//#include "MvRender.h"

namespace Dahua{

	class IMvCameraRender
	{
	public:
		virtual ~IMvCameraRender() {}

	public:
		virtual void onCameraRender(uint8_t* pData, int iWidth, int iHeight, uint64_t iPixelFormat) = 0;
	};

class CMvDisplayHelper
{
public:
	CMvDisplayHelper(const GenICam::ICameraPtr& sptrCamera, IMvCameraRender* pRender = NULL);
	~CMvDisplayHelper();

public:
	bool isValid();

	bool startDisplay(void* pHandle, int iDispalyRate);

	bool stopDisplay();

	bool pauseDisplay();

	bool ClearBuf();

	void enableRateCtrl(bool bEnable);

private:
	void displayThreadProc(Infra::CThreadLite&);

private:
	CMvImageConvertHelperPtr		m_sptrImageConvertHelper;
	Infra::CThreadLite				m_displayThread;
	bool							m_bRunning;
	bool							m_bFinished;
	bool							m_bPause;
	//CRender							m_render;
	IMvCameraRender*				m_pMvCameraRender;
	uint32_t						m_iWidth;
	uint32_t						m_iHeight;
};

typedef Memory::TSharedPtr<CMvDisplayHelper> CMvDisplayHelperPtr;

} // namespace Dahua

#endif // __DAHUA_MULTICAMERAS_DEMO_MVDISPLAY_HELPER_H__
