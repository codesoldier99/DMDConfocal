// ConfocalUILib.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "ResizeDlg.h"
#include "StageCtlDlg.h"
#include "VideoConfDlg.h"
#include "VideoView.h"
#include "DMDdnDlg.h"
#include "MergeView.h"
#include "ScannerDlg.h"
#include "GLviewWnd.h"
#include "MsgViewBox.h"
#include "ColorView.h"
#include "MapViewDlg.h"
#include "WndBmp.h"
#include "ShowCVDlg.h"
#include "ViewScan.h"
#include "SIView.h"
#include "DifferentialSettingsDlg.h"
#include "DiffMeasureView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: 更改错误代码以符合您的需要
			_tprintf(_T("错误: MFC 初始化失败\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: 在此处为应用程序的行为编写代码。
		}
	}
	else
	{
		// TODO: 更改错误代码以符合您的需要
		_tprintf(_T("错误: GetModuleHandle 失败\n"));
		nRetCode = 1;
	}

	return nRetCode;
}


CConfocalUILib::CConfocalUILib( void )
{

}

HCorePanel* CConfocalUILib::GetDockPanel( DOCKPANEL_TYPE p_ID )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HCorePanel* p=0;
	HCorePanel* p2=0;
	switch(p_ID)
	{
		case DOCKPANEL_VIDEO:

			p=new CResizeDlg();
			((CResizeDlg*)p)->Create(CResizeDlg::IDD);	//建自动滚动窗

			p2=new CVideoConfDlg();
			((CVideoConfDlg*)p2)->Create(CVideoConfDlg::IDD);//视频窗口	

			((CResizeDlg*)p)->SetChildWnd(((CVideoConfDlg*)p2));
			break;

		case DOCKPANEL_VIDEOEX:

			p = new CResizeDlg();
			((CResizeDlg*)p)->Create(CResizeDlg::IDD);	//建自动滚动窗

			p2 = new CVideoConfDlg();
			((CVideoConfDlg*)p2)->Create(CVideoConfDlg::IDD);//视频窗口	

			((CResizeDlg*)p)->SetChildWnd(((CVideoConfDlg*)p2));
			((CResizeDlg*)p)->SetWindowTextW(L"VideoControl Ex");
			break;

		case DOCKPANEL_SCANERDLG:
			p=new CResizeDlg();
			((CResizeDlg*)p)->Create(CResizeDlg::IDD);	//建自动滚动窗
			p2=new CScannerDlg();
			((CScannerDlg*)p2)->Create(CScannerDlg::IDD);//建载物台控制子窗口	
			((CResizeDlg*)p)->SetChildWnd(((CScannerDlg*)p2));
			break;

		case DOCKPANEL_STAGECTL:
			p=new CResizeDlg();
			((CResizeDlg*)p)->Create(CResizeDlg::IDD);	//建自动滚动窗
			p2=new CStageCtlDlg();
			((CStageCtlDlg*)p2)->Create(CStageCtlDlg::IDD);//建载物台控制子窗口	
			((CResizeDlg*)p)->SetChildWnd(((CStageCtlDlg*)p2));
			break;
		case DOCKPANEL_DMDCTL:
			p=new CResizeDlg();
			((CResizeDlg*)p)->Create(CResizeDlg::IDD);	//建自动滚动窗
			p2=new CDMDdnDlg();
			((CDMDdnDlg*)p2)->Create(CDMDdnDlg::IDD);//建DMD子窗口	
			((CResizeDlg*)p)->SetChildWnd(((CDMDdnDlg*)p2));
			//p=new CDMDdnDlg();
			//((CDMDdnDlg*)p)->Create(CDMDdnDlg::IDD);//建DMD子窗口	
			break;
		case  DOCKPANEL_MAPVIEWDLG:
			p=new CMapViewDlg();
			((CMapViewDlg*)p)->Create(CMapViewDlg::IDD);//建DMD子窗口	
			break;
		case DOCKPANEL_DCAMCTL:
			//p=new CResizeDlg();
			//((CResizeDlg*)p)->Create(CResizeDlg::IDD);	//建自动滚动窗
			////p2=new CDcamCtlDlb();
			////((CDcamCtlDlb*)p2)->Create(CDcamCtlDlb::IDD);//Dcam相机控制窗口	
			////改一下
			//p2=new CPluginDlg();
			//((CPluginDlg*)p2)->Create(CPluginDlg::IDD);//plugin 窗口
			//((CResizeDlg*)p)->SetChildWnd(((CVideoConfDlg*)p2));
			break;
		case DOCKPANEL_DIFF:
			p = new CResizeDlg();
			((CResizeDlg*)p)->Create(CResizeDlg::IDD);	//建自动滚动窗
			p2 = new DifferentialSettingsDlg();
			((DifferentialSettingsDlg*)p2)->Create(IDD_DIFFERENTIALSETTINGSDLG);//视频窗口	
			((CResizeDlg*)p)->SetChildWnd(((DifferentialSettingsDlg*)p2));
			break;
		default:
			break;
	}
	return (HCorePanel*)p;
}

