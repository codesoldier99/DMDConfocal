#pragma once

#include "HConfocalCore.h"
#include "CarlVideo.h"
#include "HGloableFunction.h"

class CMapScanner:public HConfocalPlug,public HVideoRender//,public HMapScanner public IHsmObserver,
{
public:
	CMapScanner(void);
	~CMapScanner(void);

public://实现
	virtual bool InitPlugin(LPVOID p_Param=0, LPVOID p_Param2 = 0);//初始化库
	virtual bool Stop();
	virtual bool Set(void* WParas);//设置参数
	virtual bool Start(void* vParas=0);//启动扫描
	virtual CString GetPluginName() {return L"MapScanner";};
	virtual void* GetUserInterface(int p_Type=0) {return (HConfocalPlug*)this;};//获取扫描接口	
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	friend DWORD WINAPI ScanMapProc(LPVOID lp);
	
private:
	bool Capture();
	void ScanMap();
	bool MoveAxis(HAxis	*m_Axis,float mPos,CString strOut);

private:
	HConfocalCore* m_ConfocalCore;
	HCoreProcess* mMapProcess;//地图处理函数

	HAxis *m_AxisX,*m_AxisY,*m_AxisZ;
	HConfigure* m_Conf;
	HVideoDevice* m_CurVideoDevice;//当前相机
	MapBuildParas* mMapScanParas;//扫地图的参数结构体
	HANDLE m_ScanThread;// 采集数据线程
	HANDLE m_hMoveEvent;// 线程与载物台同步
	HANDLE m_hCaptureEvent;// 线程与采图同步
	bool bScan;//控制线程的扫描
	CString strMsg;

	CPointF m_PointStart,m_PointEnd;
	int m_MoveCount,m_XCount,m_YCount;//移动次数，X、Y各自的移动次数
	int m_MoveTCount,m_XtCount,m_YtCount;//已经移动的次数
	CPointF m_PointNext;//指示下一个要移动到的坐标
	float m_MoveStep;	//单步移动的距离

	HVideoHeader m_CaptureVideo;
};

