#include "StdAfx.h"
#include "DMDManager.h"
#include "PattenPoint.h"
#include "PatternLine.h"

CDMDManager::CDMDManager(void)
{
  mWlpAdapter = 0;
	mWlpDMDParas=0;
	m_FrameCount=0;
  m_type = -1;
}

CDMDManager::~CDMDManager(void)
{
  if (mWlpAdapter)
  {
    delete mWlpAdapter;
    mWlpAdapter = 0;
  }
}

bool CDMDManager::InitDMD(void* wPara, int type)
{
	 mWlpDMDParas=(WlpDMDParas*)wPara;

   if (mWlpAdapter)
   {
     delete mWlpAdapter;
   }
   if (type == 0)
   {
     mWlpAdapter = new CWlpAdapter();
   }
   else
   {
     mWlpAdapter = new AlpAdapter();
   }
   m_type = type;

	 if (mWlpDMDParas)
	 {
		 if (mWlpAdapter->OpenDev()) // ≤‚ ‘ || 1
		 {
			 if (!mWlpAdapter->InitForDownload(mWlpDMDParas->m_FrameMargin,mWlpDMDParas->m_FrameInterval,
				 mWlpDMDParas->gray,mWlpDMDParas->trigeMode,mWlpDMDParas->trigeSync))
				 return false;
			 Notify(this,Notify_WLPDMD_OPEN,0,0);
			 mInfoDMD.RAMsizeGB=mWlpAdapter->RAMsizeGB;
			 mInfoDMD.RAMsizeMB=mWlpAdapter->RAMsizeMB;
			 mInfoDMD.m_BitFrameSize=mWlpAdapter->m_BitFrameSize;
			 mInfoDMD.strInfo.Format(L" DMD RAM Size: %.3fGB = %dMB \r\n BitFrameSize is 0D%d Bytes = 0X%X Bytes = %5.2f KB = %5.2f MB",
				 mInfoDMD.RAMsizeGB,mInfoDMD.RAMsizeMB,mInfoDMD.m_BitFrameSize, mInfoDMD.m_BitFrameSize  , 
				 (float)((float)mInfoDMD.m_BitFrameSize /(ULONG)1024) , (float)((float)mInfoDMD.m_BitFrameSize /(ULONG)1048576));
			 return true;
		 }		 
	 }	
	 return false;
}

bool CDMDManager::RestPara(void* wPara)
{
	 mWlpDMDParas=(WlpDMDParas*)wPara;
	 if (mWlpAdapter->InitForDownload(mWlpDMDParas->m_FrameMargin,mWlpDMDParas->m_FrameInterval,
		 mWlpDMDParas->gray,mWlpDMDParas->trigeMode,mWlpDMDParas->trigeSync))
		 return true;
	 return false;
}

bool CDMDManager::UnInitDMD()
{
  if (mWlpAdapter)
  {
    mWlpAdapter->CloseDev();
    delete mWlpAdapter;
    mWlpAdapter = 0;
  }
  mInfoDMD.strInfo = _T("");
  mInfoDMD.RAMsizeGB = 0;
  mInfoDMD.RAMsizeMB = 0;
  mInfoDMD.m_BitFrameSize = 0;

  m_type = -1;
	return true;
}

float CDMDManager::GetFrameRate()
{
	float freq = (float)1000000/(float)(mWlpDMDParas->m_FrameMargin+mWlpDMDParas->m_FrameInterval);
	return freq;
}

void CDMDManager::GetDownSize(int* iWidth, int* iHeight)
{
  if (mWlpAdapter)
  {
    mWlpAdapter->GetPicSize(iWidth, iHeight);
  }
}

