
#include "StdAfx.h"
#include "TUCamVideo.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TUCamVideo::TUCamVideo(int idx, CString strName)
{
  InitializeCriticalSection(&m_crtSec);
  m_VideoRender = 0;
  m_AWB = false;
  bOpen = false;
  bRun = false;
  bMirror = false;
  bFlip = false;

  m_nTriMode = TUCCM_SEQUENCE;

  m_TUCameraParas.m_handle = NULL;
  m_TUCameraParas.m_CameraName = strName;
  m_TUCameraParas.iSel = idx;
  m_TUCameraParas.m_pBuffer = 0;
  m_TUCameraParas.m_pDcamBuffer = 0;
  m_TUCameraParas.m_pWidth = 0;
  m_TUCameraParas.m_pHeight = 0;
  m_TUCameraParas.m_fExposure = 10.0;
  m_TUCameraParas.m_Gain = 1;
  width = 0;
  height = 0;

  mFlipMirrorParas.iCount = 0;
  mFlipMirrorParas.iFlipDst = NULL;
  mFlipMirrorParas.iFlipSrc = NULL;
  mFlipMirrorParas.iMirrorSrc = NULL;
  mFlipMirrorParas.iMirrorDst = NULL;

  for (int i = 0; i < 3; i++)
    mBinPt[i].x = mBinPt[i].y = 0;

  m_bWaitting = FALSE;

  m_hThdWaitForFrame = 0;
}

TUCamVideo::~TUCamVideo(void)
{
  CloseCamera();
  m_VideoRender = 0;
}

CString TUCamVideo::GetBining(int pIndex)
{
  CString temp;
  temp.Format(_T("%d X %d"), m_TUCameraParas.m_pWidth[pIndex],
    m_TUCameraParas.m_pHeight[pIndex]);
  return temp;
}

void TUCamVideo::SetBinningPt(int pIndex, CPoint Pt)
{
  mBinPt[pIndex] = Pt;
}

void TUCamVideo::SetBinning(int pIndex, CPoint Pt)
{
  if (!m_TUCameraParas.m_handle)
  {
    return;
  }
  if (pIndex == m_TUCameraParas.m_iBinningIndx)
    return;
  int iX = 0, iY = 0;
  int iW = m_TUCameraParas.m_pWidth[pIndex];
  int iH = m_TUCameraParas.m_pHeight[pIndex];
  if (pIndex > 0)
  {
    if (mBinPt[pIndex].x > 0 && mBinPt[pIndex].y > 0)
    {
      iX = mBinPt[pIndex].x; iY = mBinPt[pIndex].y;
    }
    else
    {
      if (pIndex == 1)
      {
        iX = 512; iY = 512;
      }
      else
      {
        iX = 768; iY = 768;
      }
    }
  }
  EnterCriticalSection(&m_crtSec);
  m_TUCameraParas.m_iBinningIndx = pIndex;
  StopWaitForFrame();
  TUCAM_ROI_ATTR roi_attr;
  roi_attr.bEnable = pIndex != 0;
  roi_attr.nHOffset = iX;
  roi_attr.nVOffset = iY;
  roi_attr.nWidth = iW;
  roi_attr.nHeight = iH;
  TUCAM_Cap_SetROI(m_TUCameraParas.m_handle, roi_attr);
  UpdateBufferSize(m_TUCameraParas.m_pWidth[m_TUCameraParas.m_iBinningIndx],
    m_TUCameraParas.m_pHeight[m_TUCameraParas.m_iBinningIndx]);
  StartWaitForFrame();
  LeaveCriticalSection(&m_crtSec);
}

