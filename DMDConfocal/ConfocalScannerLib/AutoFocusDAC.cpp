
#include "StdAfx.h"
#include "AutoFocusDAC.h"
#include "HGloableFunction.h"

CAutoFocusDAC::CAutoFocusDAC(void)
{
	m_ConfocalCore=0;
	m_AxisZ = 0;
	m_AxisPIZ = 0;
	m_UseAxisZ = 0;

	m_CaptureVideo0.Vbuffer=0;
	m_CaptureVideo0.Vsize=0;

	m_CaptureVideo1.Vbuffer = 0;
	m_CaptureVideo1.Vsize = 0;

	m_hCaptureEvent0 = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hCaptureEvent1 = CreateEvent(NULL, TRUE, FALSE, NULL);

	pDACProcess = 0;
}

CAutoFocusDAC::~CAutoFocusDAC(void)
{
}

bool CAutoFocusDAC::InitPlugin(LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	if (m_ConfocalCore)
	{
		/***初始化相机***/
		m_CurVideoDevice0 = m_ConfocalCore->GetCurVideo();
		m_CurVideoDevice1 = m_ConfocalCore->GetCurVideoEx();
		/****初始化载物台***/
		if (m_ConfocalCore->GetGearBox())
			m_AxisZ = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Z);
		if (m_AxisZ)
			m_AxisZ->GetSubject()->Attach(this);
		if (m_ConfocalCore->GetGearBox())//PIZ轴是开环控制，没有反馈，只能靠延时
			m_AxisPIZ = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_PIZ);
	}
	if(m_ConfocalCore)
	{
		pDACProcess =m_ConfocalCore->GetCoreProcess(PROCESS_DACAF);
	}
	return true;
}

bool CAutoFocusDAC::UnInitPlugin()
{
	//bFocusOver=false;
	if (m_CaptureVideo0.Vbuffer)
		delete(m_CaptureVideo0.Vbuffer);
	if(m_CaptureVideo1.Vbuffer)
		delete(m_CaptureVideo1.Vbuffer);
	return true;
}

bool CAutoFocusDAC::Stop()
{
	//bFocusOver=false;
	return true;
}

int CAutoFocusDAC::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if (pSubject==m_AxisZ->GetSubject())
	{
		if(ID==NOTIFY_AXIS_MOVED)//||NOTIFY_AXIS_POSITION)//
		{
			//SetEvent(m_hMoveEvent);
		}
	}
	return 1;
}

bool CAutoFocusDAC::Set(void* WParas)
{
	if (WParas == 0)
		return false;
	_dmp = *(DiffMeasureParam*)WParas;
	return false;
}

bool CAutoFocusDAC::Start(void* vParas)
{
	bool bTest = false;

	ResetEvent(m_hCaptureEvent0);
	ResetEvent(m_hCaptureEvent1);

	if (bTest)
		m_ScanThread = CreateThread(NULL, 0, AutoFocusDACTestProc, this, 0, 0);
	else
		m_ScanThread = CreateThread(NULL, 0, AutoFocusDACProc, this, 0, 0);

	HCorePanel* mDlg = m_ConfocalCore->GetMsgPanel(MSG_TIME);
	mDlg->InitPanel(0);
	this->Attach(mDlg);
	m_ConfocalCore->GetDockablePanel(DOCKPANEL_STAGECTL)
		->GetCWnd()->PostMessage(0x118);
	((CDialogEx*)mDlg->GetCWnd())->DoModal();
	this->Detach(mDlg);

	return true;
}

DWORD WINAPI AutoFocusDACProc( LPVOID lp )
{
	((CAutoFocusDAC*)lp)->ScanAuto();
	return 0;
}

