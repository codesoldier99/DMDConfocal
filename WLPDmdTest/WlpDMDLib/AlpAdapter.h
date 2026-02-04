#pragma once

#include "IDMDAdapter.h"
#include "Projector.h"

class AlpAdapter : public IDMDAdapter
{
public:
  AlpAdapter();
  virtual ~AlpAdapter();

  bool OpenDev(void);
  void CloseDev(void);

  bool InitForDownload(UINT m_FrameMargin, UINT m_FrameInterval, UINT gray, int trigeMode, int trigeSync);
  bool TriggerExtSet(ULONG delay, bool polay, ULONG plusewidth, ULONG ImageNum);

  void GetPicSize(int* iWidth, int* iHeight);
  bool DownloadPic(unsigned char* pPicData, int iPicCount);

  int Start();
  void Stop(void);

  LONG SetTrigerMode1(int trigeMode, int trigeSync);
  LONG SetTrigerMode2(int trigeMode, int trigeSync);

  bool Trigger();

  char* UsePixel2bin(char* pSrc, int width, int height, unsigned char bitmask, char* pOut);
  char* UseBin2pixel(char* pSrc, int width, int height, unsigned char bitmask, char* pOut);
private:
  void Binary2Gray(int width, int height, unsigned char* src, unsigned char* dst);
private:
  CProjector m_proj;

  CProjector::CDevProperties m_props;
  CProjector::CSynchProperties m_sync_props;
  CProjector::CTimingEx m_timing;

  int m_trigger_mode;

  int m_frames;

  int m_cur_frame;
};

