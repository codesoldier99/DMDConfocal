#include "stdafx.h"
#include "AlpAdapter.h"

AlpAdapter::AlpAdapter(void)
{
  m_trigger_mode = 0;
  ReadyforTrigger = false;
  RAMsizeGB = 0;
  RAMsizeMB = 0;
  m_BitFrameSize = 0;
  m_frames = 0;
  m_cur_frame = 0;
}

AlpAdapter::~AlpAdapter(void)
{
  CloseDev();
}

bool AlpAdapter::OpenDev()
{
  CloseDev();

  int ret = m_proj.Alloc();

  if (ret == ALP_OK)
  {
    m_proj.GetDevProperties(m_props);
    m_proj.SetSyncOutputMode(ALP_LEVEL_HIGH);

    m_BitFrameSize = m_proj.GetWidth() * m_proj.GetHeight();

    RAMsizeMB = (UINT)(m_props.FreeMemory * (double)m_BitFrameSize / (1024 * 1024)) / 8;
    RAMsizeGB = (float)(RAMsizeMB / 1024.0);

    m_proj.GetSeqProperties(m_timing); // Check
  }

  return ret == ALP_OK;
}

void AlpAdapter::CloseDev()
{
  m_frames = 0;
  m_cur_frame = 0;
  if (m_proj.IsProjection())
  {
    m_proj.ProjStop();
  }
  if (m_proj.IsValidSequence())
  {
    m_proj.SequenceFree();
  }
  if (m_proj.IsConnected())
  {
    m_proj.Free();
  }
}

LONG AlpAdapter::SetTrigerMode1(int trigeMode, int trigeSync)
{
  return 0;
}

LONG AlpAdapter::SetTrigerMode2(int trigeMode, int trigeSync)
{
  return 0;
}

bool  AlpAdapter::InitForDownload(UINT m_FrameMargin, UINT m_FrameInterval, UINT gray, int trigeMode, int trigeSync)
{
  m_timing.PictureTime = m_FrameMargin + m_FrameInterval;
  m_timing.IlluminateTime = m_FrameMargin;
  m_timing.BitNum = gray;

  m_trigger_mode = trigeMode;

  // 忽略 trigeSync

  // 设置序列属性
  if (m_proj.IsValidSequence())
  {
    m_proj.SetSeqProperties(m_timing);
  }

  ReadyforTrigger = true;

  return true;
}

void AlpAdapter::GetPicSize(int* iWidth, int* iHeight)
{
  if (m_proj.IsConnected())
  {
    if (iHeight)*iHeight = m_proj.GetHeight();
    if (iWidth)*iWidth = m_proj.GetWidth();
  }
  else
  {
    if (iWidth)*iWidth = 1920;
    if (iHeight)*iHeight = 1080;
  }
}

bool AlpAdapter::DownloadPic(unsigned char* pPicData, int iPicCount)
{
  if (!m_proj.IsConnected())
  {
    return false;
  }
  if (m_proj.IsProjection())
  {
    return false;
  }
  m_proj.SequenceFree();
  m_frames = 0;
  m_cur_frame = 0;
  m_proj.SequenceAlloc(m_timing.BitNum, iPicCount);
  if (m_timing.BitNum == 1)
  {
    int w = m_proj.GetWidth();
    int h = m_proj.GetHeight();
    unsigned char* tmp = new unsigned char[w * h];
    for (int i = 0; i < iPicCount; i++)
    {
      Binary2Gray(w, h, pPicData + i * w * h / 8, tmp);
      m_proj.AddImage(tmp, w, h);
    }
    delete[]tmp;
    m_frames = iPicCount;
    m_cur_frame = 0;
    return true;
  }
  else if (m_timing.BitNum == 8)
  {
    int w = m_proj.GetWidth();
    int h = m_proj.GetHeight();
    for (int i = 0; i < iPicCount; i++)
    {
      m_proj.AddImage(pPicData + i * w * h, w, h);
    }
    m_frames = iPicCount;
    return true;
  }
  return false;
}

int AlpAdapter::Start()
{
  if (!m_proj.IsValidSequence())
  {
    return -1;
  }

  if (m_trigger_mode == 0)
  {
    m_cur_frame = 0;
    m_proj.SetSeqStartEnd(m_cur_frame, m_cur_frame);
  }
  else if (m_trigger_mode == 6) // Loop
  {
    m_proj.SetSeqStartEnd(0, m_frames - 1);
  }
  else
  {
    m_proj.SetSeqStartEnd(0, m_frames - 1);
  }

  return 0;
}