HCorePanel* CConfocalUILib::GetViewPanel( VIEWPANEL_TYPE p_ID )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HCorePanel* p=0;
	CWnd* p2=0;
	CRect	rc;
	rc.SetRect(0,0,10,10);
	switch(p_ID)
	{
		case VIEWPANEL_REALTIME:
			p=new CVideoView();
			p2=p->GetCWnd();
			p2->Create(NULL,L"RealTime View", WS_CHILD|WS_CLIPCHILDREN | WS_VISIBLE, rc, AfxGetMainWnd(), 0 );
			break;
		case VIEWPANEL_REALTIMEEX:
			p = new CVideoView();
			p2 = p->GetCWnd();
			p2->Create(NULL, L"RealTime View Ex", WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE, rc, AfxGetMainWnd(), 0);
			break;
		case VIEWPANEL_CONFOCAL:
			p=new CMergeView();
			p2=p->GetCWnd();
			p2->Create(NULL,L"Confocal View", WS_CHILD|WS_CLIPCHILDREN | WS_VISIBLE, rc, AfxGetMainWnd(), 0 );
			break;
		case VIEWPANEL_CONFOCALEX:
			p = new CMergeView();
			p2 = p->GetCWnd();
			p2->Create(NULL, L"Confocal View EX", WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE, rc, AfxGetMainWnd(), 0);
			break;
		case VIEWPANEL_3D://3D显示
			p=new CGLviewWnd();//3D显示窗口	
 			p2=p->GetCWnd();
 			p2->Create(NULL,L"3D View", WS_CHILD|WS_CLIPCHILDREN | WS_VISIBLE, rc, AfxGetMainWnd(), 0 );
			break;
		case VIEWPANEL_COLOR:
			p=new CColorView();
			p2=p->GetCWnd();
			p2->Create(NULL,L"Color View", WS_CHILD|WS_CLIPCHILDREN | WS_VISIBLE, rc, AfxGetMainWnd(), 0 );
			break;
		case VIEWPANEL_ViewFocus://dmd view
			p=new CRenderWnd();//dmd控制窗口	
			p2=p->GetCWnd();
			p2->Create(NULL,L"MutiFocuse View", WS_CHILD|WS_CLIPCHILDREN | WS_VISIBLE, rc, AfxGetMainWnd(), 0 );
			//p2->SetWindowTextW(L"MutiFocuse View");
			break;
		case VIEWPANEL_3DRange:
			p=new ViewScan();//dmd控制窗口	
			p2=p->GetCWnd();
			p2->Create(NULL,L"Multilayer View", WS_CHILD|WS_CLIPCHILDREN | WS_VISIBLE, rc, AfxGetMainWnd(), 0 );
			//p2->SetWindowTextW(L"Multilayer View");
			break;
		case VIEWPANEL_SI:
			p = new SIView();
			p2 = p->GetCWnd();
			p2->Create(NULL, L"SL View", WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE, rc, AfxGetMainWnd(), 0);
			break;
		case VIEWPANEL_DIFF_MEASURE_VIEW:
			p = new DiffMeasureView();
			p2 = p->GetCWnd();
			p2->Create(NULL, L"DiffMeasure View", WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE, rc, AfxGetMainWnd(), 0);
			break;
		default:
			break;
	}
	return (HCorePanel*)p;
}

HCorePanel* CConfocalUILib::GetMsgPanel( MESSAGE_TYPE p_ID,bool bModal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HCorePanel* p=0;
	switch(p_ID)
	{
		case MSG_TIME:
			p=new CMsgViewBox();
			if (bModal)
				((CMsgViewBox*)p)->Create(CMsgViewBox::IDD);
			break;
		case  MSG_CALCV:
			p=new CShowCVDlg();
			if (bModal)
				((CShowCVDlg*)p)->Create(CShowCVDlg::IDD);
			break;
		default:
			break;
	}	
	return (HCorePanel*)p;
}

