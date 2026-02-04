
#include "StdAfx.h"
#include "C3DCapture.h"
#include "HGloableFunction.h"

C3DCapture::C3DCapture(void)
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
	m_hMoveEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

C3DCapture::~C3DCapture(void)
{
	if (m_CaptureVideo0.Vbuffer)
		delete(m_CaptureVideo0.Vbuffer);
	if (m_CaptureVideo1.Vbuffer)
		delete(m_CaptureVideo1.Vbuffer);
}

bool C3DCapture::InitPlugin(LPVOID p_Param, LPVOID p_Param2)
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
		/****以下用于存图***/
		m_ReBuildPro = m_ConfocalCore->GetCoreProcess(PROCESS_3DReBuild);
	}
	return true;
}

bool C3DCapture::UnInitPlugin()
{
	if (m_CaptureVideo0.Vbuffer)
		delete(m_CaptureVideo0.Vbuffer);
	if(m_CaptureVideo1.Vbuffer)
		delete(m_CaptureVideo1.Vbuffer);
	return true;
}

bool C3DCapture::Stop()
{
	return true;
}

int C3DCapture::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if (pSubject==m_AxisZ->GetSubject())
	{
		if(ID==NOTIFY_AXIS_MOVED)//||NOTIFY_AXIS_POSITION)//
		{
			SetEvent(m_hMoveEvent);
		}
	}
	return 1;
}

bool C3DCapture::Set(void* WParas)
{
	if (WParas == 0)
		return false;
	m_CapParam = *(ReBuildParas*)WParas;
	
	strSavsPath = m_ConfocalCore->GetConfigure()->GetString(L"3DRebuild", L"strPath", L"Confocal_Scaner");
	struct tm *local;
	time_t t;
	t = time(NULL);
	local = localtime(&t);
	CString strMV;
	strMV.Format(_T("\\%d%02d%02d\\%d-%d-%d"), local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
		local->tm_hour, local->tm_min, local->tm_sec);
	strSavsPath = strSavsPath + strMV;
	BuildDirectory(strSavsPath);

	return false;
}

bool C3DCapture::Start(void* vParas)
{
	if (vParas == 0)
		return false;
	m_bConfocal = *(bool*)vParas;

	m_CaptureVideo0.VcaptureTime = 0;//表示当前编号
	m_CaptureVideo0.VnCount = (int)((m_CapParam.fDownPos - m_CapParam.fUpPos) / m_CapParam.fStepPos) + 1;
	m_CaptureVideo1.VcaptureTime = 0;
	m_CaptureVideo1.VnCount = m_CaptureVideo0.VnCount;//表示共有多少张
	ResetEvent(m_hCaptureEvent0);
	ResetEvent(m_hCaptureEvent1);
	ResetEvent(m_hMoveEvent);

	m_ScanThread = CreateThread(NULL, 0, Capture3DProc, this, 0, 0);

	HCorePanel* mDlg = m_ConfocalCore->GetMsgPanel(MSG_TIME);
	mDlg->InitPanel(0);
	this->Attach(mDlg);
	m_ConfocalCore->GetDockablePanel(DOCKPANEL_STAGECTL)
		->GetCWnd()->PostMessage(0x118);
	((CDialogEx*)mDlg->GetCWnd())->DoModal();
	this->Detach(mDlg);

	return true;
}

DWORD WINAPI Capture3DProc( LPVOID lp )
{
	((C3DCapture*)lp)->Capture3D();
	return 0;
}

