#ifndef __DAHUA_MULTICAMERAS_DEMO_MVIMAGE_CONVERT_H__
#define __DAHUA_MULTICAMERAS_DEMO_MVIMAGE_CONVERT_H__

#include "MvImageBuf.h"
#include "GenICam/System.h"
#include "Infra/Thread.h"
#include "Infra/Mutex.h"
#include "Infra/Semaphore.h"
#include "Memory/SharedPtr.h"

#include <list>

namespace Dahua{

#define TIMESTAMPFREQUENCY 125000000	//大华相机的时间戳频率固定为125,000,000Hz | dahua camera timestamp permanent as 125000000Hz 

class CMvImageConvertHelper
{
public:
	CMvImageConvertHelper(const GenICam::ICameraPtr& sptrCamera);
	~CMvImageConvertHelper();

public:
	bool isValid();

	bool startConvert(int iConvertRate);

	bool stopConvert();

	bool ClearBuf();

	CMvImageBufPtr getConvertedImage();

	void enableRateCtrl(bool bEnable);

	void setDisplayFPS(int nFPS);

private:
	void grabbingThreadProc(Infra::CThreadLite&);

	bool isTimeToDisplay();

	bool convertFrame(const GenICam::CFrame& frameBuf, CMvImageBufPtr& sptrConvertedImage);

	int findMatchCode(int iCode);

	uint32_t calcRateCtrlNum();

	void addConvertedImage(const CMvImageBufPtr& sptrConvertedImage);

private:
	typedef std::list<CMvImageBufPtr> ImageListType;

private:
	GenICam::ICameraPtr				m_sptrCamera;
	GenICam::IStreamSourcePtr		m_sptrStreamSrc;
	ImageListType					m_listImages;
	bool							m_bRunning;
	bool							m_bFinished;
	Infra::CThreadLite				m_grabbingThread;
	Infra::CMutex					m_mutexQue;
	Infra::CSemaphore				m_semQue;
	uint32_t						m_iControlNum;
	bool							m_bEnableRateCtrl;

	Infra::CMutex					m_mxTime;
	int							    m_dDisplayInterval;         // 显示间隔			| diaplay interval
	uintmax_t						m_nTimestampFreq;           // 时间戳频率		| time stamp frequency
	uintmax_t						m_nFirstFrameTime;          // 第一帧的时间戳	| frist frame time stamp 
	uintmax_t						m_nLastFrameTime;           // 上一帧的时间戳	| previous frame time stamp
};

typedef Memory::TSharedPtr<CMvImageConvertHelper> CMvImageConvertHelperPtr;

} // namespace Dahua

#endif // __DAHUA_MULTICAMERAS_DEMO_MVIMAGE_CONVERT_H__
