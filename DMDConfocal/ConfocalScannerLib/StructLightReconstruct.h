#pragma once

#include "IDMDManager.h"
#include "HImgProcess.h"
#include "CarlVideo.h"
#include "HConfocalCore.h"
//#include "MergeView.h"
#include <vector>
using namespace std;

using namespace WlpDMDControl;

class StructLightReconstruct: public HVideoRender,public HConfocalPlug//,public IHsmObserver,public IHsmSubject
{

public:
	StructLightReconstruct(void);
	~StructLightReconstruct(void);
	
	virtual bool Set(void* WParas){return true;};//设置参数
	virtual bool Start(void* vParas=0){return true;};//启动扫描
	virtual bool Stop(){return true;};
	friend DWORD WINAPI ImageProc(LPVOID lp);
	void ProcessImage();

	void GetParam();
	void RestMerge(void);
	void Save_Result();

	virtual bool InitPlugin(LPVOID p_Param=0, LPVOID p_Param2 = 0);
	virtual CString GetPluginName() {return L"SIMode";};
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0
		,float fParam=0.0f,void* mParam=0);
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	bool BuildDirectory(CString strPath);

	//存图线程
	friend DWORD WINAPI SaveProc(LPVOID lp);
	void DoSaveProc();
	HANDLE m_hSaveThread;//存图线程句柄
	int iSaveW,iSaveH,iBit;
	CString thSavePath;
	LPBYTE m_SaveTemp;//要存储的图
private:
  // 是否是暗的图像
  bool IsDarkImage(int w, int h, unsigned char* img, double thr);
  // 重构图像
  int ReconstructImage(vector<vector<unsigned char>>& imgs, unsigned char* merge);
protected:

	IDMDManager* m_DmdManage;
	HConfocalCore* m_ConfocalCore;
	HVideoRender* m_RenderChain;
	long iWidth,iHeight;
	int wbit,m_NowIndex;
	LPBYTE m_bmpResult;//存储最终结果图
	LPBYTE m_bmpTemp;//过程图
	long m_imgsize;
	bool m_bStart;
	bool m_bExit;
	HANDLE m_hGrabberThread;// 采集数据线程
	HANDLE m_hEvent;// 线程退出事件

	int m_MergeCount;
	int m_iBackTh;//背景阈值
	CString strPath;

	CString strSaveFile;
	bool m_bSaveBmp;//存储共聚焦结果图
	bool bSaveConfocal;//存储共聚焦过程图

  vector<vector<unsigned char>> _imgs;

  bool _has_dark_img;
};

