#include "stdafx.h"
#include "PatternGeneratorImpl.h"

namespace WlpDMDControl
{
  PatternGeneratorImpl::PatternGeneratorImpl()
  {
    m_width = 1024;
    m_height = 768;
  }

  PatternGeneratorImpl::~PatternGeneratorImpl()
  {
  }
  void PatternGeneratorImpl::Gernerate(void* Paras)
  {

  }
  unsigned char* PatternGeneratorImpl::GetData(int index, int iGray)
  {
    return 0;
  }
  int PatternGeneratorImpl::GetFrameCount()
  {
    return 0;
  }
  int PatternGeneratorImpl::GetMode()
  {
    return 0;
  }
  bool PatternGeneratorImpl::SetSave(bool bSave, CString strPath)
  {
    return false;
  }
  int PatternGeneratorImpl::SetSize(int width, int height)
  {
    if (width <= 0 || height <= 0)
    {
      return -1;
    }

    m_width = width;
    m_height = height;
    return 0;
  }
  void PatternGeneratorImpl::GetSize(int* width, int* height)
  {
    if (width)*width = m_width;
    if (height)*height = m_height;
  }

  bool PatternGeneratorImpl::SaveBmp(LPBYTE pBuffer,
    long IWidth, long IHeight, int wBitsPerPixel, LPCTSTR lpszFileName)
  {
    if (wBitsPerPixel == 8)
    {
      BITMAPINFOHEADER bmpinfoHeader = { 0 };//没有考虑4的倍数哦。。
      bmpinfoHeader.biSize = sizeof(BITMAPINFOHEADER);
      bmpinfoHeader.biBitCount = wBitsPerPixel;
      bmpinfoHeader.biClrImportant = 0;//use all colors
      bmpinfoHeader.biClrUsed = 0;//use an many colors according to bits per pixel
      bmpinfoHeader.biCompression = BI_RGB;
      bmpinfoHeader.biHeight = IHeight;
      bmpinfoHeader.biWidth = IWidth;
      bmpinfoHeader.biPlanes = 1;//使用默认调色
      bmpinfoHeader.biSizeImage = IWidth*IHeight*(wBitsPerPixel / 8);

      BITMAPFILEHEADER bfh = { 0 };
      bfh.bfType = 0x4D42;//"BM"
      bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//图像数据偏移字节
      bfh.bfSize = bfh.bfOffBits + bmpinfoHeader.biSizeImage;//文件大小 

      RGBQUAD *pRgb(0);//调色板
      int	sizeQuad = sizeof(RGBQUAD) * 256;
      if (wBitsPerPixel == 8)//加调色板
      {
        pRgb = (RGBQUAD*)malloc(sizeQuad);
        for (int i = 0; i < 256; i++)
        {
          pRgb[i].rgbBlue = i;
          pRgb[i].rgbGreen = i;
          pRgb[i].rgbRed = i;
        }
        bfh.bfOffBits += sizeQuad;
        bfh.bfSize += sizeQuad;
      }

      HANDLE hFile = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      if (!hFile)
      {
        free(pRgb);
        return FALSE;
      }

      DWORD dwWritten = 0;
      WriteFile(hFile, &bfh, sizeof(bfh), &dwWritten, NULL);
      WriteFile(hFile, &bmpinfoHeader, sizeof(bmpinfoHeader), &dwWritten, NULL);
      if (wBitsPerPixel == 8)//加调色板
      {
        WriteFile(hFile, pRgb, sizeQuad, &dwWritten, NULL);
        free(pRgb);
      }
      WriteFile(hFile, pBuffer, bmpinfoHeader.biSizeImage, &dwWritten, NULL);

      CloseHandle(hFile);
      return TRUE;
    }
    else if (wBitsPerPixel == 1)
    {
      BITMAPINFOHEADER bmpinfoHeader = { 0 };//没有考虑4的倍数哦。。
      bmpinfoHeader.biSize = sizeof(BITMAPINFOHEADER);
      bmpinfoHeader.biBitCount = wBitsPerPixel;
      bmpinfoHeader.biClrImportant = 0;//use all colors
      bmpinfoHeader.biClrUsed = 0;//use an many colors according to bits per pixel
      bmpinfoHeader.biCompression = BI_RGB;
      bmpinfoHeader.biHeight = IHeight;
      bmpinfoHeader.biWidth = IWidth;
      bmpinfoHeader.biPlanes = 1;//使用默认调色
      bmpinfoHeader.biSizeImage = IWidth*IHeight*wBitsPerPixel / 8;

      BITMAPFILEHEADER bfh = { 0 };
      bfh.bfType = 0x4D42;//"BM"
      bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//图像数据偏移字节
      bfh.bfSize = bfh.bfOffBits + bmpinfoHeader.biSizeImage;//文件大小 

      RGBQUAD *pRgb(0);//调色板
      int	sizeQuad = sizeof(RGBQUAD) * 2;
      if (wBitsPerPixel == 1)//加调色板
      {
        pRgb = (RGBQUAD*)new BYTE[sizeQuad];

        pRgb[0].rgbBlue = 0;
        pRgb[0].rgbGreen = 0;
        pRgb[0].rgbRed = 0;

        pRgb[1].rgbBlue = 255;
        pRgb[1].rgbGreen = 255;
        pRgb[1].rgbRed = 255;

        bfh.bfOffBits += sizeQuad;
        bfh.bfSize += sizeQuad;
      }

      HANDLE hFile = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      if (!hFile)
      {
        delete[]pRgb;
        return FALSE;
      }

      DWORD dwWritten = 0;
      WriteFile(hFile, &bfh, sizeof(bfh), &dwWritten, NULL);
      WriteFile(hFile, &bmpinfoHeader, sizeof(bmpinfoHeader), &dwWritten, NULL);
      if (wBitsPerPixel != 24)//加调色板
      {
        WriteFile(hFile, pRgb, sizeQuad, &dwWritten, NULL);
        delete []pRgb;
      }
      WriteFile(hFile, pBuffer, bmpinfoHeader.biSizeImage, &dwWritten, NULL);

      CloseHandle(hFile);
      return TRUE;
    }
    return FALSE;
  }

  bool PatternGeneratorImpl::BuildDirectory(CString strPath)
  {
    CString strSubPath;
    CString strInfo;
    int nCount = 0;
    int nIndex = 0;
    do
    {
      nIndex = strPath.Find(L"\\", nIndex) + 1;
      nCount++;
    } while ((nIndex - 1) != -1);
    nIndex = 0;
    while ((nCount - 1) >= 0)
    {
      nIndex = strPath.Find(L"\\", nIndex) + 1;
      if ((nIndex - 1) == -1)
      {
        strSubPath = strPath;
      }
      else
        strSubPath = strPath.Left(nIndex);
      if (!PathFileExists(strSubPath))
      {
        if (!::CreateDirectory(strSubPath, NULL))
        {
          strInfo = L"Build Directory";
          strInfo += strSubPath;
          strInfo += L" Fail!";
          AfxMessageBox(strInfo, MB_OK);
          return FALSE;
        }
      }
      nCount--;
    };
    return TRUE;
  }

  int PatternGeneratorImpl::Bit2Byte(int len)
  {
    return len / 8;
  }
}