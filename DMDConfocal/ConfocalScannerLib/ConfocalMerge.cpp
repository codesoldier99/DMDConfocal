#include "StdAfx.h"
#include "ConfocalMerge.h"

CConfocalMerge::CConfocalMerge(void)
{
	for(int i=0;i<100;i++)
	{
		m_Template[i].buffer=0;
		m_Template[i].index=i;
	}
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
	m_bExType = false;
	//m_hSaveEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
}

CConfocalMerge::~CConfocalMerge(void)
{
	m_bExit=false;
	CloseHandle(m_hEvent);
	for (int i=0;i<100;i++)
	{
		if (m_Template[i].buffer)
		{
			delete(m_Template[i].buffer);
		}
	}
	if(m_bmpResult)
		delete(m_bmpResult);
}

bool CConfocalMerge::InitPlugin( LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	if(!m_ConfocalCore)
		return false;
	//m_DmdManage=m_ConfocalCore->GetDmdManager();
	//if(!m_DmdManage)
		//return false;
	//DWORD t1=::GetTickCount();
	////下面读入模板图
	//for(int i=0;i<100;i++)
	//{
	//	CString fname;
	//	fname.Format(L"../Calibrate/%03d.bmp",i);//读的文件
	//	if(!LoadBmp(&m_Template[i].buffer,iWidth,iHeight,wbit,fname))
	//		break;
	//}
	//DWORD t2=::GetTickCount();
	//TRACE(L"t2-t1=%d\n",t2-t1);
	ResetEvent(m_hEvent);
	m_hGrabberThread = CreateThread(NULL, 0, ImageProc, this, 0, 0);//这个开线程
	//m_ConfocalCore->GetRenderChain()->push_back(this);
	//设置渲染链
	if (p_Param2)
		m_bExType = *((bool*)p_Param2);
	if (m_bExType&&p_Param2)
		m_RenderChain = m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_ConfocalEx);
	else
		m_RenderChain = m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal);
	//m_RenderChain=m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_Confocal);
	return true;
}

DWORD WINAPI ImageProc( LPVOID lp )
{
	((CConfocalMerge*)lp)->ProcessImage();
	return 0;
}

void CConfocalMerge::ProcessImage()
{
	CString strMsg;
	while(m_bExit)
	{
		WaitForSingleObject(m_hEvent,INFINITE);
		ResetEvent(m_hEvent);//一定要消除掉
		//if (m_bProcess)//处理图像喽
		{	
			//Img8_And(m_bmpTemp,m_Template[m_NowIndex].buffer,iWidth,iHeight);//相与
			//Img8_Add(m_bmpResult,m_bmpTemp,iWidth,iHeight,0);//阈值处理相加
			Img8_Max(m_bmpResult,m_bmpTemp,iWidth,iHeight,m_iBackTh);
			if (m_bExType)
			{
				this->Notify(this, NOTIFY_CONFOCAL_INPROCESSEX, m_NowIndex);//刷新界面很耗费时间？不能刷新
			}
			else
			{
				this->Notify(this, NOTIFY_CONFOCAL_INPROCESS, m_NowIndex);//刷新界面很耗费时间？不能刷新
			}
			
			m_NowIndex++;//相加这个数
			if(m_NowIndex>=m_MergeCount)
			{
				////////输出外界
				HVideoHeader tHeader;
				tHeader.Vwidth=iWidth;
				tHeader.Vheight=iHeight;
				tHeader.VnCount=m_MergeCount;//总数
				tHeader.Vbuffer=m_bmpResult;//缓存
				tHeader.VwBit=8;//固定8位--->
				tHeader.Vsize=iWidth*iHeight*tHeader.VwBit/8;
				tHeader.Vuser = 0;
				if (m_bExType)
					tHeader.Vuser = 1;
				////////输出到核心渲染库
				if(m_RenderChain)
					m_RenderChain->Renderer(&tHeader,m_bmpResult);
				//这里要改一下*************************************************************
				if(m_bSaveBmp)
				{			
					strMsg.Format(L"Begin Save Result %d",m_NowIndex-1);
					//WaitForSingleObject(m_hSaveThread,INFINITE);
					m_ConfocalCore->LogString(strMsg,m_bExType);
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
					m_hSaveThread= CreateThread(NULL, 0, SaveProc, this, 0, 0);//开启存图线程
					//SaveBmp(m_bmpResult,iWidth,iHeight,8,strSaveFile.GetBuffer());				
				}
				RestMerge();//清零m_NowIndex
			}
			strMsg.Format(L"Have Finished ProcessImage %d",m_NowIndex-1);
			m_ConfocalCore->LogString(strMsg, m_bExType);
			//TRACE(strMsg);
			//m_bProcess=false;//处理完了
		}
	}
}

DWORD WINAPI SaveProc(LPVOID lp)
{
	((CConfocalMerge*)lp)->DoSaveProc();
	return 0;
}

void CConfocalMerge::DoSaveProc()
{
	SaveBmp(m_SaveTemp,iSaveW,iSaveH,iBit,thSavePath);
	CString strMsg;
	strMsg.Format(L"Have Save Image %d",m_NowIndex);
	m_ConfocalCore->LogString(strMsg, m_bExType);
}

