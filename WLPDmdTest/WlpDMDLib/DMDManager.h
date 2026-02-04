#pragma once
#include "IDMDManager.h"
#include "WlpAdapter.h"
#include "AlpAdapter.h"

using namespace WlpDMDControl;

class CDMDManager:public IDMDManager
{
public:
	CDMDManager(void);
	~CDMDManager(void);
	bool InitDMD(void* wPara, int type);//【要包括那些默认的DMD参数】
	bool UnInitDMD(void);
	//记得frams是帧数，内存别太小了
	bool DownLoadFrame(unsigned char* p_data,long frams);
	//下载模板
	IPatternGenerate* CreateDmdPattern(int Mode);
  void DestroyDmdPattern(IPatternGenerate* pg);
	bool DownLoadPattern(IPatternGenerate* p_Pattern);
	//设置所有参数
	bool RestPara(void* wPara);
	//DMD状态控制 1播放 2停止 3Trigger 4复位DMD（重新启动DMD)
	void SetStatus(int status);
	//获取帧频
	float GetFrameRate();
	//获取帧频
	void GetDownSize(int* iWidth, int* iHeight);
	//相应模式状态
	bool GetSyncMode(int mode){return false;};
	//获取播放帧数
	int GetFrameCount(){return m_FrameCount;};
	//获取播放状态
	bool IsReady(){return mWlpAdapter->ReadyforTrigger;};
	//获取信息
	virtual WlpDMDInfo* GetInfo(){return &mInfoDMD;};

	virtual char* pixel2bin(char* pSrc, int width,int height, unsigned char bitmask, char* pOut);
	virtual char* bin2pixel(char* pSrc, int width,int height, unsigned char bitmask, char* pOut);

  virtual int GetType();
private:
	IDMDAdapter* mWlpAdapter;
	WlpDMDParas* mWlpDMDParas;
	WlpDMDInfo mInfoDMD;
	int m_FrameCount;
  int m_type;
};