bool TUCamVideo::OpenCamera()
{
  if (bOpen)
    return true;
  TUCAM_OPEN tucam_open;
  tucam_open.uiIdxOpen = m_TUCameraParas.iSel;
  tucam_open.hIdxTUCam = m_TUCameraParas.m_handle;
  if (TUCAM_Dev_Open(&tucam_open) != TUCAMRET_SUCCESS)return false;

  m_TUCameraParas.m_handle = tucam_open.hIdxTUCam;

  //step1 设分辨率
  int binCount = 3;
  if (binCount != m_TUCameraParas.m_iBinningCount ||
    m_TUCameraParas.m_pWidth == NULL || m_TUCameraParas.m_pHeight == NULL)
  {
    m_TUCameraParas.m_iBinningCount = binCount;//设总数
    if (m_TUCameraParas.m_pWidth)delete[] m_TUCameraParas.m_pWidth;
    if (m_TUCameraParas.m_pHeight)delete[] m_TUCameraParas.m_pHeight;
    m_TUCameraParas.m_pWidth = new int[m_TUCameraParas.m_iBinningCount];
    m_TUCameraParas.m_pHeight = new int[m_TUCameraParas.m_iBinningCount];
  }	

  m_TUCameraParas.m_pWidth[0] = 2048;
  m_TUCameraParas.m_pHeight[0] = 2040;

  m_TUCameraParas.m_pWidth[1] = 1024;
  m_TUCameraParas.m_pHeight[1] = 1024;

  m_TUCameraParas.m_pWidth[2] = 512;
  m_TUCameraParas.m_pHeight[2] = 512;

  m_TUCameraParas.m_iBinningIndx = 0;//记入参数
  //Step2 设置数据格式
  m_TUCameraParas.m_iBitDepth = 8;
  m_TUCameraParas.m_iColorChannel = 1;//RGB 3 channels

  EnterCriticalSection(&m_crtSec);
  UpdateBufferSize(m_TUCameraParas.m_pWidth[m_TUCameraParas.m_iBinningIndx],
    m_TUCameraParas.m_pHeight[m_TUCameraParas.m_iBinningIndx]);
  LeaveCriticalSection(&m_crtSec);

  //step3 打开相机
  Notify(this, Notify_CarlVideo_OpenCamera, 0, &m_TUCameraParas.m_CameraName);
  bOpen = true;
  return true;
}

bool TUCamVideo::Run()
{
  if (!bOpen)
    return false;

  bRun = true;
  StartWaitForFrame();
  return true;
}

bool TUCamVideo::Pause()
{
  if (!bOpen)
    return true;

  bRun = false;

  StopWaitForFrame();
  return true;
}

bool TUCamVideo::CloseCamera()
{
  bOpen = false;
  if (!m_TUCameraParas.m_handle)return false;

  StopWaitForFrame();

  bRun = false;
  TUCAM_Dev_Close(m_TUCameraParas.m_handle);
  m_TUCameraParas.m_handle = NULL;

  ClearBuffer();

  if (m_TUCameraParas.m_pWidth)
  {
    delete[] m_TUCameraParas.m_pWidth;
    m_TUCameraParas.m_pWidth = 0;
  }
  if (m_TUCameraParas.m_pHeight)
  {
    delete[] m_TUCameraParas.m_pHeight;
    m_TUCameraParas.m_pHeight = 0;
  }

  Notify(this, Notify_CarlVideo_CloseCamera, 0, &m_TUCameraParas.m_CameraName);
  return true;
}

bool TUCamVideo::GetEnable(HVideoPID pPID)
{
  switch (pPID)
  {
  case VIDEO_PID_WhitBalance:
    return false;
    break;
  case VIDEO_PID_AutoExp:
    return false;
    break;
  default:
    return false;
    break;
  }
  return false;
}

void TUCamVideo::SetValue(HVideoPID pPID, float pValue)
{
  EnterCriticalSection(&m_crtSec);
  if (!m_TUCameraParas.m_handle)
    return;
  switch (pPID)
  {
  case VIDEO_PID_RGAIN:

    break;
  case VIDEO_PID_GGAIN:

    break;
  case VIDEO_PID_BGAIN:

    break;
  case VIDEO_PID_GAIN:

    break;
  case VIDEO_PID_TRIGER:
    break;
  }
  LeaveCriticalSection(&m_crtSec);
}

