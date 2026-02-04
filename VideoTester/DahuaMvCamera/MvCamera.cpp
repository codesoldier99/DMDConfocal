#include "stdafx.h"

#include "MvCamera.h"

namespace Dahua{

CMvCamera::CMvCamera(const GenICam::ICameraPtr& sptrCamera, int idx, CString strName)
	: m_sptrCamera(sptrCamera)
	, m_pMvCameraSink(NULL)
	, m_triggerSoftware(sptrCamera, "TriggerSoftware")
{
	m_CameraParas.m_CameraName  = strName;
	m_CameraParas.iSel			= idx;
	m_CameraParas.m_pBuffer		= 0;
	m_CameraParas.m_pDcamBuffer = 0;
	m_CameraParas.m_pWidth		= 0;
	m_CameraParas.m_pHeight		= 0;
	m_CameraParas.m_fExposure	= 10.0;
	m_CameraParas.m_Gain		= 1;
	m_VideoRender = 0;

	m_VideoFormate.VcaptureTime = 0;
	m_bTriggerSoft = false;
}

CMvCamera::~CMvCamera()
{
	m_VideoRender = 0;
	this->CloseCamera();
}

bool CMvCamera::open(IMvCameraSink* pSink)
{
	if ( NULL == m_sptrCamera )
	{
		return false;
	}

	bool bConnRet = m_sptrCamera->connect();
	if ( !bConnRet )
	{
		return false;
	}

	GenICam::IEventSubscribePtr sptrEventSubscribe = GenICam::CSystem::getInstance().createEventSubscribe(m_sptrCamera);
	if ( NULL != sptrEventSubscribe &&
		NULL != pSink)
	{
		sptrEventSubscribe->subscribeConnectArgs(GenICam::ConnectArgProc(&CMvCamera::procConnectArg, this));
		m_pMvCameraSink = pSink;
	}

	return true;
}

void CMvCamera::close()
{
	if ( NULL != m_sptrCamera )
	{
		m_sptrGigECamera.reset();
		m_sptrGigEInterface.reset();
		m_sptrParamCtrlSet.reset();
		m_sptrDisplayHelper.reset();
		m_pMvCameraSink = NULL;

		m_sptrCamera->disConnect();
	}
}

std::string CMvCamera::getDevUserID()
{
	if ( NULL == m_sptrCamera )
	{
		return "Too lazy to set a ID";
	}

	return m_sptrCamera->getName();
}

bool CMvCamera::forceIpAddress( const char* pIpAddr, const char* pMaskAddr, const char* pGatewayAddr )
{
	if ( NULL == m_sptrGigECamera )
	{
		m_sptrGigECamera = GenICam::IGigECamera::getInstance(m_sptrCamera);
	}

	return m_sptrGigECamera->forceIpAddress(pIpAddr, pMaskAddr, pGatewayAddr);
}

bool CMvCamera::startDisplay(void* pShowHandle, int iShowRate)
{
	if ( !m_sptrCamera->isConnected() )
	{
		return false;
	}

	if ( NULL == m_sptrDisplayHelper )
	{
		m_sptrDisplayHelper = CMvDisplayHelperPtr(new CMvDisplayHelper(m_sptrCamera, this));
	}

	if ( !m_sptrDisplayHelper->isValid() )
	{
		return false;
	}

	return m_sptrDisplayHelper->startDisplay(pShowHandle, iShowRate);
}

bool CMvCamera::stopDisplay()
{
	if ( NULL == m_sptrDisplayHelper )
	{
		return false;
	}

	return m_sptrDisplayHelper->stopDisplay();
}

bool CMvCamera::subscribeParamUpdate(const ParamVectorType* pParamList)
{
	// TODO
	return false;
}

CMvCamera::ParamControlSetPtr CMvCamera::getParamControl()
{
	if ( NULL == m_sptrParamCtrlSet &&
		m_sptrCamera->isConnected() )
	{
		m_sptrParamCtrlSet = ParamControlSetPtr(new ParamControlSet());
		m_sptrParamCtrlSet->_sptrAcquistionCtrl = GenICam::CSystem::getInstance().createAcquisitionControl(m_sptrCamera);
		m_sptrParamCtrlSet->_sptrAnalogCtrl = GenICam::CSystem::getInstance().createAnalogControl(m_sptrCamera);
		m_sptrParamCtrlSet->_sptrImageFormatCtrl = GenICam::CSystem::getInstance().createImageFormatControl(m_sptrCamera);
		m_sptrParamCtrlSet->_sptrIspCtrl = GenICam::CSystem::getInstance().createISPControl(m_sptrCamera);
		m_sptrParamCtrlSet->_sptrUserSetCtrl = GenICam::CSystem::getInstance().createUserSetControl(m_sptrCamera);
	}

	return m_sptrParamCtrlSet;
}
// 获取Gige 相机信息
// get gige camera information
CMvCamera::NetAddressInfoPtr CMvCamera::getGigECameraInfo()
{
	if ( NULL == m_sptrGigECamera )
	{
		m_sptrGigECamera = GenICam::IGigECamera::getInstance(m_sptrCamera);
	}

	if ( NULL == m_sptrGigECamera )
	{
		return CMvCamera::NetAddressInfoPtr();
	}

	CMvCamera::NetAddressInfoPtr sptrCameraIpInfo(new CMvCamera::NetAddressInfo());
	sptrCameraIpInfo->_strGatewayAddr = m_sptrGigECamera->getGateway().c_str();
	sptrCameraIpInfo->_strMaskAddr = m_sptrGigECamera->getSubnetMask().c_str();
	sptrCameraIpInfo->_strIpAddr = m_sptrGigECamera->getIpAddress().c_str();
	sptrCameraIpInfo->_strMac = m_sptrGigECamera->getMacAddress().c_str();

	return sptrCameraIpInfo;
}

// 获取Gige 接口信息
// get gige camera interface information
CMvCamera::NetAddressInfoPtr CMvCamera::getGigEInterfaceInfo()
{
	if ( NULL == m_sptrGigEInterface )
	{
		m_sptrGigEInterface = GenICam::IGigEInterface::getInstance(m_sptrCamera);
	}

	if ( NULL == m_sptrGigEInterface )
	{
		return CMvCamera::NetAddressInfoPtr();
	}

	CMvCamera::NetAddressInfoPtr sptrInterfaceInfo(new CMvCamera::NetAddressInfo());
	sptrInterfaceInfo->_strGatewayAddr = m_sptrGigEInterface->getGateway().c_str();
	sptrInterfaceInfo->_strMaskAddr = m_sptrGigEInterface->getSubnetMask().c_str();
	sptrInterfaceInfo->_strIpAddr = m_sptrGigEInterface->getIpAddress().c_str();
	sptrInterfaceInfo->_strMac = m_sptrGigEInterface->getMacAddress().c_str();

	return sptrInterfaceInfo;
}

void CMvCamera::enableRateCtrl(bool bEnable)
{
	if ( NULL != m_sptrDisplayHelper )
	{
		m_sptrDisplayHelper->enableRateCtrl(bEnable);
	}
}

void CMvCamera::procConnectArg(const GenICam::SConnectArg& arg)
{
	if ( GenICam::SConnectArg::offLine == arg.m_event &&
		NULL != m_pMvCameraSink)
	{
		m_pMvCameraSink->onCameraDisconnect();
	}
}

CString CMvCamera::GetBining(int pIndex)
{
	CString temp;
	temp.Format(_T("%d X %d"), m_CameraParas.m_pWidth[pIndex],m_CameraParas.m_pHeight[pIndex]);
	return temp;
}

HVideoHeader CMvCamera::GetImgHeader()
{
	HVideoHeader t_result;
	t_result.Vheight	= m_CameraParas.m_pHeight[m_CameraParas.m_iBinningIndx];
	t_result.Vwidth		= m_CameraParas.m_pWidth[m_CameraParas.m_iBinningIndx];
	t_result.VwBit		= m_CameraParas.m_iColorChannel*8;
	t_result.Vuser		= m_CameraParas.iSel;//标识相机
	t_result.Vsize		= t_result.Vwidth*t_result.Vheight*t_result.VwBit;
	return t_result;
}

bool CMvCamera::OpenCamera()
{
	if (NULL == m_sptrCamera)
	{
		return false;
	}
	if(m_sptrCamera->isConnected())
		return true;

	bool bConnRet = m_sptrCamera->connect();
	if (!bConnRet)
	{
		return false;
	}

	m_CameraParas.m_iBinningCount = 1;
	m_CameraParas.m_pWidth = new int[1];
	m_CameraParas.m_pHeight = new int[1];
	m_CameraParas.m_pWidth[0] = 1920;
	m_CameraParas.m_pHeight[0] = 1200;
	m_CameraParas.m_iBitDepth = 8;
	m_CameraParas.m_iColorChannel = 1;

	this->SetTrigerMode(TRIGER_INTERAL);

	Notify(this, Notify_CarlVideo_OpenCamera, 0, &m_CameraParas.m_CameraName);
	return true;
}

bool CMvCamera::Run()
{
	if (!m_sptrCamera->isConnected())
	{
		return false;
	}

	if (NULL == m_sptrDisplayHelper)
	{
		m_sptrDisplayHelper = CMvDisplayHelperPtr(new CMvDisplayHelper(m_sptrCamera,this));
	}

	if (!m_sptrDisplayHelper->isValid())
	{
		return false;
	}

	if (m_sptrDisplayHelper->startDisplay(NULL, 30))
	{
		Notify(this, Notify_CarlVideo_RunCamera, 0, &m_CameraParas.m_CameraName);
		return true;
	}
	return false;
}

bool CMvCamera::Pause()
{
	if (NULL == m_sptrDisplayHelper)
		return false;

	if (!m_sptrCamera->isConnected())
		return true;

	if(m_sptrDisplayHelper->stopDisplay())
	{
		Notify(this, Notify_CarlVideo_PauseCamera, 0, &m_CameraParas.m_CameraName);
		return true;
	}
	return false;
}

bool CMvCamera::CloseCamera()
{
	if (NULL != m_sptrCamera)
	{
		m_sptrDisplayHelper->stopDisplay();
		m_sptrGigECamera.reset();
		m_sptrGigEInterface.reset();
		m_sptrParamCtrlSet.reset();
		m_sptrDisplayHelper.reset();
		m_pMvCameraSink = NULL;
		m_sptrCamera->disConnect();
	}
	//bRun = false;
	Notify(this, Notify_CarlVideo_CloseCamera, 0, &m_CameraParas.m_CameraName);
	return true;
}

bool CMvCamera::GetEnable(HVideoPID pPID)
{
	switch (pPID)
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

void CMvCamera::SetValue(HVideoPID pPID, float pValue)
{
	switch (pPID)
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
			//dcam_settriggermode(g_MuCameraParas.m_iCurHandle, pValue);
			break;
	}
}

float CMvCamera::GetValue(HVideoPID pPID)
{
	switch (pPID)
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
	return 0;
}

void CMvCamera::SetExposure(float pExpos)
{
	m_CameraParas.m_fExposure = pExpos; //ms为单位
	Dahua::CMvCamera::ParamControlSetPtr sptrParamCtrlSet = this->getParamControl();
	if (NULL == sptrParamCtrlSet)
		return;
	// 获取ExposureTime节点对象// get ExposureTime node object
	Dahua::GenICam::CDoubleNode sptrDoubleNode = sptrParamCtrlSet->_sptrAcquistionCtrl->exposureTime();
	if (!sptrDoubleNode.isValid())
		return;
	if (sptrDoubleNode.isWriteable())
		sptrDoubleNode.setValue(pExpos*1000);
	Notify(this, Notify_CarlVideo_ExpTime, 0, 0, pExpos * 1000);
}

void CMvCamera::AutoAWB(bool pAuto)
{
	//m_AWB = pAuto;
}

bool CMvCamera::SetFlip(bool pFlip)
{
	if (NULL == m_sptrCamera)
		return false;
	GenICam::CBoolNode nodeBool(m_sptrCamera, "ReverseY");
	if (false == nodeBool.isValid())
	{
		printf("get ReverseX node fail.\n");
		return false;
	}
	bool bReverse = false;
	if (false == nodeBool.getValue(bReverse))
	{
		printf("get ReverseX fail.\n");
		return false;
	}
	else
	{
		printf("before change ,ReverseX is %d\n", bReverse);
	}
	bReverse = pFlip;
	if (false == nodeBool.setValue(!bReverse))
	{
		printf("set ReverseX fail.\n");
		return false;
	}
	Notify(this, Notify_CarlVideo_Flip, 0, &pFlip);
	return true;
}

bool CMvCamera::SetMirror(bool pMirror)
{
	if (NULL == m_sptrCamera)
		return false;
	GenICam::CBoolNode nodeBool(m_sptrCamera, "ReverseX");
	if (false == nodeBool.isValid())
	{
		printf("get ReverseX node fail.\n");
		return false;
	}
	bool bReverse = false;
	if (false == nodeBool.getValue(bReverse))
	{
		printf("get ReverseX fail.\n");
		return false;
	}
	else
	{
		printf("before change ,ReverseX is %d\n", bReverse);
	}
	bReverse = pMirror;
	if (false == nodeBool.setValue(!bReverse))
	{
		printf("set ReverseX fail.\n");
		return false;
	}
	Notify(this, Notify_CarlVideo_Mirror, 0, &pMirror);
	return true;
}

bool CMvCamera::GrabImage(HVideoHeader* pHeader, LPBYTE pBuffer)
{ 
	//从m_CameraParas的缓存里面抓图
	pHeader->Vheight	= m_CameraParas.m_pHeight[m_CameraParas.m_iBinningIndx];
	pHeader->Vwidth		= m_CameraParas.m_pWidth[m_CameraParas.m_iBinningIndx];
	pHeader->VwBit		= m_CameraParas.m_iColorChannel * 8;   //16;
	pHeader->Vuser		= m_CameraParas.iSel;                  //标识相机
	pHeader->Vsize		= pHeader->Vwidth * pHeader->Vheight*pHeader->VwBit / 8;
	//触发采集一次	
	if (m_CameraParas.m_pBuffer)
	{	
		memcpy(pBuffer, m_CameraParas.m_pBuffer, pHeader->Vsize);
		return true;
	}
	else
	{
		return false;
	}

	return false; 
}

void CMvCamera::SetTrigerMode(TrigerType pMode) 
{ 
	Dahua::CMvCamera::ParamControlSetPtr sptrParamCtrlSet = this->getParamControl();
	Dahua::GenICam::CEnumNode triggerSource = sptrParamCtrlSet->_sptrAcquistionCtrl->triggerSource();
	Dahua::GenICam::CEnumNode selector = sptrParamCtrlSet->_sptrAcquistionCtrl->triggerSelector();
	Dahua::GenICam::CEnumNode mode = sptrParamCtrlSet->_sptrAcquistionCtrl->triggerMode();
	bool bRet = false;
	m_bTriggerSoft = false;
	switch (pMode)
	{
		case TRIGER_INTERAL:
			// 设置触发源为软触发    // set trigger source to software trigger    
			bRet = triggerSource.setValueBySymbol("Software");
			if (bRet != true)
			{
				printf("set trigger source fail.\n");
				return;
			}
			// 设置触发器    // set trigger    
			bRet = selector.setValueBySymbol("FrameStart");
			if (bRet != true)
			{
				printf("set trigger selector fail.\n");
				return;
			}
			// 设置触发模式    // set trigger mode    
			bRet = mode.setValueBySymbol("Off");
			if (bRet != true)
			{
				printf("set trigger mode fail.\n");
				return;
			}
			break;
		case TRIGER_SOFT:
			m_sptrDisplayHelper->ClearBuf();//清空缓存
			// 设置触发源为软触发    // set trigger source to software trigger    
			bRet = triggerSource.setValueBySymbol("Software");
			if (bRet != true)
			{
				printf("set trigger source fail.\n");
				return;
			}
			m_triggerSoftware = sptrParamCtrlSet->_sptrAcquistionCtrl->triggerSoftware();
			m_bTriggerSoft = true;
			// 设置触发器    // set trigger    
			bRet = selector.setValueBySymbol("FrameStart");
			if (bRet != true)
			{
				printf("set trigger selector fail.\n");
				return;
			}
			// 设置触发模式    // set trigger mode    
			bRet = mode.setValueBySymbol("On");
			if (bRet != true)
			{
				printf("set trigger mode fail.\n");
				return;
			}
			break;
		case TRIGER_OUT:
			m_sptrDisplayHelper->ClearBuf();//清空缓存
			// 设置触发源为Line1触发    // set trigger source to Line1 trigger  
			bRet = triggerSource.setValueBySymbol("Line1");
			if (bRet != true)
			{
				printf("set trigger source fail.\n");
				return;
			}    
			// 设置触发器    // set trigger    
			bRet = selector.setValueBySymbol("FrameStart");
			if (bRet != true)
			{
				printf("set trigger selector fail.\n");
				return;
			}
			// 设置触发模式    // set trigger mode    
			bRet = mode.setValueBySymbol("On");
			if (bRet != true)
			{
				printf("set trigger mode fail.\n");
				return;
			}
			// 设置外触发为上升沿（下降沿为FallingEdge）    // set external trigger as rising edge (falling edge as falling edge)    
			Dahua::GenICam::CEnumNode triggerAct = sptrParamCtrlSet->_sptrAcquistionCtrl->triggerActivation();
			bRet = triggerAct.setValueBySymbol("RisingEdge");
			if (bRet != true)
			{
				printf("set trigger activation fail.\n");
				return;
			}
			break;
	}
	Notify(this, Notify_CarlVideo_Triger, 0, &pMode);
	return; 
}

void CMvCamera::TrigerVideoData()
{ 
	// 执行一次软触发    
	if (m_bTriggerSoft)
	{
		if (!m_triggerSoftware.execute())//会自动触发onCameraRender函数，更新到渲染链中！
		{
			AfxMessageBox(_T("TriggerSoftware failed!"));
		}
	}
	return;
}

void CMvCamera::SetRender(HVideoRender* pRender) 
{ 
	m_VideoRender = pRender;
	return; 
}

void CMvCamera::onCameraRender(uint8_t* pData, int iWidth, int iHeight, uint64_t iPixelFormat)
{
	int iBitDepth = 8, iColorChannel = 0;
	if (iPixelFormat == Dahua::GenICam::gvspPixelMono8)
	{
		iColorChannel = 1;
	}
	else
	{
		iColorChannel = 3;
	}
	if (m_CameraParas.m_pBuffer)
	{
		if (m_CameraParas.m_pWidth[0] != iWidth || m_CameraParas.m_pHeight[0] != iHeight
			|| m_CameraParas.m_iColorChannel != iColorChannel)
		{
			delete(m_CameraParas.m_pBuffer);
			m_CameraParas.m_pBuffer = (LPBYTE)malloc(iWidth*iHeight*iColorChannel);
		}
	}
	else
	{
		m_CameraParas.m_pBuffer = (LPBYTE)malloc(iWidth*iHeight*iColorChannel);
	}
	m_CameraParas.m_pWidth[0] = iWidth;
	m_CameraParas.m_pHeight[0] = iHeight;
	m_CameraParas.m_iColorChannel = iColorChannel;
	memcpy(m_CameraParas.m_pBuffer, pData, iWidth*iHeight*iColorChannel);

	m_VideoFormate.Vheight = iHeight;
	m_VideoFormate.Vwidth = iWidth;
	long t_Size = iHeight * iWidth;
	m_VideoFormate.VwBit = m_CameraParas.m_iColorChannel * 8;
	m_VideoFormate.Vsize = t_Size;
	m_VideoFormate.Vuser = m_CameraParas.iSel;//标识相机

	DWORD t_now = ::GetTickCount();
	DWORD dutime = t_now - (DWORD)m_VideoFormate.VcaptureTime;
	m_VideoFormate.VcaptureTime = t_now;
	m_VideoFormate.VFps = 1000.0 / dutime;

	if (m_VideoRender)
		m_VideoRender->Renderer(&m_VideoFormate, m_CameraParas.m_pBuffer);
}

} // namespace Dahua
