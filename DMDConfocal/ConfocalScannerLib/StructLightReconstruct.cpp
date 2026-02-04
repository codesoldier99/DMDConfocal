#include "StdAfx.h"
#include "StructLightReconstruct.h"
#include <algorithm>

DWORD WINAPI SI_ImageProc(LPVOID lp)
{
  ((StructLightReconstruct*)lp)->ProcessImage();
  return 0;
}

DWORD WINAPI SI_SaveProc(LPVOID lp)
{
  ((StructLightReconstruct*)lp)->DoSaveProc();
  return 0;
}

StructLightReconstruct::StructLightReconstruct(void)
{
	m_NowIndex=0;
	m_bmpResult=0;//结果
	m_bmpTemp=0;
	m_bStart=false;
	m_bExit=true;//等待线程
	m_MergeCount=1;//默认出一侦
	m_hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_bSaveBmp=false;
	bSaveConfocal=false;
	m_imgsize=0;//图像大小
	m_RenderChain=0;
	strSaveFile=L"";
	//存图相关变量
	iSaveW=0;
	iSaveH=0;
	iBit=0;
	thSavePath=L"";
	m_SaveTemp=0;
	m_iBackTh=0;
	//m_hSaveEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
  _has_dark_img = false;
}

StructLightReconstruct::~StructLightReconstruct(void)
{
	m_bExit=false;
	CloseHandle(m_hEvent);

	if(m_bmpResult)
		delete(m_bmpResult);
}

bool StructLightReconstruct::InitPlugin( LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	if(!m_ConfocalCore)
		return false;
	ResetEvent(m_hEvent);
	m_hGrabberThread = CreateThread(NULL, 0, SI_ImageProc, this, 0, 0);//这个开线程
	//设置渲染链
	m_RenderChain=m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_SL);
	return true;
}

void StructLightReconstruct::ProcessImage()
{
	CString strMsg;
	while(m_bExit)
	{
		WaitForSingleObject(m_hEvent,INFINITE);
		ResetEvent(m_hEvent);//一定要消除掉
		// 获取起始图像
		if (!_has_dark_img)
		{
			if (IsDarkImage(iWidth, iHeight, m_bmpTemp, 3.0))
			{
				m_NowIndex = 0;
				_has_dark_img = true;
			}
			continue;
		}
		_imgs.push_back(vector<unsigned char>(m_bmpTemp,
        m_bmpTemp + iWidth * iHeight));
		this->Notify(this,NOTIFY_SI_INPROCESS,m_NowIndex);//刷新界面很耗费时间？不能刷新
		m_NowIndex++;//相加这个数
		if(m_NowIndex>=m_MergeCount)
		{
			// 合成图像
			ReconstructImage(_imgs, m_bmpResult);
				////////输出外界
				HVideoHeader tHeader;
				tHeader.Vwidth=iWidth;
				tHeader.Vheight=iHeight;
				tHeader.VnCount=m_MergeCount;//总数
				tHeader.Vbuffer=m_bmpResult;//缓存
				tHeader.VwBit=8;//固定8位--->
				tHeader.Vsize=iWidth*iHeight*tHeader.VwBit/8;
				////////输出到核心渲染库
				if(m_RenderChain)
					m_RenderChain->Renderer(&tHeader,m_bmpResult);
				//这里要改一下*************************************************************
				if(m_bSaveBmp)
				{			
					strMsg.Format(L"Begin Save Result %d",m_NowIndex-1);
					//WaitForSingleObject(m_hSaveThread,INFINITE);
					m_ConfocalCore->LogString(strMsg);
					//TRACE(strMsg);
					m_bSaveBmp=false;
					if (iSaveW!=iWidth)
					{
						if (m_SaveTemp)
							delete m_SaveTemp;
						iSaveW=iWidth;iSaveH=iHeight;iBit=8;
						m_SaveTemp=(LPBYTE)malloc(tHeader.Vsize);				
					}	
					thSavePath=strSaveFile.GetBuffer();
					memcpy(m_SaveTemp,m_bmpResult,m_imgsize);
					m_hSaveThread= CreateThread(NULL, 0, SI_SaveProc, this, 0, 0);//开启存图线程
					//SaveBmp(m_bmpResult,iWidth,iHeight,8,strSaveFile.GetBuffer());				
				}
				RestMerge();//清零m_NowIndex
				_has_dark_img = false;
				_imgs.clear();
		}
		strMsg.Format(L"Have Finished ProcessImage %d",m_NowIndex-1);
		m_ConfocalCore->LogString(strMsg);
		//TRACE(strMsg);	
	}
}

