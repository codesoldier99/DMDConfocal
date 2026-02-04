#include "stdafx.h"

#include "MvImageConvertHelper.h"
#include "GenICam/StreamSource.h"
#include "GenICam/PixelType.h"
#include "GenICam/System.h"
#include "Media/Imageconvert.h"
#include "Infra/Guard.h"

#define RATE_CALC_TIME (3000)
#define DEFAULT_SHOW_RATE (30)

#define CHECK_IMGCNV_STATUS(vxOp) \
	do {\
			if (IMGCNV_SUCCESS != vxOp) \
			{ \
				return false; \
			} \
	} while (0)

namespace Dahua{

/**
* @enum gFormatTransferTbl
* @brief 图像像素格式 | Image pixel format
* @attention 无
*/
static uint32_t gFormatTransferTbl[] =
{	
	// Mono Format
	GenICam::gvspPixelMono1p,
	GenICam::gvspPixelMono8,
	GenICam::gvspPixelMono10,                
	GenICam::gvspPixelMono10Packed,			
	GenICam::gvspPixelMono12,			
	GenICam::gvspPixelMono12Packed,

	// Bayer Format			
	GenICam::gvspPixelBayRG8,                
	GenICam::gvspPixelBayGB8,                
	GenICam::gvspPixelBayBG8,                              
	GenICam::gvspPixelBayRG10,               
	GenICam::gvspPixelBayGB10,               
	GenICam::gvspPixelBayBG10,                            
	GenICam::gvspPixelBayRG12,               
	GenICam::gvspPixelBayGB12,          
	GenICam::gvspPixelBayBG12,                      
	GenICam::gvspPixelBayRG10Packed,			
	GenICam::gvspPixelBayGB10Packed,			
	GenICam::gvspPixelBayBG10Packed,	        
	GenICam::gvspPixelBayRG12Packed,			
	GenICam::gvspPixelBayGB12Packed,			
	GenICam::gvspPixelBayBG12Packed,			
	GenICam::gvspPixelBayRG16,    
	GenICam::gvspPixelBayGB16, 
	GenICam::gvspPixelBayBG16,
	GenICam::gvspPixelBayRG10p,              
	GenICam::gvspPixelBayRG12p,             

	GenICam::gvspPixelMono1c,

	// RGB Format
	GenICam::gvspPixelRGB8,				
	GenICam::gvspPixelBGR8,            

	// YVR Format
	GenICam::gvspPixelYUV411_8_UYYVYY,     
	GenICam::gvspPixelYUV422_8_UYVY,       
	GenICam::gvspPixelYUV422_8,         
	GenICam::gvspPixelYUV8_UYV,                    
};
#define gFormatTransferTblLen	sizeof(gFormatTransferTbl)/sizeof(gFormatTransferTbl[0])

CMvImageConvertHelper::CMvImageConvertHelper(const GenICam::ICameraPtr& sptrCamera)
	: m_sptrCamera(sptrCamera)
	, m_bRunning(false)
	, m_grabbingThread(Infra::CThreadLite::ThreadProc(&CMvImageConvertHelper::grabbingThreadProc, this), "GrabbingThread")
	, m_semQue(0)
	, m_iControlNum(0)
	, m_dDisplayInterval(0)
	, m_nTimestampFreq(TIMESTAMPFREQUENCY)
	, m_nFirstFrameTime(0)
	, m_nLastFrameTime(0)
	, m_bEnableRateCtrl(true)
{
	m_sptrStreamSrc = GenICam::CSystem::getInstance().createStreamSource(sptrCamera);
	setDisplayFPS(30);
}

CMvImageConvertHelper::~CMvImageConvertHelper()
{
	stopConvert();
}

bool CMvImageConvertHelper::isValid()
{
	return NULL == m_sptrStreamSrc ? false : true;
}

bool CMvImageConvertHelper::startConvert(int iConvertRate)
{
	if ( NULL == m_sptrStreamSrc )
	{
		return false;
	}

	if ( m_sptrStreamSrc->isGrabbing() )
	{
		return false;
	}

	bool bStartRet = m_sptrStreamSrc->startGrabbing();
	if ( !bStartRet )
	{
		return false;
	}

	m_bRunning = true;

	return m_grabbingThread.createThread();
}

bool CMvImageConvertHelper::stopConvert()
{
	m_bRunning = false;

	if ( NULL == m_sptrStreamSrc )
	{
		return false;
	}

	if ( !m_sptrStreamSrc->isGrabbing() )
	{
		return true;
	}	

	{
		Infra::CGuard guard(m_mutexQue);
		m_listImages.clear();
		m_semQue.post();
	}

	m_grabbingThread.destroyThread();

	m_sptrStreamSrc->stopGrabbing();

	int i = 0;
	while (!m_bFinished)
	{
		i++;
		if (i > 100000)
		{
			break;
		}
	}

	return true;
}

bool CMvImageConvertHelper:: ClearBuf()
{
	m_listImages.clear();
	return true;
}

CMvImageBufPtr CMvImageConvertHelper::getConvertedImage()
{
	CMvImageBufPtr sptrConvertedImage;

	while ( m_bRunning )
	{
		m_mutexQue.enter();

		if ( m_listImages.empty() )
		{
			m_mutexQue.leave();
			m_semQue.pend();

			continue;
		}
		else
		{
			sptrConvertedImage = m_listImages.front();
			m_listImages.pop_front();

			m_mutexQue.leave();

			break;
		}		
	}
	
	return sptrConvertedImage;
}

void CMvImageConvertHelper::enableRateCtrl(bool bEnable)
{
	m_bEnableRateCtrl = bEnable;
}

void CMvImageConvertHelper::grabbingThreadProc(Infra::CThreadLite&)
{
	m_bFinished = false;
	while ( m_bRunning )
	{
		if ( NULL == m_sptrStreamSrc)
		{
			continue;
		}

		GenICam::CFrame frameBuf;
		m_sptrStreamSrc->getFrame(frameBuf, 500);
		if ( !frameBuf.valid() )
		{
			continue;
		}

		/*if (!isTimeToDisplay())
		{
			continue;
		}*/

		CMvImageBufPtr sptrConvertedImage;
		bool bRet = convertFrame(frameBuf, sptrConvertedImage);
		if ( bRet )
		{
			addConvertedImage(sptrConvertedImage);
		}
	}
	m_bFinished = true;
	m_iControlNum = 0;
}

bool CMvImageConvertHelper::isTimeToDisplay()
{
	Infra::CGuard guard(m_mxTime);

	// 不显示
	// don't display
	if (m_dDisplayInterval <= 0)
	{
		return false;
	}

	// 时间戳频率获取失败, 默认全显示. 这种情况理论上不会出现
	// Time stamp frequency acquisition failed, default full display. This situation will not appear in theory
	if (m_nTimestampFreq <= 0)
	{
		return true;
	}

	// 第一帧必须显示
	// The first frame must be displayed
	if (m_nFirstFrameTime == 0 || m_nLastFrameTime == 0)
	{
		m_nFirstFrameTime = Infra::CTime::getCurrentMicroSecond() * 1000;
		m_nLastFrameTime = m_nFirstFrameTime;
		
		warnf("set m_nFirstFrameTime: %I64d\n", m_nFirstFrameTime);
		return true;
	}
	uint64_t nCurTimeTmp = Infra::CTime::getCurrentMicroSecond() * 1000;
	uint64_t nAcquisitionInterval = nCurTimeTmp - m_nLastFrameTime;

	if (nAcquisitionInterval > m_dDisplayInterval)
	{
		m_nLastFrameTime = nCurTimeTmp;
		return true;
	}

	// 当前帧相对于第一帧的时间间隔
	// Time interval between the current frame and the first frame
	uint64_t nPre = (m_nLastFrameTime - m_nFirstFrameTime) % m_dDisplayInterval;
	if (nPre + nAcquisitionInterval > m_dDisplayInterval)
	{
		m_nLastFrameTime = nCurTimeTmp;
		return true;
	}

	return false;
}

void CMvImageConvertHelper::setDisplayFPS(int nFPS)
{
	if (nFPS > 0)
	{
		Infra::CGuard guard(m_mxTime);
		m_dDisplayInterval = 1000 * 1000 * 1000.0 / nFPS;
	}
	else
	{
		Infra::CGuard guard(m_mxTime);
		m_dDisplayInterval = 0;
	}
}


bool CMvImageConvertHelper::convertFrame(const GenICam::CFrame& frameBuf, CMvImageBufPtr& sptrConvertedImage)
{
	int iFormat = findMatchCode(frameBuf.getImagePixelFormat());
	if (iFormat < 0)
	{
		return false;
	}

	CMvImageBufPtr sptrImageBuf(new CMvImageBuf(frameBuf));
	if(!sptrImageBuf)
	{
		TRACE("sptrImageBuf is null.\n");
		return false;
	}

	// Mono8无需转换直接源数据显示
	// Mono8 does not need to convert direct source data display
	if(frameBuf.getImagePixelFormat() == GenICam::gvspPixelMono8)
	{
		memcpy(sptrImageBuf->bufPtr(), frameBuf.getImage(), frameBuf.getImageSize());
	}
	else
	{
		uint8_t* pSrcData = new(std::nothrow) uint8_t[frameBuf.getImageSize()];
		if(pSrcData)
		{
			memcpy(pSrcData, frameBuf.getImage(), frameBuf.getImageSize());
		}
		else
		{
			TRACE("pSrcData is null.\n");
			return false;		
		}

		int dstDataSize = 0;
		IMGCNV_SOpenParam openParam;
		openParam.width = sptrImageBuf->imageWidth();
		openParam.height = sptrImageBuf->imageHeight();
		openParam.paddingX = sptrImageBuf->imagePaddingX();
		openParam.paddingY = sptrImageBuf->imagePaddingY();
		openParam.dataSize = sptrImageBuf->bufSize();
		openParam.pixelForamt = sptrImageBuf->imagePixelFormat();

		IMGCNV_EErr status = IMGCNV_ConvertToBGR24(pSrcData, &openParam, sptrImageBuf->bufPtr(), &dstDataSize);
		if (IMGCNV_SUCCESS != status)
		{
			delete[] pSrcData;
			TRACE("IMGCNV_open is failed.err=%d\n",status);
			return false;
		}
		delete[] pSrcData;
	}

	sptrConvertedImage = sptrImageBuf;
	return true;
}

int CMvImageConvertHelper::findMatchCode(int iCode)
{
	for ( int i = 0; i < gFormatTransferTblLen; ++i )
	{
		if ( gFormatTransferTbl[i] == iCode )
		{
			return i;
		}
	}

	return -1;
}

uint32_t CMvImageConvertHelper::calcRateCtrlNum()
{
	uint32_t iControlNum = 1;
	uint32_t iFrameRate = 0;

	GenICam::IAcquisitionControlPtr sptrAcquisitionCtrl = GenICam::CSystem::getInstance().createAcquisitionControl(m_sptrCamera);
	if ( NULL != sptrAcquisitionCtrl )
	{
		double fltFrameRate = 0.0;
		sptrAcquisitionCtrl->acquisitionFrameRate().getValue(fltFrameRate);
		iFrameRate = (uint32_t)fltFrameRate;
	}

	if ( iFrameRate > DEFAULT_SHOW_RATE )
	{
		if ( iFrameRate < 40 )
		{
			iControlNum = 6;
		}
		else if ( iFrameRate < 50 )
		{
			iControlNum = 4;
		}
		else if ( iFrameRate < 60 )
		{
			iControlNum = 3;
		}
		else
		{
			iControlNum = 2;
		}
	}

	return iControlNum;
}

void CMvImageConvertHelper::addConvertedImage(const CMvImageBufPtr& sptrConvertedImage)
{
	static bool bThrowCtrl = true;

	Infra::CGuard guard(m_mutexQue);
	
	sptrConvertedImage->setImagePixelFormat(sptrConvertedImage->imagePixelFormat());
	sptrConvertedImage->setImageWidth(sptrConvertedImage->imageWidth());
	sptrConvertedImage->setImageHeight(sptrConvertedImage->imageHeight());

	if ( m_listImages.size() > DEFAULT_SHOW_RATE )
	{
		if ( bThrowCtrl )
		{
			m_listImages.pop_front();
			bThrowCtrl = false;
		}
		else
		{
			bThrowCtrl = true;
			return;
		}
	}

	m_listImages.push_back(sptrConvertedImage);
	m_semQue.post();
}

} // namespace Dahua