float TUCamVideo::GetValue(HVideoPID pPID)
{
  EnterCriticalSection(&m_crtSec);
  switch (pPID)
  {
  case VIDEO_PID_RGAIN:

    break;
  case VIDEO_PID_GGAIN:

    break;
  case VIDEO_PID_BGAIN:

    break;
  case VIDEO_PID_GAIN:

    break;

  }
  LeaveCriticalSection(&m_crtSec);
  return 0;
}

void TUCamVideo::SetExposure(float pExpos)
{
  if (!m_TUCameraParas.m_handle)
    return;
  m_TUCameraParas.m_fExposure = pExpos;
  EnterCriticalSection(&m_crtSec);
  TUCAM_Prop_SetValue(m_TUCameraParas.m_handle, TUIDP_EXPOSURETM, pExpos);
  LeaveCriticalSection(&m_crtSec);
  Notify(this, Notify_CarlVideo_ExpTime, 0, 0, pExpos);
}

void TUCamVideo::AutoAWB(bool pAuto)
{
  m_AWB = pAuto;
}

HVideoHeader TUCamVideo::GetImgHeader()
{
  HVideoHeader t_result;
  t_result.Vheight = m_TUCameraParas.m_pHeight[m_TUCameraParas.m_iBinningIndx];
  t_result.Vwidth = m_TUCameraParas.m_pWidth[m_TUCameraParas.m_iBinningIndx];
  t_result.VwBit = m_TUCameraParas.m_iColorChannel;
  t_result.Vuser = (long)m_TUCameraParas.m_handle;//标识相机
  t_result.Vsize = t_result.Vwidth*t_result.Vheight*t_result.VwBit;
  t_result.VwBit *= 8;
  return t_result;
}

bool TUCamVideo::GrabImage(HVideoHeader* pHeader, LPBYTE pBuffer)
{
  //pHeader->Vheight = m_TUCameraParas.m_pHeight[m_TUCameraParas.m_iBinningIndx];
  //pHeader->Vwidth = m_TUCameraParas.m_pWidth[m_TUCameraParas.m_iBinningIndx];
  //pHeader->VwBit = m_TUCameraParas.m_iColorChannel * 8;
  //pHeader->Vuser = (long)m_TUCameraParas.m_handle;//标识相机
  //if (GrabBuffer(width, height))
  //{
  //  pHeader->Vsize = width*height*pHeader->VwBit / 8;
  //  memcpy(pBuffer, m_TUCameraParas.m_pBuffer, pHeader->Vsize);
  //  return true;
  //}
  //else
  //{
  //  pHeader->Vsize = width*height*pHeader->VwBit / 8;
  //  memcpy(pBuffer, m_TUCameraParas.m_pBuffer, pHeader->Vsize);
  //  return false;
  //}
  return false;
}

void TUCamVideo::SetRender(HVideoRender* pRender)
{
  EnterCriticalSection(&m_crtSec);
  m_VideoRender = pRender;
  LeaveCriticalSection(&m_crtSec);
}