void StructLightReconstruct::DoSaveProc()
{
	SaveBmp(m_SaveTemp,iSaveW,iSaveH,iBit,thSavePath);
	CString strMsg;
	strMsg.Format(L"Have Save Image %d",m_NowIndex);
	m_ConfocalCore->LogString(strMsg);
}

void StructLightReconstruct::RestMerge(void)
{
	//m_DmdManage->SetStatus(DMD_STOP);
	if (m_bmpResult)
		memset(m_bmpResult,0,iWidth*iHeight);//清零
	m_NowIndex=0;//重置
	if (bSaveConfocal)
	{
		this->Notify(this,NOTIFY_SI_SAVEPROCESSEND);
	}
	bSaveConfocal=false;
}

void StructLightReconstruct::GetParam()//获取参数
{
	strPath=m_ConfocalCore->GetConfigure()->GetString(L"DMDParas",L"strPath",L"Confocal_DMDControl");
}

void StructLightReconstruct::Renderer(HVideoHeader* pHeader,LPBYTE pBuffer)
{
	CString strMsg;
	if(m_imgsize!=pHeader->Vsize)//缓存再定
	{
		iWidth=pHeader->Vwidth;
		iHeight=pHeader->Vheight;
		m_imgsize=pHeader->Vsize;
		if(m_bmpResult)
			delete(m_bmpResult);
		m_bmpResult=(LPBYTE)malloc(m_imgsize);//几位的还没有定哦！！，用于做图像叠加处理
		if(m_bmpTemp)
			delete(m_bmpTemp);
		m_bmpTemp=(LPBYTE)malloc(m_imgsize);//缓存 图像 
	}
	if (bSaveConfocal)
	{
		strMsg.Format(L"SI Save %d",m_NowIndex);
		//WaitForSingleObject(m_hSaveThread,INFINITE);
		m_ConfocalCore->LogString(strMsg);
    //TRACE(strMsg);
		CString fname;
		fname=strPath+L"\\SIData";
		BuildDirectory(fname);
		fname.Format(strPath+L"\\SIData\\%03d.bmp",m_NowIndex);
		if (iSaveW!=pHeader->Vwidth||iBit!=pHeader->VwBit)
		{
			if (m_SaveTemp)
				delete m_SaveTemp;
			iSaveW=pHeader->Vwidth;iSaveH=pHeader->Vheight;iBit=pHeader->VwBit;
			m_SaveTemp=(LPBYTE)malloc(pHeader->Vsize);					
		}
		thSavePath=fname;
		memcpy(m_SaveTemp,pBuffer,m_imgsize);
		m_hSaveThread= CreateThread(NULL, 0, SaveProc, this, 0, 0);//开启存图线程
		//SaveBmp(pBuffer,pHeader->Vwidth,pHeader->Vheight,pHeader->VwBit,fname);
	}
	memcpy(m_bmpTemp,pBuffer,m_imgsize);//这里的图像大小也还没有定哦
	strMsg.Format(L"SI Renderered %d",m_NowIndex);
	m_ConfocalCore->LogString(strMsg);
  //TRACE(strMsg);
	SetEvent(m_hEvent);//处理事件
}