void C3DCapture::Capture3D()
{
	m_UseAxisZ = m_AxisZ;
	if (m_CapParam.bPIZ)
		m_UseAxisZ = m_AxisPIZ;
	if (!m_UseAxisZ)
	{
		strMsg = L"Info:   Axis is not exist!!!";
		Notify(this, NOTIFY_MSGVIEW_Info, 0, &strMsg);
		Notify(this, NOTIFY_MSGVIEW_END, 0, 0);
		return;
	}

	if (!m_bConfocal)
	{
		m_CurVideoDevice0->SetTrigerMode(TRIGER_SOFT);
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Normal)->push_back(this);

		m_CurVideoDevice1->SetTrigerMode(TRIGER_SOFT);
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_NormalEx)->push_back(this);
	}
	else if (m_bConfocal)
	{
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->push_back(this);
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_ConfocalEx)->push_back(this);
	}
	else
	{
		return;
	}

	//循环抓图保存
	float fCurPos;
	float fNextPos = m_CapParam.fUpPos;
	bool bRun = true;
	int iStepSum = (int)((m_CapParam.fDownPos - m_CapParam.fUpPos) / m_CapParam.fStepPos) + 1;
	int iStepCur = 0;

	while (bRun)
	{
		fCurPos = m_UseAxisZ->GetPosition();//得到当前位置
		if (abs(fCurPos - fNextPos) > 0.001)
		{
			m_UseAxisZ->MoveTo(fNextPos, m_CapParam.fSpeed);//移动
			strMsg = L"Status:   Moving(Capture)!";
			Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
			if (!m_CapParam.bPIZ) //非PI电机的话，需要等待移动到位
			{
				if (WaitForSingleObject(m_hMoveEvent, 10000) == WAIT_TIMEOUT)//等待载物台移动到位
				{
					strMsg = L"Status:   Moving Time Out!";
					Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
					Notify(this, NOTIFY_MSGVIEW_END, 0, 0);
					bRun = false;
					break;
				}
				ResetEvent(m_hMoveEvent);
			}

			strMsg.Format(L"Capture: %d/%d ", iStepCur + 1, iStepSum);
			Notify(this, NOTIFY_MSGVIEW_Info, 0, &strMsg);

			Sleep(m_CapParam.iSleep);//等待震荡时间
			if (!CaptureImg())//抓图+存图
			{
				bRun = false;
				break;
			}
		}
		
		fNextPos += m_CapParam.fStepPos;//下一位
		iStepCur++;
		if (iStepCur >= iStepSum)
		{
			bRun = false;
			break;
		}
	}

	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Normal)->remove(this);
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->remove(this);
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_NormalEx)->remove(this);
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_ConfocalEx)->remove(this);
	if (!m_bConfocal)
	{
		m_CurVideoDevice0->SetTrigerMode(TRIGER_INTERAL);
		m_CurVideoDevice1->SetTrigerMode(TRIGER_INTERAL);
	}

	if (iStepCur >= iStepSum)//代表成功完成了采图
		Notify(this, NOTIFY_MSGVIEW_END, 0, 0);
}

bool C3DCapture::CaptureImg()
{

	bool bCapSuccess = Capture0(!m_bConfocal) && Capture1(!m_bConfocal);
	if (!bCapSuccess || m_CaptureVideo0.Vbuffer == 0 || m_CaptureVideo1.Vbuffer == 0)
	{
		return false;
	}
	m_CaptureVideo0.VcaptureTime++;//编号加一
	m_CaptureVideo1.VcaptureTime++;//编号加一

	CString strName0, strName1;
	if (m_CapParam.bPIZ)
	{
		float mV = m_UseAxisZ->GetPosition();
		mV = mV * 1000;
		strName0.Format(L"%s\\Camera0-FX%04d-%.3fum.jpg", strSavsPath, m_CaptureVideo0.VcaptureTime, mV);
		strName1.Format(L"%s\\Camera1-FX%04d-%.3fum.jpg", strSavsPath, m_CaptureVideo1.VcaptureTime, mV);
	}
	else
	{
		strName0.Format(L"%s\\Camera0-FX%04d.jpg", strSavsPath, m_CaptureVideo0.VcaptureTime);
		strName1.Format(L"%s\\Camera1-FX%04d.jpg", strSavsPath, m_CaptureVideo1.VcaptureTime);
	}
	m_ReBuildPro->SavePic(&m_CaptureVideo0, strName0);
	m_ReBuildPro->SavePic(&m_CaptureVideo1, strName1);
	return true;
}

bool C3DCapture::Capture0(bool bTrigger)
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
	strMsg = L"Status:   Capturing0 success!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
	return true;
}

bool C3DCapture::Capture1(bool bTrigger)
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
	strMsg = L"Status:   Capturing1 success!";
	Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
	return true;
}

void C3DCapture::Renderer(HVideoHeader* pHeader, LPBYTE pBuffer)
{
	if (pHeader->Vuser == 0)
		Func_FlushImg(pHeader, pBuffer, m_CaptureVideo0, m_hCaptureEvent0);
	else
		Func_FlushImg(pHeader, pBuffer, m_CaptureVideo1, m_hCaptureEvent1);
}

void C3DCapture::Func_FlushImg(HVideoHeader* pSrcHeader, LPBYTE pBuffer
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

bool C3DCapture::BuildDirectory(CString strPath)
{
	CString strSubPath;
	CString strInfo;
	int nCount = 0;
	int nIndex = 0;

	//查找字符"\\"的个数
	do
	{
		nIndex = strPath.Find(L"\\", nIndex) + 1;
		nCount++;
	} while ((nIndex - 1) != -1);
	nIndex = 0;
	//检查，并创建目录
	while ((nCount - 1) >= 0)
	{
		nIndex = strPath.Find(L"\\", nIndex) + 1;
		if ((nIndex - 1) == -1)
		{
			strSubPath = strPath;
		}
		else
			strSubPath = strPath.Left(nIndex);
		if (!PathFileExists(strSubPath))
		{
			if (!::CreateDirectory(strSubPath, NULL))
			{
				strInfo = L"Build Directory";
				strInfo += strSubPath;
				strInfo += L" Fail!";
				AfxMessageBox(strInfo, MB_OK);
				return FALSE;
			}
		}
		nCount--;
	};
	return TRUE;
}