void TUCamVideo::SetTrigerMode(TrigerType pMode)
{
  EnterCriticalSection(&m_crtSec);
  StopWaitForFrame();
  switch (pMode)
  {
  case TRIGER_INTERAL:
  {
    TUCAM_TRIGGER_ATTR tgrAttr;
    tgrAttr.nTgrMode = TUCCM_SEQUENCE;    // 顺序模式
    tgrAttr.nExpMode = TUCTE_EXPTM;
    tgrAttr.nEdgeMode = TUCTD_RISING;
    tgrAttr.nFrames = 0;
    tgrAttr.nDelayTm = 0;
    TUCAM_Cap_SetTrigger(m_TUCameraParas.m_handle, tgrAttr);

    m_nTriMode = TUCCM_SEQUENCE;
  }
  break;
  case TRIGER_SOFT:
  {
    TUCAM_TRIGGER_ATTR tgrAttr;
    tgrAttr.nTgrMode = TUCCM_TRIGGER_SOFTWARE;    // 软件触发模式
    tgrAttr.nExpMode = TUCTE_EXPTM;
    tgrAttr.nEdgeMode = TUCTD_RISING;
    tgrAttr.nFrames = 1;                           // 触发1帧
    tgrAttr.nDelayTm = 0;                           // 延时0 ms
    TUCAM_Cap_SetTrigger(m_TUCameraParas.m_handle, tgrAttr);

    m_nTriMode = TUCCM_TRIGGER_SOFTWARE;
  }
  break;
  case TRIGER_OUT:
  {
    TUCAM_TRIGGER_ATTR tgrAttr;
    tgrAttr.nTgrMode = TUCCM_TRIGGER_STANDARD;    // 标准触发模式
    tgrAttr.nExpMode = TUCTE_EXPTM;                // 曝光模式
    tgrAttr.nEdgeMode = TUCTD_RISING;               // 激发上升沿
    tgrAttr.nFrames = 1;                           // 触发1帧
    tgrAttr.nDelayTm = 0;                           // 延时0 ms
    TUCAM_Cap_SetTrigger(m_TUCameraParas.m_handle, tgrAttr);

    m_nTriMode = TUCCM_TRIGGER_STANDARD;
  }
  break;
  }
  
  StartWaitForFrame();
  LeaveCriticalSection(&m_crtSec);

  Notify(this, Notify_CarlVideo_Triger, 0, &pMode);
}

void TUCamVideo::TrigerVideoData()
{
  //if (m_TUCameraParas.m_handle)
  //{
  //  if (GrabBuffer(width, height))//抓图成功
  //  {
  //    DWORD t_now = ::GetTickCount();
  //    DWORD dutime = t_now - (DWORD)m_VideoFormate.VcaptureTime;
  //    m_VideoFormate.VcaptureTime = t_now;
  //    m_VideoFormate.VFps = (float)(1000.0 / dutime);
  //    RenderVideoData();//渲染
  //  }
  //}

  if (m_nTriMode == TUCCM_SEQUENCE)
  {
    // 自由模式直接返回
    return;
  }
  else if (m_nTriMode == TUCCM_TRIGGER_STANDARD)
  {
    // 硬件触发模式直接返回
    return;
  }
  else if(m_nTriMode == TUCCM_TRIGGER_SOFTWARE)
  {
    // 软件触发
    if (NULL != m_TUCameraParas.m_handle)
    {
      TUCAM_Cap_DoSoftwareTrigger(m_TUCameraParas.m_handle);
    }
  }
}

void TUCamVideo::UpdateBufferSize(int width, int height)
{
  if (m_TUCameraParas.m_pBuffer)
    delete [](m_TUCameraParas.m_pBuffer);
  if (m_TUCameraParas.m_pDcamBuffer)
    delete [](m_TUCameraParas.m_pDcamBuffer);
  m_TUCameraParas.m_pBuffer = (LPBYTE)new unsigned char[width*height];
  m_TUCameraParas.m_pDcamBuffer = (unsigned short*)new unsigned short[width*height];

  int m_PixelBytes = 1;
  if (mFlipMirrorParas.iMirrorDst)
    delete []mFlipMirrorParas.iMirrorDst;
  if (mFlipMirrorParas.iMirrorSrc)
    delete []mFlipMirrorParas.iMirrorSrc;
  if (mFlipMirrorParas.iFlipDst)
    delete []mFlipMirrorParas.iFlipDst;
  if (mFlipMirrorParas.iFlipSrc)
    delete []mFlipMirrorParas.iFlipSrc;
  mFlipMirrorParas.iCount = height*width*m_PixelBytes;
  mFlipMirrorParas.iMirrorDst = new int[mFlipMirrorParas.iCount];
  mFlipMirrorParas.iMirrorSrc = new int[mFlipMirrorParas.iCount];
  mFlipMirrorParas.iFlipSrc = new int[mFlipMirrorParas.iCount];
  mFlipMirrorParas.iFlipDst = new int[mFlipMirrorParas.iCount];
  int iCount = 0;
  for (int j = height - 1; j >= 0; j--)
  {
    for (int i = 0; i < width; i++)
    {
      for (int k = 0; k < m_PixelBytes; k++)
      {
        *(mFlipMirrorParas.iMirrorSrc + iCount) = (width*j + i)*m_PixelBytes + k;
        *(mFlipMirrorParas.iMirrorDst + iCount) = (width*(j + 1) - i - 1)*m_PixelBytes + k;
        *(mFlipMirrorParas.iFlipSrc + iCount) = (width*j + i)*m_PixelBytes + k,
          *(mFlipMirrorParas.iFlipDst + iCount) = (width*(height - j - 1) + i)*m_PixelBytes + k;
        iCount++;
      }
    }
  }
}

