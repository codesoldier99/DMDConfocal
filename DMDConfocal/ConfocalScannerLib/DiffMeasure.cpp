#include "StdAfx.h"
#include "DiffMeasure.h"
#include "HGloableFunction.h"

DiffMeasure::DiffMeasure(void)
{
	m_AxisZ=0;
	m_AxisPIZ=0;
	m_ConfocalCore=0;
	m_CaptureVideo.Vsize=0;//表示没有东西
	m_CaptureVideo.Vbuffer=0;
	m_hMoveEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_hCaptureEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_ScanThread=0;
	bScan=false;
	bSaveImg=false;
	strMsg=L"";
	_img_w = _img_h = 0;
	m_DmdManager = 0;
}

DiffMeasure::~DiffMeasure(void)
{

}

bool DiffMeasure::InitPlugin(LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	//初始化载物台
	if(m_ConfocalCore)
	{
		if (m_ConfocalCore->GetGearBox())
			m_AxisZ = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_Z);
		if (m_AxisZ)
			m_AxisZ->GetSubject()->Attach(this);
		if (m_ConfocalCore->GetGearBox())
			m_AxisPIZ = m_ConfocalCore->GetGearBox()->GetAxis(AXIS_PIZ);
		/*if (m_AxisPIZ) //PIZ轴是开环控制，没有反馈，只能靠延时
			m_AxisPIZ->GetSubject()->Attach(this);*/
	}
	m_ConfigureFile=m_ConfocalCore->GetConfigure();//初始化其它参数
	m_CurVideoDevice=m_ConfocalCore->GetCurVideo();//获取相机

	/*HConfocalPlug* m = m_ConfocalCore->GetPlugin(PLUGIN_CONFOCALMODE);
	((IHsmSubject*)this)->Attach(m);
	m->Attach(this);*/
	m_DmdManager = m_ConfocalCore->GetDmdManager();//获取DMD对象

	return true;
}

bool DiffMeasure::UnInitPlugin()
{
	bScan=false;
	return true;
}

bool DiffMeasure::Stop()
{
	bScan=false;
	return true;
}

bool DiffMeasure::Set(void* WParas)
{
	if (WParas==0)
		return false;
	_dmp = *(DiffMeasureParam*)WParas;
	return true;
}

bool DiffMeasure::Start(void* vParas)//Start(bool bConfocal)
{
	ResetEvent(m_hMoveEvent);
	ResetEvent(m_hCaptureEvent);
	bScan = true;
	m_ScanThread = CreateThread(NULL, 0, DiffMeasureProc, this, 0, 0);//这个开线程

	HCorePanel* mDlg=m_ConfocalCore->GetMsgPanel(MSG_TIME);
	mDlg->InitPanel(0);
	this->Attach(mDlg);
	m_ConfocalCore->GetDockablePanel(DOCKPANEL_STAGECTL)->GetCWnd()->PostMessage(0x118);
	((CDialogEx*)mDlg->GetCWnd())->DoModal();  //创建模态对话框，阻止程序
	this->Detach(mDlg);
	WaitForSingleObject(m_ScanThread,2000);
	bScan=false;//退出线程

	return true;
}

void DiffMeasure::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
{
	if(m_CaptureVideo.Vsize!=pHeader->Vsize)
	{
		if(m_CaptureVideo.Vbuffer)
			delete(m_CaptureVideo.Vbuffer);
		m_CaptureVideo.Vbuffer=(LPBYTE)malloc(pHeader->Vsize);
		m_CaptureVideo.Vwidth=pHeader->Vwidth;
		m_CaptureVideo.Vheight=pHeader->Vheight;
		m_CaptureVideo.Vsize=pHeader->Vsize;
		m_CaptureVideo.VwBit=pHeader->VwBit;
	}
	if (!pBuffer)
		return;
	memcpy(m_CaptureVideo.Vbuffer,pBuffer,m_CaptureVideo.Vsize);

	SetEvent(m_hCaptureEvent);
}

