
#pragma once

#ifndef IDMDMANAGER_H
#define IDMDMANAGER_H

#include "IPatternGenerate.h"
#include "IHsmObserver.h"

#define Notify_WLPDMD_OPEN 6001
#define Notify_WLPDMD_START 6002
#define Notify_WLPDMD_STOP 6003

typedef struct
{
	UINT m_FrameMargin;
	UINT m_FrameInterval;
	UINT gray;
	int trigeMode;
	int trigeSync;
	ULONG delay;
	bool polay;
	ULONG plusewidth;

	CString strInfo;
	float RAMsizeGB;//RAM大小
	UINT RAMsizeMB;//RAM大小
	LONG m_BitFrameSize;//指示根据DMD尺寸的大小显示的图像大小
}WlpDMDParas;

typedef struct
{
	CString strInfo;
	float RAMsizeGB;//RAM大小
	UINT RAMsizeMB;//RAM大小
	LONG m_BitFrameSize;//指示根据DMD尺寸的大小显示的图像大小
}WlpDMDInfo;

namespace WlpDMDControl
{
	class IDMDManager:public IHsmSubject
	{
		//播放模式定义 
		#define DMD_SYNC_INTERNAL	1
		#define DMD_SYNC_EXTERNAL	2
		#define DMD_PLAY_LOOP_EN	3
		#define DMD_PLAY_LOOP_DIS	4
		//状态控制定义 
		#define DMD_START	1
		#define DMD_STOP	2
		#define DMD_Trigger	3

	public:
		virtual ~IDMDManager(void){};
		//初始化 0成功 1串口失败 2usb失败 3都失败
		virtual bool InitDMD(void* wPara, int type)=0;//【要包括那些默认的DMD参数】
		virtual bool UnInitDMD(void)=0;
		//记得frams是帧数，内存别太小了
		virtual bool DownLoadFrame(unsigned char* p_data,long frams)=0;	
		//下载模板
		virtual IPatternGenerate* CreateDmdPattern(int Mode)=0;
    virtual void DestroyDmdPattern(IPatternGenerate* pg) = 0;
		virtual bool DownLoadPattern(IPatternGenerate* p_Pattern)=0;
		//设置所有参数
		virtual bool RestPara(void* wPara)=0;
		virtual void SetStatus(int status)=0;
		//获取帧频
		virtual float GetFrameRate()=0;	
		//获取大小
		virtual void GetDownSize(int* iWidth, int* iHeight)=0;
		//相应模式状态
		virtual bool GetSyncMode(int mode)=0;
		//通信口是否正常
		virtual bool IsReady()=0;
		//获取播放帧数
		virtual int GetFrameCount()=0;	
		//获取信息
		virtual WlpDMDInfo* GetInfo()=0;
		//下载灰度图像之前的切换
		virtual char* pixel2bin(char* pSrc, int width,int height, unsigned char bitmask, char* pOut)=0;
		virtual char* bin2pixel(char* pSrc, int width,int height, unsigned char bitmask, char* pOut)=0;

    virtual int GetType() = 0;
	};
}
#endif