bool TUCamVideo::GrabBuffer(int& pwidth, int& pheight)
{
  EnterCriticalSection(&m_crtSec);
  if (pwidth != m_TUCameraParas.m_pWidth[m_TUCameraParas.m_iBinningIndx] &&
    pheight != m_TUCameraParas.m_pHeight[m_TUCameraParas.m_iBinningIndx])
  {
    pwidth = m_TUCameraParas.m_pWidth[m_TUCameraParas.m_iBinningIndx];
    pheight = m_TUCameraParas.m_pHeight[m_TUCameraParas.m_iBinningIndx];
    UpdateBufferSize(pwidth, pheight);
    m_TUCameraParas.m_DcamSize = pwidth*pheight * 2;//固定16位大小
  }

  m_frame.pBuffer = NULL;

  LeaveCriticalSection(&m_crtSec);
  return true;
}

void TUCamVideo::RenderVideoData()
{
  m_VideoFormate.Vheight = height;
  m_VideoFormate.Vwidth = width;
  long t_Size = height*width;//16转8位
  m_VideoFormate.VwBit = m_TUCameraParas.m_iColorChannel * 8;
  m_VideoFormate.Vsize = t_Size;
  m_VideoFormate.Vuser = 1;// (long)m_TUCameraParas.m_handle;//标识相机

  //memcpy(m_TUCameraParas.m_pBuffer,m_TUCameraParas.m_pDcamBuffer,t_Size);
  //下面转8位数据;
  unsigned char *p_scr = (unsigned char*)(m_TUCameraParas.m_pDcamBuffer) + 1;
  unsigned char *p_Dest = m_TUCameraParas.m_pBuffer;
  for (long i = 0; i < t_Size; i++)
  {
    *p_Dest = *p_scr;
    p_Dest++;
    p_scr += 2;
  }//这个要用5ms
  LPBYTE middleBuffer = NULL;
  if (bMirror || bFlip)
    middleBuffer = (LPBYTE)new unsigned char[m_VideoFormate.Vsize];

  if (bMirror&&bFlip)
  {
    MirrorConvert(m_TUCameraParas.m_pBuffer, middleBuffer, m_VideoFormate);
    FlipConvert(middleBuffer, m_TUCameraParas.m_pBuffer, m_VideoFormate);
    delete []middleBuffer;
  }
  else if (bMirror)
  {
    memcpy(middleBuffer, m_TUCameraParas.m_pBuffer, m_VideoFormate.Vsize);
    MirrorConvert(middleBuffer, m_TUCameraParas.m_pBuffer, m_VideoFormate);
    delete []middleBuffer;
  }
  else if (bFlip)
  {
    memcpy(middleBuffer, m_TUCameraParas.m_pBuffer, m_VideoFormate.Vsize);
    FlipConvert(middleBuffer, m_TUCameraParas.m_pBuffer, m_VideoFormate);
    delete []middleBuffer;
  }
  if (m_VideoRender)
    m_VideoRender->Renderer(&m_VideoFormate, m_TUCameraParas.m_pBuffer);
}

