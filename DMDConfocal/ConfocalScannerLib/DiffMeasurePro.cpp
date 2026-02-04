#include "StdAfx.h"
#include "DiffMeasurePro.h"
#include "HGloableFunction.h"

DiffMeasurePro::DiffMeasurePro(void)
{
	m_ConfocalCore=0;

	m_CaptureVideo0.Vsize = 0;//表示没有东西
	m_CaptureVideo0.Vbuffer = 0;
	m_CaptureVideo1.Vsize = 0;//表示没有东西
	m_CaptureVideo1.Vbuffer = 0;
	m_hCaptureEvent0 = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hCaptureEvent1 = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_ScanEvent0 = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_ScanEvent1 = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_ScanThread=0;
	strMsg	=	L"";
	_img_w	= _img_h = 0;

	m_ScanThread0 = 0;
	m_ScanThread1 = 0;
	m_ProcessThread = 0;
}

DiffMeasurePro::~DiffMeasurePro(void)
{

}

bool DiffMeasurePro::InitPlugin(LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	//初始化载物台
	if(m_ConfocalCore)
	{
		m_CurVideoDevice0 = m_ConfocalCore->GetCurVideo();
		m_CurVideoDevice1 = m_ConfocalCore->GetCurVideoEx();
	}
	return true;
}

bool DiffMeasurePro::UnInitPlugin()
{
	return true;
}

bool DiffMeasurePro::Stop()
{
	return true;
}

bool DiffMeasurePro::Set(void* WParas)
{
	if (WParas==0)
		return false;
	_dmp = *(DiffMeasureParam*)WParas;
	return true;
}

bool DiffMeasurePro::Start(void* vParas)//Start(bool bConfocal)
{
	bool bParallel = false;
	if (_dmp.bTest)
	{
		m_ScanThread = CreateThread(NULL, 0, DiffMeasureProTestProc, this, 0, 0);
		//return true;
	}
	else
	{
		ResetEvent(m_hCaptureEvent0);
		ResetEvent(m_hCaptureEvent1);
		if (!bParallel)
		{
			m_ScanThread = CreateThread(NULL, 0, DiffMeasureProProc, this, 0, 0);
		}
		else
		{
			ResetEvent(m_ScanEvent0);
			ResetEvent(m_ScanEvent1);
			m_ScanThread0 = CreateThread(NULL, 0, Cap0Proc, this, 0, 0);
			m_ScanThread1 = CreateThread(NULL, 0, Cap1Proc, this, 0, 0);
			m_ProcessThread = CreateThread(NULL, 0, AllProc, this, 0, 0);
		}
	}

	HCorePanel* mDlg=m_ConfocalCore->GetMsgPanel(MSG_TIME);
	mDlg->InitPanel(0);
	this->Attach(mDlg);
	m_ConfocalCore->GetDockablePanel(DOCKPANEL_STAGECTL)
		->GetCWnd()->PostMessage(0x118);
	((CDialogEx*)mDlg->GetCWnd())->DoModal();
	this->Detach(mDlg);

	if (!bParallel)
	{
		WaitForSingleObject(m_ScanThread, 2000);
	}
	else
	{
		WaitForSingleObject(m_ScanThread0, 2000);
		WaitForSingleObject(m_ScanThread1, 2000);
		WaitForSingleObject(m_ProcessThread, 2000);
	}
	return true;
}

void DiffMeasurePro::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
{
	if (pHeader->Vuser == 0)
		Func_FlushImg(pHeader, pBuffer, m_CaptureVideo0, m_hCaptureEvent0);
	else
		Func_FlushImg(pHeader, pBuffer, m_CaptureVideo1, m_hCaptureEvent1);
}

void DiffMeasurePro::Func_FlushImg(HVideoHeader* pSrcHeader, LPBYTE pBuffer
	, HVideoHeader &DstHeader, HANDLE hCaptureEvent)
{
	if (WaitForSingleObject(hCaptureEvent, 1) != WAIT_TIMEOUT)
	{
		return;//旧的图还没有被取走，就不去更新！
	}
	
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

int DiffMeasurePro::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	return 0;
}

DWORD WINAPI DiffMeasureProProc( LPVOID lp )
{
	((DiffMeasurePro*)lp)->ScanImage();
	return 0;
}

DWORD WINAPI Cap0Proc(LPVOID lp)
{
	((DiffMeasurePro*)lp)->ScanImage0();
	return 0;
}

DWORD WINAPI Cap1Proc(LPVOID lp)
{
	((DiffMeasurePro*)lp)->ScanImage1();
	return 0;
}

DWORD WINAPI AllProc(LPVOID lp)
{
	((DiffMeasurePro*)lp)->ProcessImage();
	return 0;
}

DWORD WINAPI DiffMeasureProTestProc(LPVOID lp)
{
	((DiffMeasurePro*)lp)->DoTest();
	return 0;
}

void DiffMeasurePro::DoTest()
{
	strMsg = L"Status:   Image processing!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);

	Notify(this, NOTIFY_DIFF_MEASURE, 0, 0, 0, &_dmp); //算法会阻塞

	strMsg = L"Status:   Test Process【Finished】!";
	Notify(this, NOTIFY_MSGVIEW_END, 0, &strMsg);
}