void CConfocalMerge::RestMerge(void)
{
	//m_DmdManage->SetStatus(DMD_STOP);
	if (m_bmpResult)
		memset(m_bmpResult,0,iWidth*iHeight);//清零
	m_NowIndex=0;//重置
	if (bSaveConfocal)
	{
		this->Notify(this,NOTIFY_CONFOCAL_SAVEPROCESSEND);
	}
	bSaveConfocal=false;
}

void CConfocalMerge::GetParam()//获取参数
{
	//m_MergeCount=m_ConfocalCore->GetConfigure()->GetInt(L"DMDParas",L"MergeCount",L"Confocal_DMDControl");
	strPath=m_ConfocalCore->GetConfigure()->GetString(L"DMDParas",L"strPath",L"Confocal_DMDControl");
}

void CConfocalMerge::Renderer(HVideoHeader* pHeader,LPBYTE pBuffer)
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
		strMsg.Format(L"Confocal Save %d",m_NowIndex);
		//WaitForSingleObject(m_hSaveThread,INFINITE);
		m_ConfocalCore->LogString(strMsg, m_bExType);
		//TRACE(strMsg);
		CString fname;
		if (m_bExType)
		{
			fname = strPath + L"\\ConfocalDataEx";
			BuildDirectory(fname);
			fname.Format(strPath + L"\\ConfocalDataEx\\%03d.bmp", m_NowIndex);
		} 
		else
		{
			fname = strPath + L"\\ConfocalData";
			BuildDirectory(fname);
			fname.Format(strPath + L"\\ConfocalData\\%03d.bmp", m_NowIndex);
		}
		
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
	strMsg.Format(L"Confocal Renderered %d",m_NowIndex);
	m_ConfocalCore->LogString(strMsg, m_bExType);
	//TRACE(strMsg);
	SetEvent(m_hEvent);//处理事件
}

void CConfocalMerge::Save_Result(CString PathFile)
{
	if (PathFile != L"")
	{
		strSaveFile = PathFile;
	}
	else
	{
		CString ImgName;
		SYSTEMTIME st;
		GetLocalTime(&st);
		CFileDialog fileDlg(false);
		fileDlg.m_ofn.lpstrTitle = _T("SaveImg");
		fileDlg.m_ofn.lpstrFilter = _T("Bmp Files(*.bmp)\0*.bmp\0All Files(*.*)\0*.*\0\0");
		fileDlg.m_ofn.lpstrDefExt = _T("bmp");
		ImgName.Format(_T("%02d-%02d_%02d%02d%02d.bmp"), st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		fileDlg.m_ofn.lpstrFile = ImgName.GetBuffer(200);
		fileDlg.m_ofn.nMaxFile = 200;
		if (IDOK == fileDlg.DoModal())
			strSaveFile = fileDlg.GetPathName();
		else
			return;
	}
	if (m_bExType)
	{
		int nPos = strSaveFile.ReverseFind('.');
		strSaveFile = strSaveFile.Left(nPos);
		strSaveFile += L"_Ex.bmp";
	}
	m_bSaveBmp = true;
	
}

int CConfocalMerge::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	switch(ID)
	{
		case NOTIFY_CONFOCAL_START:
			if(m_bStart)
			{
				TRACE("ConfocalMode has start!\n");
				break;
			}	
			
			m_ConfocalCore->InitLog(m_bExType);
			if (m_bExType)
			{
				if (m_ConfocalCore->GetCurVideoEx() == 0)
				{
					break;
				}
				m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_NormalEx)->push_back(this);
			}			
			else
			{
				m_ConfocalCore->GetRenderChain()->push_back(this);//开始共聚焦
			}
				
			GetParam();//获取参数
			m_MergeCount=wParam;
			m_iBackTh=fParam;
			m_bStart=true;
			TRACE("ConfocalMode Start!\n");
			break;
		case NOTIFY_CONFOCAL_STOP:
			if(!m_bStart)
			{
				TRACE("ConfocalMode waitfor start!\n");
				break;
			}
			m_ConfocalCore->CloseLog(m_bExType);
			if (m_bExType)
			{
				m_ConfocalCore->GetRenderChain(HConfocalCore::RenderChain_NormalEx)->remove(this);
			}
			else
			{
				m_ConfocalCore->GetRenderChain()->remove(this);
			}
			m_bStart=false;
			RestMerge();
			TRACE("ConfocalMode stop!\n");
			break;
		case NOTIFY_CONFOCAL_SAVERESULT:
			if (pParam != NULL)
			{
				CString strFile = *((CString*)pParam);
				Save_Result(strFile);
			}
			else
				Save_Result();
			break;
		case NOTIFY_CONFOCAL_RESUM:
			RestMerge();//重置取图
			break;
		case NOTIFY_CONFOCAL_SAVEPROCESS:
			bSaveConfocal=!bSaveConfocal;//true;
			break;
		default:
			break;
	}
	return 0;
}

bool CConfocalMerge::BuildDirectory(CString strPath)
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