bool CDMDManager::DownLoadFrame(unsigned char* p_data,long frams)
{
	m_FrameCount=frams;
	//œ»≈‰÷√
	if (!mWlpAdapter->InitForDownload(mWlpDMDParas->m_FrameMargin,mWlpDMDParas->m_FrameInterval,
			mWlpDMDParas->gray,mWlpDMDParas->trigeMode,mWlpDMDParas->trigeSync))
		return false;

	if (!mWlpAdapter->TriggerExtSet(mWlpDMDParas->delay,mWlpDMDParas->polay,mWlpDMDParas->plusewidth,frams))
		return false;
	//œ¬‘ÿ
	if (!mWlpAdapter->DownloadPic(p_data,frams))
		return false;
	return true;
}

IPatternGenerate* CDMDManager::CreateDmdPattern(int Mode)
{
	switch (Mode)
	{
	case PATTERN_LINE:
		return new PatternLine();
		break;
	case PATTERN_POINT:
		return new PattenPoint();
		break;
	}
  return 0;
}

void CDMDManager::DestroyDmdPattern(IPatternGenerate* pg)
{
  delete pg;
}

bool CDMDManager::DownLoadPattern(IPatternGenerate* p_Pattern)
{
	int iCount=p_Pattern->GetFrameCount();
	m_FrameCount=iCount;
	int iW=0,iH=0;
	mWlpAdapter->GetPicSize(&iW,&iH);
	int m_BitFrameSize=iW*iH;
	if (m_BitFrameSize==0)
		return false;
	m_BitFrameSize=m_BitFrameSize*mWlpDMDParas->gray/8;
	__int64 LargeImageBuffSize = (__int64)m_BitFrameSize * iCount;
	unsigned char* pLargeImageBuff = (unsigned char*)malloc((size_t)(sizeof(BYTE)*LargeImageBuffSize));
  if (!pLargeImageBuff)
  {
    return false; // ƒ⁄¥Ê∑÷≈‰ ß∞‹
  }
	for(int i=0;i<iCount;i++)
		memcpy(pLargeImageBuff+i*m_BitFrameSize,p_Pattern->GetData(i,mWlpDMDParas->gray),m_BitFrameSize);

	//œ»≈‰÷√
  if (!mWlpAdapter->InitForDownload(mWlpDMDParas->m_FrameMargin, mWlpDMDParas->m_FrameInterval,
    mWlpDMDParas->gray, mWlpDMDParas->trigeMode, mWlpDMDParas->trigeSync))
  {
    free(pLargeImageBuff);
    return false;
  }
  if (!mWlpAdapter->TriggerExtSet(mWlpDMDParas->delay, mWlpDMDParas->polay, mWlpDMDParas->plusewidth, iCount))
  {
    free(pLargeImageBuff);
    return false;
  }
	//œ¬‘ÿ
  if (!mWlpAdapter->DownloadPic(pLargeImageBuff, iCount))
  {
    free(pLargeImageBuff);
    return false;
  }
	/*CString strS;
	strS.Format(L"DownLoad %d Pictures",iCount);
	AfxMessageBox(strS);*/
	free(pLargeImageBuff);
	return true;
}

void CDMDManager::SetStatus(int status)
{
	switch(status)
	{
		case DMD_START:
			if (!mWlpAdapter->InitForDownload(mWlpDMDParas->m_FrameMargin,mWlpDMDParas->m_FrameInterval,
				mWlpDMDParas->gray,mWlpDMDParas->trigeMode,mWlpDMDParas->trigeSync))
				return;
			mWlpAdapter->Start();
			Notify(this,Notify_WLPDMD_START,0,0);
			break;
		case DMD_STOP:
			mWlpAdapter->Stop();
			Notify(this,Notify_WLPDMD_STOP,0,0);
			break;
		case DMD_Trigger:
			mWlpAdapter->Trigger();
			break;
	}
}

char* CDMDManager::pixel2bin(char* pSrc, int width,int height, unsigned char bitmask, char* pOut)
{
	return mWlpAdapter->UsePixel2bin(pSrc,width,height,bitmask,pOut);
}

char* CDMDManager::bin2pixel(char* pSrc, int width,int height, unsigned char bitmask, char* pOut)
{
	return mWlpAdapter->UseBin2pixel(pSrc,width,height,bitmask,pOut);
}

int CDMDManager::GetType()
{
  return m_type;
}