void StructLightReconstruct::Save_Result()
{
	CString ImgName;
	SYSTEMTIME st;
	GetLocalTime(&st);
	CFileDialog fileDlg(false);
	fileDlg.m_ofn.lpstrTitle=_T("SaveImg");
	fileDlg.m_ofn.lpstrFilter=_T("Bmp Files(*.bmp)\0*.bmp\0All Files(*.*)\0*.*\0\0");
	fileDlg.m_ofn.lpstrDefExt=_T("bmp");
	ImgName.Format(_T("%02d-%02d_%02d%02d%02d.bmp"),st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	fileDlg.m_ofn.lpstrFile = ImgName.GetBuffer(200);  
	fileDlg.m_ofn.nMaxFile = 200;
	if (IDOK==fileDlg.DoModal())
	{
		strSaveFile=fileDlg.GetPathName();
		m_bSaveBmp=true;
	}
}

int StructLightReconstruct::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	switch(ID)
	{
		case NOTIFY_SI_START:
			if(m_bStart)
			{
				TRACE("SIMode has start!\n");
				break;
			}	
			m_ConfocalCore->InitLog();
			m_ConfocalCore->GetRenderChain()->push_back(this);//开始结构光
			GetParam();//获取参数
			m_MergeCount=wParam;
      _imgs.clear();
			m_iBackTh=fParam;
			m_bStart=true;
			TRACE("SIMode Start!\n");
			break;
		case NOTIFY_SI_STOP:
			if(!m_bStart)
			{
				TRACE("SIMode waitfor start!\n");
				break;
			}
			m_ConfocalCore->CloseLog();
			m_ConfocalCore->GetRenderChain()->remove(this);
			m_bStart=false;
			RestMerge();
			TRACE("SIMode stop!\n");
			break;
		case NOTIFY_SI_SAVERESULT:
			Save_Result();
			break;
		case NOTIFY_SI_RESUM:
			RestMerge();//重置取图
			break;
		case NOTIFY_SI_SAVEPROCESS:
			bSaveConfocal=!bSaveConfocal;//true;
			break;
		default:
			break;
	}
	return 0;
}

bool StructLightReconstruct::BuildDirectory(CString strPath)
{
	CString strSubPath;
	CString strInfo;
	int nCount = 0; 
	int nIndex = 0;
	do
	{
		nIndex = strPath.Find(L"\\",nIndex) + 1;
		nCount++;
	}while( (nIndex-1) != -1);
	nIndex = 0;
	while( (nCount-1) >= 0)
	{
		nIndex = strPath.Find(L"\\",nIndex) + 1;
		if( (nIndex - 1) == -1)
		{
			strSubPath = strPath;
		}
		else
			strSubPath = strPath.Left(nIndex);
		if(!PathFileExists(strSubPath))
		{
			if(!::CreateDirectory(strSubPath,NULL))
			{
				strInfo = L"Build Directory";
				strInfo += strSubPath;
				strInfo += L" Fail!";
				AfxMessageBox(strInfo,MB_OK);
				return FALSE;
			}
		}
		nCount--;
	};
	return TRUE;
}

bool StructLightReconstruct::IsDarkImage(int w, int h, unsigned char* img, double thr)
{
  if (w <= 0 || h <= 0 || !img)
  {
    return false;
  }

  double sum(0), sum2(0);
  for (int i = 0; i < w * h; i++)
  {
    sum += img[i];
    sum2 += img[i] * img[i];
  }
  int n = w * h;
  double stdev = sqrt((sum2 - sum * sum / n) / (n - 1));
  return stdev < thr;
}

int StructLightReconstruct::ReconstructImage(vector<vector<unsigned char>>& imgs,
  unsigned char* merge)
{
  if (imgs.size() == 3)
  {
    int len = imgs[0].size();
    unsigned char* a0 = imgs[0].data();
    unsigned char* b0 = imgs[1].data();
    unsigned char* c0 = imgs[2].data();
    // I=sqrt((2*I2-I1-I3).^2+(I3-I1).^2)./2;
    for (int i = 0; i < len; i++)
    {
      double s = (2 * b0[i] - a0[i] - c0[i]);
      double t = c0[i] - a0[i];
      double res = sqrt(s * s + t * t) / 2;
      int r = (int)res;
      if (r < 0)r = 0;
      if (r > 255)r = 255;
      merge[i] = r;
    }
  }
  else if (imgs.size() == 6)
  {
    int len = imgs[0].size();
    unsigned char* a0 = imgs[0].data();
    unsigned char* b0 = imgs[1].data();
    unsigned char* c0 = imgs[2].data();
    unsigned char* a1 = imgs[3].data();
    unsigned char* b1 = imgs[4].data();
    unsigned char* c1 = imgs[5].data();
    // I=sqrt((2*I2-I1-I3).^2+(I3-I1).^2)./2;
    for (int i = 0; i < len; i++)
    {
      double s = (2 * b0[i] - a0[i] - c0[i]);
      double t = c0[i] - a0[i];
      double res = sqrt(s * s + t * t) / 2;

      s = (2 * b1[i] - a1[i] - c1[i]);
      t = c1[i] - a1[i];
      double res2 = sqrt(s * s + t * t) / 2;

      int r = (int)((res + res2) / 2);
      if (r < 0)r = 0;
      if (r > 255)r = 255;
      merge[i] = r;
    }
  }
  return 0;
}