void CAutoFocusDAC::ScanAuto()
{
	if (_dmp.defocus_cap_mode == HConfocalCore::RenderChain_Normal)
	{
		m_CurVideoDevice0->SetTrigerMode(TRIGER_SOFT);
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Normal)->push_back(this);

		m_CurVideoDevice1->SetTrigerMode(TRIGER_SOFT);
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_NormalEx)->push_back(this);
	}
	else if (_dmp.defocus_cap_mode == HConfocalCore::RenderChain_Confocal)
	{
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->push_back(this);
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_ConfocalEx)->push_back(this);
	}
	else
	{
		return;
	}
	//抓图1、2
	bool bCapSuccess = Capture0(_dmp.defocus_cap_mode == HConfocalCore::RenderChain_Normal)
		&& Capture1(_dmp.defocus_cap_mode == HConfocalCore::RenderChain_Normal);

	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Normal)->remove(this);
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->remove(this);
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_NormalEx)->remove(this);
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_ConfocalEx)->remove(this);

	if (_dmp.defocus_cap_mode == HConfocalCore::RenderChain_Normal)
	{
		m_CurVideoDevice0->SetTrigerMode(TRIGER_INTERAL);
		m_CurVideoDevice1->SetTrigerMode(TRIGER_INTERAL);
	}

	if (!bCapSuccess || m_CaptureVideo0.Vbuffer == 0 || m_CaptureVideo1.Vbuffer == 0)
	{
		strMsg = L"Status:   Failed!!!";
		Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
		Notify(this, NOTIFY_MSGVIEW_END, 0, 0);
		return;
	}

	//传递到算法中进行计算，然后移动Z轴完成调焦
	DACAFPARAM dacParam;
	double dMoveZ;
	dacParam.pCaptureVideo0 = &m_CaptureVideo0;
	dacParam.pCaptureVideo1 = &m_CaptureVideo1;
	dacParam.coeff_b = _dmp.coeff_b;
	dacParam.coeff_k = _dmp.coeff_k;
	dacParam.bFilter = _dmp.bFilter;

	pDACProcess->ProcessImg(&dacParam, &dMoveZ, NULL);
	strMsg.Format(L"Info:  AxisZ move %.2f", dMoveZ);
	Notify(this, NOTIFY_MSGVIEW_Info, 0, &strMsg);

	m_UseAxisZ = m_AxisZ;
	if (_dmp.z_ctrl_mode == 1)
		m_UseAxisZ = m_AxisPIZ;
	if (m_UseAxisZ)
		MoveZ(1, dMoveZ);

	strMsg = L"Status:   DAC-AF finished!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
	Notify(this, NOTIFY_MSGVIEW_END, 0, &strMsg);
}

DWORD WINAPI AutoFocusDACTestProc(LPVOID lp)
{
	((CAutoFocusDAC*)lp)->ScanAutoTest();
	return 0;
}

void CAutoFocusDAC::ScanAutoTest()
{
	//抓图1、2
	int width = 0;
	int height = 0;
	int wbit = 0;
	LPBYTE mLoadBuf = 0;
	strMsg = L"Status:   Load Img0!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
	HLoadBmp(&mLoadBuf, width, height, wbit, HGetStartPath() + L"//DiffMeasureResult//defocus_a.bmp");
	if (!SetImg(width, height, wbit, mLoadBuf,m_CaptureVideo0))
	{
		strMsg = L"Status:   Failed!!!";
		Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
		Notify(this, NOTIFY_MSGVIEW_END, 0, 0);
		return;
	}
	
	strMsg = L"Status:   Load Img1!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
	HLoadBmp(&mLoadBuf, width, height, wbit, HGetStartPath() + L"//DiffMeasureResult//defocus_b.bmp");
	if (!SetImg(width, height, wbit, mLoadBuf, m_CaptureVideo1))
	{
		strMsg = L"Status:   Failed!!!";
		Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
		Notify(this, NOTIFY_MSGVIEW_END, 0, 0);
		return;
	}

	//传递到算法中进行计算，然后移动Z轴完成调焦
	DACAFPARAM dacParam;
	double dMoveZ;
	dacParam.pCaptureVideo0 = &m_CaptureVideo0;
	dacParam.pCaptureVideo1 = &m_CaptureVideo1;
	dacParam.coeff_b = _dmp.coeff_b;
	dacParam.coeff_k = _dmp.coeff_k;
	dacParam.bFilter = _dmp.bFilter;

	pDACProcess->ProcessImg(&dacParam, &dMoveZ, NULL);
	strMsg.Format(L"Info:  AxisZ move %.2f", dMoveZ);
	Notify(this, NOTIFY_MSGVIEW_Info, 0, &strMsg);

	m_UseAxisZ = m_AxisZ;
	if (_dmp.z_ctrl_mode == 1)
		m_UseAxisZ = m_AxisPIZ;
	if (m_UseAxisZ)
		MoveZ(1, dMoveZ);

	strMsg = L"Status:   DAC-AF finished!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
	Notify(this, NOTIFY_MSGVIEW_END, 0, &strMsg);
}