void AlpAdapter::Stop()
{
  ReadyforTrigger = false;
  m_proj.ProjStop();
}

bool AlpAdapter::Trigger()
{
  if (ReadyforTrigger != true)
  {
    return false;
  }
  if (m_trigger_mode == 0)
  {
    m_proj.SetSeqStartEnd(m_cur_frame, m_cur_frame);
    m_proj.ProjStartContinuous();
    m_cur_frame++; // 是否要增加？
    if (m_cur_frame >= m_frames)
    {
      m_cur_frame = 0;
    }
  }
  else if (m_trigger_mode == 2)
  {
    if (m_proj.IsProjection())
    {
      m_proj.ProjStop();
    }
    m_proj.SetSeqStartEnd(0, m_frames - 1);
    m_proj.ProjStart();
  }
  else if (m_trigger_mode == 4)
  {
    if (m_proj.IsProjection())
    {
      m_proj.ProjStop();
    }
    m_proj.SetSeqStartEnd(0, m_frames - 1);
    m_proj.ProjStart();
  }
  else if (m_trigger_mode == 6)
  {
    if (!m_proj.IsProjection())
    {
      m_proj.ProjStartContinuous();
    }
  }
  return true;
}

bool AlpAdapter::TriggerExtSet(ULONG delay, bool polarity, ULONG plusewidth, ULONG ImageNum)
{
  m_timing.SynchDelay = delay;
  m_timing.SynchPulseWidth = plusewidth;
  m_props.Polarity = polarity;

  // 忽略 ImageNum
  return true;
}

char* AlpAdapter::UsePixel2bin(char* pSrc, int width, int height, unsigned char bitmask, char* pOut)
{
  if (pSrc == NULL || pOut == NULL || width % 8 != 0)
    return NULL;
  char* ret = pOut;
  int cnt = (width / 8)*height;  //once operate 8bytes.
  unsigned char bm;
  int i, j, k, m;
  char c;
  UINT64 temp;
  UINT64 mark[8];
  for (i = 0; i < 8; i++)
  {
    temp = 0x80 >> i;
    mark[i] = 0;
    for (int j = 0; j < 8; j++)
      mark[i] |= temp << j * 8;
  }
  UINT64* p = (UINT64*)pSrc;
  for (i = 0; i < cnt; i++)
  {
    for (j = 0, m = 0, bm = bitmask; j < 8 && (bm & 0x80); j++, bm <<= 1)
    {	// get  A7,B7,C7,D7,E7,F7,G7,H7 (most high bit), then A6,B6,C6..H6.
      c = 0;
      temp = *p & mark[j];
      temp >>= (7 - j);
      for (k = 0; k < 8; k++)
      {
        c |= ((temp & 0x01) << k);
        temp >>= 8;
      }
      pOut[m * cnt] = c;
      m++;
    }
    pOut++;
    p++;
  }
  return ret;
}

char* AlpAdapter::UseBin2pixel(char* pSrc, int width, int height, unsigned char bitmask, char* pOut)
{
  if (pOut == NULL || pSrc == NULL || width % 8 != 0)
    return NULL;
  char* ret = pOut;
  int cnt = (width / 8)*height;  //once operate 8bytes.
  unsigned char bm;
  int i, j, k, m;
  UINT64 c;
  UINT64 temp;
  UINT64* p = (UINT64*)pOut;
  for (i = 0; i < cnt; i++)
  {
    temp = 0;
    for (j = 0, m = 0, bm = bitmask; j < 8 && (bm & 0x80); j++, bm <<= 1)  //j means which bit
    {
      c = (UINT64) *(pSrc + cnt*m);
      m++;
      for (k = 0; k < 8; k++) //which pixel
      {
        temp |= (c & 0x01) << (k * 8 + 7 - j);
        c >>= 1;
      }
    }
    pSrc++;
    *p++ = temp;
  }
  return ret;
}

void AlpAdapter::Binary2Gray(int width, int height, unsigned char* src, unsigned char* dst)
{
  int cw = width / 8;
  unsigned char* s, *d;
  for (int i = 0; i < height; i++)
  {
    d = dst + i * width;
    s = src + i * cw;
    for (int j = 0; j < cw; j++)
    {
      for (int k = 0; k < 8; k++)
      {
        d[j * 8 + k] = (s[j] & (1 << k)) ? 0xff : 0;
      }
    }
  }
}