void DiffMeasurePro::ScanImage0()
{
	if (_dmp.defocus_cap_mode == HConfocalCore::RenderChain_Normal)
	{
		m_CurVideoDevice0->SetTrigerMode(TRIGER_SOFT);
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Normal)->push_back(this);
	}
	else if (_dmp.defocus_cap_mode == HConfocalCore::RenderChain_Confocal)
	{
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->push_back(this);
	}
	else
	{
		return;
	}

	if (Capture0(_dmp.defocus_cap_mode == HConfocalCore::RenderChain_Normal))
	{
		
		_img_w = m_CaptureVideo0.Vwidth;
		_img_h = m_CaptureVideo0.Vheight;
		_img_a.resize(_img_w * _img_h);
		memset(_img_a.data(), 125, _img_w * _img_h);//复制聚焦清楚的图
		_img_c.resize(_img_w * _img_h);
		memcpy(_img_c.data(), m_CaptureVideo0.Vbuffer, _img_w * _img_h);//复制焦后图
		SetEvent(m_ScanEvent0);
	}
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Normal)->remove(this);
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->remove(this);
}

void DiffMeasurePro::ScanImage1()
{
	if (_dmp.defocus_cap_mode == HConfocalCore::RenderChain_Normal)
	{
		m_CurVideoDevice1->SetTrigerMode(TRIGER_SOFT);
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_NormalEx)->push_back(this);
	}
	else if (_dmp.defocus_cap_mode == HConfocalCore::RenderChain_Confocal)
	{
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_ConfocalEx)->push_back(this);
	}
	else
	{
		return;
	}

	if (Capture1(_dmp.defocus_cap_mode == HConfocalCore::RenderChain_Normal))
	{
		_img_w = m_CaptureVideo1.Vwidth;
		_img_h = m_CaptureVideo1.Vheight;
		_img_b.resize(_img_w * _img_h);
		memcpy(_img_b.data(), m_CaptureVideo1.Vbuffer, _img_w * _img_h);//复制焦后图
		SetEvent(m_ScanEvent1);
	}
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_NormalEx)->remove(this);
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_ConfocalEx)->remove(this);
}

void DiffMeasurePro::ProcessImage()
{
	int iCount = 0;
	while (WaitForSingleObject(m_ScanEvent0, 3000) == WAIT_TIMEOUT)
	{
		iCount++;
		if (iCount <= 6)
		{

		}
		else
		{
			strMsg = L"Status:   ScanCap0 Time Out!";
			Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
			return;
		}
	}
	iCount = 0;
	while (WaitForSingleObject(m_ScanEvent1, 3000) == WAIT_TIMEOUT)
	{
		iCount++;
		if (iCount <= 6)
		{

		}
		else
		{
			strMsg = L"Status:   ScanCap1 Time Out!";
			Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
			return;
		}
	}

	_img_w = m_CaptureVideo0.Vwidth > m_CaptureVideo1.Vwidth ? m_CaptureVideo1.Vwidth : m_CaptureVideo0.Vwidth;
	_img_h = m_CaptureVideo0.Vheight > m_CaptureVideo1.Vheight ? m_CaptureVideo1.Vheight : m_CaptureVideo0.Vheight;
	DiffMeasureResult dmr;
	dmr.width	= _img_w;
	dmr.height	= _img_h;
	dmr.a		= _img_a.data();
	dmr.b		= _img_b.data();
	dmr.c		= _img_c.data();

	strMsg = L"Status:   Parallel Process【Finished】!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);

	Notify(this, NOTIFY_DIFF_MEASURE, 0, &dmr, 0, &_dmp);
	Notify(this, NOTIFY_MSGVIEW_END, 0, &strMsg);
}

void DiffMeasurePro::ScanImage()
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

     //复制聚焦清楚的图
	_img_w = m_CaptureVideo0.Vwidth > m_CaptureVideo1.Vwidth ? m_CaptureVideo1.Vwidth : m_CaptureVideo0.Vwidth;
	_img_h = m_CaptureVideo0.Vheight > m_CaptureVideo1.Vheight ? m_CaptureVideo1.Vheight : m_CaptureVideo0.Vheight;
	_img_a.resize(_img_w * _img_h);
	memset(_img_a.data(),125, _img_w * _img_h);
	//memcpy(_img_a.data(), m_CaptureVideo0.Vbuffer, _img_w * _img_h);

	//复制焦前图
	_img_b.resize(_img_w * _img_h);
	memcpy(_img_b.data(), m_CaptureVideo1.Vbuffer, _img_w * _img_h);

	//复制焦后图
	_img_c.resize(_img_w * _img_h);
	memcpy(_img_c.data(), m_CaptureVideo0.Vbuffer, _img_w * _img_h);

    //发送消息到结果面板
	DiffMeasureResult dmr;
	dmr.width  = _img_w;
	dmr.height = _img_h;
	dmr.a = _img_a.data();
	dmr.b = _img_b.data();
	dmr.c = _img_c.data();

	strMsg = L"Status:   Serial Process【Finished】!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);

	Notify(this, NOTIFY_DIFF_MEASURE, 0, &dmr, 0, &_dmp);
	Notify(this, NOTIFY_MSGVIEW_END, 0, &strMsg);
}

bool DiffMeasurePro::Capture0(bool bTrigger)
{
	if (bTrigger)
		m_CurVideoDevice0->TrigerVideoData();

	strMsg = L"Status:   Capturing0!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);

	//ResetEvent(m_hCaptureEvent0);
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

bool DiffMeasurePro::Capture1(bool bTrigger)
{
	if (bTrigger)
		m_CurVideoDevice1->TrigerVideoData();
	
	strMsg = L"Status:   Capturing1!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);

	//ResetEvent(m_hCaptureEvent1);
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