bool TUCamVideo::SetFlip(bool pFlip)
{
  if (!bOpen)
    return false;
  EnterCriticalSection(&m_crtSec);
  bFlip = pFlip;
  LeaveCriticalSection(&m_crtSec);
  Notify(this, Notify_CarlVideo_Flip, 0, &bFlip);
  return true;
}

bool TUCamVideo::SetMirror(bool pMirror)
{
  if (!bOpen)
    return false;
  EnterCriticalSection(&m_crtSec);
  bMirror = pMirror;
  LeaveCriticalSection(&m_crtSec);
  Notify(this, Notify_CarlVideo_Mirror, 0, &pMirror);
  return true;
}

void TUCamVideo::MirrorConvert(LPBYTE Src, LPBYTE Dst, HVideoHeader mPara)
{
  if (mFlipMirrorParas.iCount > 0)
  {
    for (int i = 0; i < mFlipMirrorParas.iCount; i++)
      Dst[mFlipMirrorParas.iMirrorDst[i]] = Src[mFlipMirrorParas.iMirrorSrc[i]];
    return;
  }
  int with = mPara.Vwidth;
  int height = mPara.Vheight;
  int m_PixelBytes = mPara.VwBit / 8;
  for (int j = height - 1; j >= 0; j--)
  {
    for (int i = 0; i < with; i++)
    {
      for (int k = 0; k < m_PixelBytes; k++)
      {
        int indexSrc = (with*j + i)*m_PixelBytes + k,
          indexDst = (with*(j + 1) - i - 1)*m_PixelBytes + k;
        Dst[indexDst] = Src[indexSrc];
      }
    }
  }
}

void TUCamVideo::FlipConvert(LPBYTE Src, LPBYTE Dst, HVideoHeader mPara)
{
  if (mFlipMirrorParas.iCount > 0)
  {
    for (int i = 0; i < mFlipMirrorParas.iCount; i++)
      Dst[mFlipMirrorParas.iFlipDst[i]] = Src[mFlipMirrorParas.iFlipSrc[i]];
    return;
  }
  int with = mPara.Vwidth;
  int height = mPara.Vheight;
  int m_PixelBytes = mPara.VwBit / 8;
  for (int j = height - 1; j >= 0; j--)
  {
    for (int i = 0; i < with; i++)
    {
      for (int k = 0; k < m_PixelBytes; k++)
      {
        int indexSrc = (with*j + i)*m_PixelBytes + k,
          indexDst = (with*(height - j - 1) + i)*m_PixelBytes + k;
        Dst[indexDst] = Src[indexSrc];
      }
    }
  }
}

void TUCamVideo::StartWaitForFrame()
{
  if (m_bWaitting)
  {
    return;
  }

  m_bWaitting = TRUE;

  if (NULL == m_hThdWaitForFrame)
  {
    m_frame.pBuffer = NULL;

    if (m_nTriMode == TUCCM_SEQUENCE)
      m_frame.ucFormatGet = TUFRM_FMT_USUAl;
    else
      m_frame.ucFormatGet = TUFRM_FMT_RAW;

    m_frame.uiRsdSize = 1;                            // how many frames do you want

    TUCAM_Buf_Alloc(m_TUCameraParas.m_handle, &m_frame);       // Alloc buffer after set resolution or set ROI attribute
    TUCAM_Cap_Start(m_TUCameraParas.m_handle, m_nTriMode);     // Start capture

    // 如果是软件触发模式，开始时先触发一次
    if (m_nTriMode == TUCCM_TRIGGER_SOFTWARE)
    {
      TUCAM_Cap_DoSoftwareTrigger(m_TUCameraParas.m_handle);
    }

    m_hThdWaitForFrame = CreateEvent(NULL, TRUE, FALSE, NULL);
    _beginthread(WaitForFrameThread, 0, this);          // Start capturing thread
    Notify(this, Notify_CarlVideo_RunCamera, 0, &m_TUCameraParas.m_CameraName);
  }
}

