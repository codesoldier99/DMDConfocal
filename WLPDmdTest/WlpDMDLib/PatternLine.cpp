#include "StdAfx.h"
#include "PatternLine.h"

PatternLine::PatternLine(void)
{
	bS=false;
	strSavePath=L"";
	mParas=0;
	m_pData=0;
  m_pData2 = 0;
}

PatternLine::~PatternLine(void)
{
	if (m_pData) free(m_pData);
  if (m_pData2) free(m_pData2);
}

void PatternLine::Gernerate(void* Paras)
{
	mParas=(LinePara*)Paras;
	if (m_pData) free(m_pData);	
  if (m_pData2) free(m_pData2);
	m_pData=(unsigned char*)malloc(Bit2Byte(m_width)*m_height);
  m_pData2 = (unsigned char*)malloc(m_width*m_height);
}

unsigned char* PatternLine::GetData(int index,int iGray)
{
	if (!mParas) return 0;	
	if (!m_pData) return 0;	
  if (!m_pData2)return 0;
	int StartLine=index*mParas->iLineW;
	int StopLine=(index+1)*mParas->iLineW+mParas->iLExW;////////在这边可以加上一个数，指定为拓宽的程度！！！
	int AllWidth=mParas->iLineW*mParas->iLPeriod;//代表该图轮询一行的高度循环值
	int CountLine=AllWidth;
	if (mParas->iLType==PATTERN_LINE_HORIZ)//水平扫
	{
		if (StartLine>m_height) StartLine=m_height;
		if(StopLine>m_height) StopLine=m_height;
		unsigned char* pBuffer=m_pData;//图像指针
		for (int i=0;i<m_height;i++)//针对的大图循环
		{
			if(i>=CountLine&&i>=StopLine)//加上一个StopLine，因为StopLine有会超过CountLine
			{
				CountLine+=AllWidth;
				StartLine+=AllWidth;
				StopLine+=AllWidth;
			}
			if(i>=StartLine&&i<StopLine)
				memset(pBuffer,0xFF,Bit2Byte(m_width));//复制方式，是针对小图[单bit图]，一行一行复制
			else
				memset(pBuffer,0x00, Bit2Byte(m_width));
			pBuffer+= Bit2Byte(m_width);
		}
	}
	else//竖扫
	{		
		if (StartLine>m_width) StartLine=m_width;
		if(StopLine>m_width) StopLine=m_width;
		unsigned char* pBufferScr=(unsigned char*)malloc(m_width);//源位置
		unsigned char* pBufferDest=m_pData;//目标位置
		memset(m_pData,0, Bit2Byte(m_width)*m_height);
		///////////////先生成大图的一行图/////////////////////////
		for (int i=0;i<m_width;i++)
		{
			if(i>=CountLine&&i>=StopLine)
			{
				CountLine+=AllWidth;
				StartLine+=AllWidth;
				StopLine+=AllWidth;
			}
			if (i>=StartLine&&i<StopLine)
				pBufferScr[i]=0xFF;
			else
				pBufferScr[i]=0;
		}
		///////////////针对大图来生成小图一行/////////////////////////
		int counts=0;//定位
		for(int j=0;j<Bit2Byte(m_width) / 8;j++)
			for(int i=0;i<8;i++)
			{
				for(int k=0;k<8;k++)
				{					
					pBufferDest[8*j + i]<<=1;
					if(pBufferScr[counts++])
						pBufferDest[8*j + i]++;		
				}
			}		
		free(pBufferScr);
		pBufferScr=m_pData;
		///////////////将小图的每行进行复制，完成小图构建/////////////////////////
		for (int i=0;i<m_height;i++)
		{
			memcpy(pBufferDest,pBufferScr, Bit2Byte(m_width));
			pBufferDest+= Bit2Byte(m_width);
		}
	}

  if (iGray == 8)
  {
    Binary2Gray(m_width, m_height, m_pData, m_pData2);
  }

	if (bS)
	{
		CString name;
		name.Format(L"%s\\BufferLine(%dX%d-%d)",strSavePath,mParas->iLineW,mParas->iLPeriod,mParas->iLType);
		BuildDirectory(name);
		name.Format(L"%s\\BufferLine(%dX%d-%d)\\%03d.bmp",strSavePath,mParas->iLineW,mParas->iLPeriod,mParas->iLType,index);
		SaveBmp(m_pData,m_width,m_height,1,name.GetBuffer());
	}
	return iGray == 1 ? m_pData : m_pData2;
}

int PatternLine::GetFrameCount()
{
	if (mParas)
		return mParas->iLPeriod;
	return 0;
}

int PatternLine::GetMode()
{
	return PATTERN_LINE;
}

bool PatternLine::SetSave(bool bSave,CString strPath)
{
	bS=bSave;
	if (strPath!="")
	{	
		strSavePath=strPath;
		return true;
	}
	else
		bS=false;
	return false;
}

void PatternLine::Binary2Gray(int width, int height, unsigned char* src, unsigned char* dst)
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
