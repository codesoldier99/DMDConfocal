#ifndef __DAHUA_MULTICAMERAS_DEMO_MVCAMERA_H__
#define __DAHUA_MULTICAMERAS_DEMO_MVCAMERA_H__

#include "GenICam/Camera.h"
#include "GenICam/System.h"
#include "GenICam/GigE/GigECamera.h"
#include "GenICam/GigE/GigEInterface.h"
#include "GenICam/EventSubscribe.h"
#include "Memory/SharedPtr.h"
#include "MvDisplayHelper.h"
#include "CarlVideo.h"

#include <string>
#include <vector>

struct DuhuaMvCameraPara
{
	int iSel;
	CString m_CameraName;
	int m_iBitDepth;
	int m_iColorChannel;
	int m_iBinningCount;
	int m_iBinningIndx;
	//HDCAM m_iCurHandle;
	float m_fExposure;
	float m_fMinExposure;
	float m_fMaxExposure;
	int m_Gain;
	BYTE* m_pBuffer;
	unsigned short* m_pDcamBuffer;//底层数据传送上来是18位的
	int * m_pWidth;
	int * m_pHeight;
	long m_DcamSize;
	long m_NowFrameIndex;
	long m_TotalFrame;
};

namespace Dahua{

class IMvCameraSink
{
public:
	virtual ~IMvCameraSink() {}

public:
	virtual void onCameraDisconnect() = 0;
};

class CMvCamera :public HVideoDevice, public IMvCameraRender
{
public:
	typedef struct _tag_NetAddressInfo_
	{
		std::string		_strIpAddr;
		std::string		_strMaskAddr;
		std::string		_strGatewayAddr;
		std::string		_strMac;

	}NetAddressInfo;

	typedef struct _tag_ParamControl_
	{
		GenICam::IAcquisitionControlPtr		_sptrAcquistionCtrl;
		GenICam::IAnalogControlPtr			_sptrAnalogCtrl;
		GenICam::IImageFormatControlPtr		_sptrImageFormatCtrl;
		GenICam::IISPControlPtr				_sptrIspCtrl;
		GenICam::IUserSetControlPtr			_sptrUserSetCtrl;
	}ParamControlSet;

	typedef Memory::TSharedPtr<ParamControlSet> ParamControlSetPtr;
	typedef std::vector<std::string> ParamVectorType;
	typedef Memory::TSharedPtr<NetAddressInfo> NetAddressInfoPtr;

public:
	CMvCamera(const GenICam::ICameraPtr& sptrCamera, int idx, CString strName);
	~CMvCamera();

public:
	bool open(IMvCameraSink* pSink = NULL);

	void close();

	std::string getDevUserID();

	bool forceIpAddress(const char* pIpAddr, const char* pMaskAddr = NULL, const char* pGatewayAddr = NULL);

	bool startDisplay(void* pShowHandle, int iShowRate = 30);

	bool stopDisplay();

	bool subscribeParamUpdate(const ParamVectorType* pParamList = NULL);

	ParamControlSetPtr getParamControl();

	NetAddressInfoPtr getGigECameraInfo();

	NetAddressInfoPtr getGigEInterfaceInfo();

	void enableRateCtrl(bool bEnable);

private:
	void procConnectArg(const GenICam::SConnectArg& arg);

private:
	GenICam::ICameraPtr				m_sptrCamera;
	GenICam::IGigECameraPtr			m_sptrGigECamera;
	GenICam::IGigEInterfacePtr		m_sptrGigEInterface;
	ParamControlSetPtr				m_sptrParamCtrlSet;
	CMvDisplayHelperPtr				m_sptrDisplayHelper;
	IMvCameraSink*					m_pMvCameraSink;
	GenICam::CCmdNode				m_triggerSoftware; //软触发
	bool							m_bTriggerSoft;
	int								m_nInterfaceType;

public:
	CString GetCameraName() { return m_CameraParas.m_CameraName; };

	int GetBiningCount() { return m_CameraParas.m_iBinningCount; };
	CString GetBining(int pIndex) ;
	void SetBinning(int pIndex, CPoint Pt = CPoint(0, 0)) { return ;};
	void SetBinningPt(int pIndex, CPoint Pt) { return; };
	int GetBiningIndex() { return m_CameraParas.m_iBinningIndx; };

	bool OpenCamera();
	bool Run();
	bool Pause();
	bool CloseCamera();

	bool GetEnable(HVideoPID pPID) ;
	void SetValue(HVideoPID pPID, float pValue);
	float GetValue(HVideoPID pPID) ;
	void SetExposure(float pExpos) ;
	float GetExposure() { return m_CameraParas.m_fExposure; };
	void AutoAWB(bool pAuto);
	bool SetFlip(bool pFlip);
	bool SetMirror(bool pMirror) ;

	HVideoHeader GetImgHeader();
	bool GrabImage(HVideoHeader* pHeader, LPBYTE pBuffer);
	void SetTrigerMode(TrigerType pMode);
	void TrigerVideoData();
	void SetRender(HVideoRender* pRender);

	void onCameraRender(uint8_t* pData, int iWidth, int iHeight, uint64_t iPixelFormat);

private:
	DuhuaMvCameraPara m_CameraParas;
	HVideoRender* m_VideoRender;
	HVideoHeader m_VideoFormate;
};

typedef Memory::TSharedPtr<CMvCamera> CMvCameraPtr;

} // namespace Dahua

#endif // __DAHUA_MULTICAMERAS_DEMO_MVCAMERA_H__