void TUCamVideo::StopWaitForFrame()
{
  if (!m_bWaitting)
  {
    return;
  }

  m_bWaitting = FALSE;

  if (NULL != m_hThdWaitForFrame)
  {
    TUCAM_Buf_AbortWait(m_TUCameraParas.m_handle);             // If you called TUCAM_Buf_WaitForFrames()

    WaitForSingleObject(m_hThdWaitForFrame, INFINITE);
    CloseHandle(m_hThdWaitForFrame);
    m_hThdWaitForFrame = NULL;

    TUCAM_Cap_Stop(m_TUCameraParas.m_handle);                  // Stop capture   
    TUCAM_Buf_Release(m_TUCameraParas.m_handle);               // Release alloc buffer after stop capture and quit drawing thread
  }
  if (bOpen)
    Notify(this, Notify_CarlVideo_PauseCamera, 0, &m_TUCameraParas.m_CameraName);
}

void __cdecl TUCamVideo::WaitForFrameThread(LPVOID lParam)
{
  TUCamVideo *pIn = (TUCamVideo *)lParam;

  while (pIn->m_bWaitting)
  {
    if (pIn->m_nTriMode == TUCCM_SEQUENCE)
    {
      pIn->m_frame.ucFormatGet = TUFRM_FMT_USUAl;
    }
    else
    {
      pIn->m_frame.ucFormatGet = TUFRM_FMT_RAW;
    }

    if (TUCAMRET_SUCCESS == TUCAM_Buf_WaitForFrame(pIn->m_TUCameraParas.m_handle, &pIn->m_frame))
    {
      if (NULL != pIn->m_frame.pBuffer)
      {
        PUCHAR pBuf = pIn->m_frame.pBuffer + pIn->m_frame.usHeader;

        DWORD t_now = ::GetTickCount();
        DWORD dutime = t_now - (DWORD)pIn->m_VideoFormate.VcaptureTime;
        pIn->m_VideoFormate.VcaptureTime = t_now;
        pIn->m_VideoFormate.VFps = (float)(1000.0 / dutime);

        pIn->width = pIn->m_frame.usWidth;
        pIn->height = pIn->m_frame.usHeight;
        memcpy(pIn->m_TUCameraParas.m_pDcamBuffer, pBuf, pIn->m_frame.usWidth * pIn->m_frame.usHeight * 2);

        pIn->RenderVideoData();
      }
    }
  }
  SetEvent(pIn->m_hThdWaitForFrame);

  _endthread();
}

void TUCamVideo::ClearBuffer()
{
  if (m_TUCameraParas.m_pBuffer)
  {
    delete[](m_TUCameraParas.m_pBuffer);
    m_TUCameraParas.m_pBuffer = 0;
  }
  if (m_TUCameraParas.m_pDcamBuffer)
  {
    delete[](m_TUCameraParas.m_pDcamBuffer);
    m_TUCameraParas.m_pDcamBuffer = 0;
  }
  if (mFlipMirrorParas.iMirrorDst)
  {
    delete[]mFlipMirrorParas.iMirrorDst;
    mFlipMirrorParas.iMirrorDst = 0;
  }
  if (mFlipMirrorParas.iMirrorSrc)
  {
    delete[]mFlipMirrorParas.iMirrorSrc;
    mFlipMirrorParas.iMirrorSrc = 0;
  }
  if (mFlipMirrorParas.iFlipDst)
  {
    delete[]mFlipMirrorParas.iFlipDst;
    mFlipMirrorParas.iFlipDst = 0;
  }
  if (mFlipMirrorParas.iFlipSrc)
  {
    delete[]mFlipMirrorParas.iFlipSrc;
    mFlipMirrorParas.iFlipSrc = 0;
  }
}