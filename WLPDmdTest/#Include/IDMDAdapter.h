#pragma once
class IDMDAdapter
{
public:
  virtual ~IDMDAdapter() {};

  virtual bool OpenDev(void) = 0;
  virtual void CloseDev(void) = 0;

  virtual bool InitForDownload(UINT m_FrameMargin, UINT m_FrameInterval, UINT gray, int trigeMode, int trigeSync) = 0;
  virtual bool TriggerExtSet(ULONG delay, bool polay, ULONG plusewidth, ULONG ImageNum) = 0;

  virtual void GetPicSize(int* iWidth, int* iHeight) = 0;
  virtual bool DownloadPic(unsigned char* pPicData, int iPicCount) = 0;

  virtual int Start() = 0;
  virtual void Stop(void) = 0;

  virtual LONG SetTrigerMode1(int trigeMode, int trigeSync) = 0;
  virtual LONG SetTrigerMode2(int trigeMode, int trigeSync) = 0;

  virtual bool Trigger() = 0;

  virtual char* UsePixel2bin(char* pSrc, int width, int height, unsigned char bitmask, char* pOut) = 0;
  virtual char* UseBin2pixel(char* pSrc, int width, int height, unsigned char bitmask, char* pOut) = 0;
public:
  bool ReadyforTrigger;//是否等待被触发

  float RAMsizeGB;//RAM大小
  UINT RAMsizeMB;//RAM大小
  LONG m_BitFrameSize;//指示根据DMD尺寸的大小显示的图像大小
};

