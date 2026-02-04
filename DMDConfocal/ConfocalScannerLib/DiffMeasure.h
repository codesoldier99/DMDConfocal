#pragma once

#include "HConfocalCore.h"
#include "HTimer.h"
#include "CarlVideo.h"
#include <vector>
using namespace std;

class DiffMeasure:public HConfocalPlug,public HVideoRender
{
public:
	DiffMeasure(void);
	~DiffMeasure(void);

	friend DWORD WINAPI DiffMeasureProc(LPVOID lp);
	void ScanImage();

public://实现
	virtual bool InitPlugin(LPVOID p_Param=0, LPVOID p_Param2 = 0);//初始化库
	virtual bool UnInitPlugin();
	virtual bool Set(void* WParas);//设置参数
	virtual bool Start(void* vParas=0);//启动扫描
	virtual bool Stop();
	virtual CString GetPluginName() {return L"DiffMeasure";};
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);
	bool Capture(bool triggerInter = false, bool triggerOut = false);
	bool BuildDirectory(CString strPath);
/*private:
  struct DiffMeasureResult
  {
    DiffMeasureResult()
    {
      width = 0;
      height = 0;
      a = 0;
      b = 0;
      c = 0;
    }
    int width;
    int height;
    unsigned char* a;
    unsigned char* b;
    unsigned char* c;
  };*/
private:
	float m_ScanStart,m_ScanStop,m_ScanStep;
	int m_ScanCount,m_ScanCountRec,iSleep;//扫描张数、计数
	float m_ScanSpeed;// Z轴速度
	HTimer m_HTimer;
	HVideoHeader m_CaptureVideo;//帧频的数据
	//CString strSavsPath;

	DiffMeasureParam _dmp;

  // 采集到的图像信息
  int _img_w;
  int _img_h;
  vector<unsigned char> _img_a, _img_b, _img_c;
protected:
	IDMDManager* m_DmdManager;
	HVideoDevice* m_CurVideoDevice;
	HConfocalCore* m_ConfocalCore;
	HConfigure* m_ConfigureFile;
	HCoreProcess* m_ReBuildPro;
	HAxis* m_AxisZ;
	HAxis* m_AxisPIZ;
	float m_Axis_Next;
	float m_Axis_Pos;

	HANDLE m_ScanThread;// 采集数据线程
	HANDLE m_hMoveEvent;// 线程与载物台同步
	HANDLE m_hCaptureEvent;// 线程与采图同步
	bool bScan;
	bool bSaveImg;
	CString strMsg;
	bool bPIZ;



};