int DiffMeasure::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if (pSubject == m_AxisZ->GetSubject())
	{
		if (ID == NOTIFY_AXIS_MOVED || ID == NOTIFY_AXIS_POSITION)
			SetEvent(m_hMoveEvent);
	}
	return 0;
}

DWORD WINAPI DiffMeasureProc( LPVOID lp )
{
	((DiffMeasure*)lp)->ScanImage();
	return 0;
}

void DiffMeasure::ScanImage()
{
	// 得到 Z 移动接口
	HAxis* z(m_AxisZ);
	if (_dmp.z_ctrl_mode && m_AxisPIZ)
	{
		z = m_AxisPIZ;
	}
	//判断触发方式
	bool bTrrigerOut	= _dmp.focus_cap_mode == HConfocalCore::RenderChain_Confocal;
	bool bTrrigerIn	= _dmp.focus_cap_mode == HConfocalCore::RenderChain_Normal;

	// 1. 根据采图方式，绑定渲染链
	if (_dmp.focus_cap_mode == HConfocalCore::RenderChain_Normal)
	{
		m_CurVideoDevice->SetTrigerMode(TRIGER_SOFT);
		m_ConfocalCore->GetRenderChain()->push_back(this);//加入渲染
	}
	else if (_dmp.focus_cap_mode == HConfocalCore::RenderChain_Confocal)
	{
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->push_back(this);
	}
	else
	{
		m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_SL)->push_back(this);
	}

	// 2. 抓取当前图，为焦面图
	/*if (!Capture(bTrrigerIn, bTrrigerOut))
	{
		Notify(this, NOTIFY_MSGVIEW_END, 0, 0);
		return;
	}
	_img_w = m_CaptureVideo.Vwidth;
	_img_h = m_CaptureVideo.Vheight;
	_img_a.resize(_img_w * _img_h);
	memcpy(_img_a.data(), m_CaptureVideo.Vbuffer, _img_w * _img_h);*/

	// 3. Z 向上移动离焦量
	if (z)
	{
		z->MoveRef(-_dmp.defocus / 1000.0);
		strMsg = L"Status:   Moving!";
		Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
		if (z == m_AxisZ)
		{
			if (WaitForSingleObject(m_hMoveEvent, 10000) == WAIT_TIMEOUT)//等待移动到位
			{
				strMsg = L"Status:   Moving Time Out!";
				Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
				bScan = false;
				return;
			}
		}
		else
			Sleep(100);
	}
	Sleep(_dmp.iSleep>0? _dmp.iSleep:200);
	if (!Capture(bTrrigerIn, bTrrigerOut))    //抓取图像
	{
		Notify(this, NOTIFY_MSGVIEW_END, 0, 0);
		return;
	}
	_img_w = m_CaptureVideo.Vwidth;
	_img_h = m_CaptureVideo.Vheight;
	_img_b.resize(_img_w * _img_h);
	memcpy(_img_b.data(), m_CaptureVideo.Vbuffer, _img_w * _img_h);

	// 4. Z 向下移动2倍离焦量 + 空回
	if (z)
	{
		z->MoveRef((_dmp.defocus * 2 + _dmp.backlash) / 1000.0);
		strMsg = L"Status:   Moving!";
		Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
		if (z == m_AxisZ)
		{
			if (WaitForSingleObject(m_hMoveEvent, 10000) == WAIT_TIMEOUT)//等待移动到位
			{
				strMsg = L"Status:   Moving Time Out!";
				Notify(this, NOTIFY_MSGVIEW_SHOW, 0, &strMsg);
				bScan = false;
				return;
			}
		}
		else
			Sleep(100);
	}
	Sleep(_dmp.iSleep > 0 ? _dmp.iSleep : 200);
	if (!Capture(bTrrigerIn, bTrrigerOut))
	{
		Notify(this, NOTIFY_MSGVIEW_END, 0, 0);
		return;
	}
	_img_w = m_CaptureVideo.Vwidth;
	_img_h = m_CaptureVideo.Vheight;
	_img_c.resize(_img_w * _img_h);
	memcpy(_img_c.data(), m_CaptureVideo.Vbuffer, _img_w * _img_h);

    //没有焦面图，补充一张焦面图
	_img_a.resize(_img_w * _img_h);
	memset(_img_a.data(), 188, _img_w * _img_h);


	//发送消息到结果面板
	DiffMeasureResult dmr;
	dmr.width = _img_w;
	dmr.height = _img_h;
	dmr.a = _img_a.data();
	dmr.b = _img_b.data();
	dmr.c = _img_c.data();
	Notify(this, NOTIFY_DIFF_MEASURE, 0, &dmr, 0, &_dmp);

	//删除 Render
	m_ConfocalCore->GetRenderChain()->remove(this);//删除视频
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal)->remove(this);
	m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_SL)->remove(this);
	//恢复视频
	if (_dmp.focus_cap_mode == HConfocalCore::RenderChain_Normal)
	{
		m_CurVideoDevice->SetTrigerMode(TRIGER_INTERAL);
	}
	//回到初始位置
	if (z)
	{
		z->MoveRef(-_dmp.defocus / 1000.0);
	}
	Notify(this, NOTIFY_MSGVIEW_END, 0, 0);
}


