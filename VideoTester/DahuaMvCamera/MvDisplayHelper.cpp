#include "stdafx.h"

#include "MvDisplayHelper.h"
#include "GenICam/System.h"

namespace Dahua{

CMvDisplayHelper::CMvDisplayHelper(const GenICam::ICameraPtr& sptrCamera, IMvCameraRender* pRender)
	: m_sptrImageConvertHelper(new CMvImageConvertHelper(sptrCamera))
	, m_displayThread(Infra::CThreadLite::ThreadProc(&CMvDisplayHelper::displayThreadProc, this), "DisplayThread")
	, m_iWidth(0)
	, m_iHeight(0)
{
	GenICam::IImageFormatControlPtr sptrImageFormatCtrl = GenICam::CSystem::getInstance().createImageFormatControl(sptrCamera);
	GenICam::CIntNode nodeWidth = sptrImageFormatCtrl->width();
	if ( nodeWidth.isValid() )
	{
		int64_t i64Val = 0;
		nodeWidth.getValue(i64Val);
		m_iWidth = (uint32_t)i64Val;
	}

	GenICam::CIntNode nodeHeight = sptrImageFormatCtrl->height();
	if ( nodeHeight.isValid() )
	{
		int64_t i64Val = 0;
		nodeHeight.getValue(i64Val);
		m_iHeight = (uint32_t)i64Val;
	}
	m_pMvCameraRender = pRender;
	m_bPause = false;
}

CMvDisplayHelper::~CMvDisplayHelper()
{

}

bool CMvDisplayHelper::isValid()
{
	if ( NULL == m_sptrImageConvertHelper )
	{
		return false;
	}

	return m_sptrImageConvertHelper->isValid();
}

bool CMvDisplayHelper::startDisplay(void* pHandle, int iDispalyRate)
{
	if ( !m_sptrImageConvertHelper->isValid() )
	{
		return false;
	}

	bool bOpenRet = m_sptrImageConvertHelper->startConvert(iDispalyRate);
	if ( !bOpenRet )
	{
		return false;
	}

	/*m_render.setHandle(pHandle);
	bOpenRet = m_render.open();
	if ( !bOpenRet )
	{
		m_sptrImageConvertHelper->stopConvert();
		return false;
	}*/

	m_bRunning = true;
	m_bPause = false;

	return m_displayThread.createThread();
}

bool CMvDisplayHelper::stopDisplay()
{
	m_bRunning	= false;
	m_bPause	= true;

	if ( NULL != m_sptrImageConvertHelper )
	{
		m_sptrImageConvertHelper->stopConvert();
	}

	//m_render.close();
	m_displayThread.destroyThread();

	int i = 0;
	while (!m_bFinished)
	{
		i++;
		if (i>100000)
		{
			break;
		}
	}

	return true;
}

bool CMvDisplayHelper::pauseDisplay()
{
	m_bPause = true;
	return true;
}

void CMvDisplayHelper::enableRateCtrl(bool bEnable)
{
	if ( NULL != m_sptrImageConvertHelper )
	{
		m_sptrImageConvertHelper->enableRateCtrl(bEnable);
	}
}

void CMvDisplayHelper::displayThreadProc(Infra::CThreadLite&)
{
	m_bFinished = false;
	while (m_bRunning)
	{
		if ( NULL == m_sptrImageConvertHelper || m_bPause)
		{
			continue;
		}

		CMvImageBufPtr sptrComvertedImage = m_sptrImageConvertHelper->getConvertedImage();
		if ( NULL != sptrComvertedImage )
		{
			if(m_pMvCameraRender)
				m_pMvCameraRender->onCameraRender(sptrComvertedImage->bufPtr(), sptrComvertedImage->imageWidth(), sptrComvertedImage->imageHeight()
					, sptrComvertedImage->imagePixelFormat());
			//回调函数回去？
			//m_render.display(sptrComvertedImage->bufPtr(), sptrComvertedImage->imageWidth(), sptrComvertedImage->imageHeight(), sptrComvertedImage->imagePixelFormat());
		}		
	}
	m_bFinished = true;
}

bool CMvDisplayHelper::ClearBuf()
{
	m_sptrImageConvertHelper->ClearBuf();
	return true;
}

} // namespace Dahua
