#pragma once
#include "IPatternGenerate.h"
namespace WlpDMDControl
{
  class PatternGeneratorImpl : public IPatternGenerate
  {
  public:
    PatternGeneratorImpl();

    virtual ~PatternGeneratorImpl();

    virtual void Gernerate(void* Paras);
    virtual unsigned char* GetData(int index, int iGray);
    virtual int GetFrameCount();
    virtual int GetMode();
    virtual bool SetSave(bool bSave, CString strPath);
    virtual int SetSize(int width, int height);
    virtual void GetSize(int* width, int* height);
  protected:
    bool SaveBmp(LPBYTE pBuffer,
      long IWidth, long IHeight, int wBitsPerPixel, LPCTSTR lpszFileName);

    bool BuildDirectory(CString strPath);

    int Bit2Byte(int len);
  protected:
    int m_width;
    int m_height;
  };
}