// | 图像截取的代码 |
bool DiffMeasure::Capture(bool triggerInter, bool triggerOut)
{
	if (triggerInter)
		m_CurVideoDevice->TrigerVideoData();
	else if (triggerOut)
	{
		if (m_DmdManager)
		{
			m_DmdManager->SetStatus(DMD_STOP);
			Notify(this, NOTIFY_CONFOCAL_RESUM);
			m_DmdManager->SetStatus(DMD_START);
			m_DmdManager->SetStatus((DMD_Trigger));
		}		
	}

	int iCount=0;
	strMsg=L"Status:   Capturing!";
	Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
	ResetEvent(m_hCaptureEvent);

	while(WaitForSingleObject(m_hCaptureEvent,1300)==WAIT_TIMEOUT)
	{
		iCount++;
		if (iCount<=6)
		{
			if (triggerInter)
				m_CurVideoDevice->TrigerVideoData();
		}
		else
		{
			//AfxMessageBox(L"图像采集超时！");
			strMsg=L"Status:   Capturing Time Out!";
			Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);
			return false;
			//break;
		}		
	}
	ResetEvent(m_hCaptureEvent);
	strMsg=L"Status:   Processing!";
	Notify(this,NOTIFY_MSGVIEW_SHOW,0,&strMsg);

  m_CaptureVideo.VcaptureTime++;//编号加一
	if (!bSaveImg)
		return true;
	return true;
}

bool DiffMeasure::BuildDirectory(CString strPath)
{
	CString strSubPath;
	CString strInfo;
	int nCount = 0; 
	int nIndex = 0;

	//查找字符"\\"的个数
	do
	{
		nIndex = strPath.Find(L"\\",nIndex) + 1;
		nCount++;
	}while( (nIndex-1) != -1);
	nIndex = 0;
	//检查，并创建目录
	while( (nCount-1) >= 0)
	{
		nIndex = strPath.Find(L"\\",nIndex) + 1;
		if( (nIndex - 1) == -1)
		{
			strSubPath = strPath;
		}
		else
			strSubPath = strPath.Left(nIndex);
		if(!PathFileExists(strSubPath))
		{
			if(!::CreateDirectory(strSubPath,NULL))
			{
				strInfo = L"Build Directory";
				strInfo += strSubPath;
				strInfo += L" Fail!";
				AfxMessageBox(strInfo,MB_OK);
				return FALSE;
			}
		}
		nCount--;
	};
	return TRUE;
}