bool CAutoFocusDAC::Capture0(bool bTrigger)
{
	if (bTrigger)
		m_CurVideoDevice0->TrigerVideoData();

	strMsg = L"Status:   Capturing0!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);

	int iCount = 0;
	while (WaitForSingleObject(m_hCaptureEvent0, 1300) == WAIT_TIMEOUT)
	{
		iCount++;
		if (iCount <= 6)
		{
			if (bTrigger)
				m_CurVideoDevice0->TrigerVideoData();
		}
		else
		{
			strMsg = L"Status:   Capturing0 Time Out!";
			Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
			return false;
		}
	}
	ResetEvent(m_hCaptureEvent0);
	strMsg = L"Status:   Processing0!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
	return true;
}

bool CAutoFocusDAC::Capture1(bool bTrigger)
{
	if (bTrigger)
		m_CurVideoDevice1->TrigerVideoData();

	strMsg = L"Status:   Capturing1!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);

	int iCount = 0;
	while (WaitForSingleObject(m_hCaptureEvent1, 1300) == WAIT_TIMEOUT)
	{
		iCount++;
		if (iCount <= 6)
		{
			if (bTrigger)
				m_CurVideoDevice1->TrigerVideoData();
		}
		else
		{
			strMsg = L"Status:   Capturing1 Time Out!";
			Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
			return false;
		}
	}
	ResetEvent(m_hCaptureEvent1);
	strMsg = L"Status:   Processing1!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
	return true;
}

void CAutoFocusDAC::Renderer(HVideoHeader* pHeader, LPBYTE pBuffer)
{
	if (pHeader->Vuser == 0)
		Func_FlushImg(pHeader, pBuffer, m_CaptureVideo0, m_hCaptureEvent0);
	else
		Func_FlushImg(pHeader, pBuffer, m_CaptureVideo1, m_hCaptureEvent1);
}

void CAutoFocusDAC::Func_FlushImg(HVideoHeader* pSrcHeader, LPBYTE pBuffer
	, HVideoHeader &DstHeader, HANDLE hCaptureEvent)
{
	if (WaitForSingleObject(hCaptureEvent, 1) != WAIT_TIMEOUT)
		return;//旧的图还没有被取走，就不去更新！

	if (DstHeader.Vsize != pSrcHeader->Vsize)
	{
		if (DstHeader.Vbuffer)
			delete(DstHeader.Vbuffer);
		DstHeader.Vbuffer = (LPBYTE)malloc(pSrcHeader->Vsize);
		DstHeader.Vwidth = pSrcHeader->Vwidth;
		DstHeader.Vheight = pSrcHeader->Vheight;
		DstHeader.Vsize = pSrcHeader->Vsize;
		DstHeader.VwBit = pSrcHeader->VwBit;
	}
	if (!pBuffer)
		return;
	memcpy(DstHeader.Vbuffer, pBuffer, DstHeader.Vsize);
	SetEvent(hCaptureEvent);
}

bool CAutoFocusDAC::SetImg(int width, int height, int wbit, LPBYTE pBuffer, HVideoHeader &DstHeader)
{
	if (DstHeader.Vbuffer)
		delete(DstHeader.Vbuffer);
	DstHeader.Vwidth = width;
	DstHeader.Vheight = height;
	DstHeader.Vsize = width * height*wbit / 8;
	DstHeader.Vbuffer = (LPBYTE)malloc(DstHeader.Vsize);
	DstHeader.VwBit = wbit;
	if (!DstHeader.Vbuffer)
		return false;
	memcpy(DstHeader.Vbuffer, pBuffer, DstHeader.Vsize);
}

bool CAutoFocusDAC::MoveZ(int iDirection, float fValue)
{
	if (m_UseAxisZ->GetPosition() + iDirection * fValue <= 0)
		return true;
	strMsg.Format(L"Info: Moving %0.2f", fValue*iDirection);
	Notify(this, NOTIFY_MSGVIEW_Info, 0, &strMsg);
	strMsg = L"Status:  Z Moving";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
	m_UseAxisZ->MoveRef(iDirection*fValue);
	return true;
}

bool CAutoFocusDAC::MoveToZ(float fValue)
{
	if (m_UseAxisZ->GetPosition() == fValue)
		return true;
	strMsg.Format(L"Info: Moving to %0.2f", fValue);
	Notify(this, NOTIFY_MSGVIEW_Info, 0, &strMsg);
	strMsg = L"Status:  Z Moving";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
	m_UseAxisZ->MoveTo(fValue);
	return true;
}

