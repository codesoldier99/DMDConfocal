#include "StdAfx.h"
#include "VideoType.h"


CVideoType::CVideoType(void)
{
}


CVideoType::~CVideoType(void)
{
	mCameraTypes.clear();
}


int CVideoType::FindCameraType()
{
	CFileFind finder;
	HMODULE t_mudule=GetModuleHandle(0);
	CString pfileName;
	GetModuleFileName(t_mudule,pfileName.GetBufferSetLength(MAX_PATH),MAX_PATH);
	pfileName.ReleaseBuffer();
	int nPos=pfileName.ReverseFind('\\');
	pfileName=pfileName.Left(nPos);
	pfileName+=L"\\*.cdm";
	bool bWork=finder.FindFile(pfileName);//设置路径
	while(bWork)
	{
		bWork=finder.FindNextFileW();
		if(finder.IsDots())
			continue;//跳过.和..
		CString t_moudulename=finder.GetFilePath();
		TRACE(L"Find a file: %s\n",t_moudulename);//输出 
		VideoTypeParas mV;//=OpenVideoLib(t_moudulename);
		if (OpenVideoLib(t_moudulename,&mV))
		{
			mCameraTypes.push_back(mV);
		}
	}
	finder.Close();//关闭查找器
	return mCameraTypes.size();
}

int CVideoType::GetTypeCount()
{
	return mCameraTypes.size();
}

bool CVideoType::OpenVideoLib(CString pPath,VideoTypeParas* mVideoType)
{
	HMODULE t_module;
	t_module=::LoadLibrary(pPath);
	if(!t_module)
		return 0;
	HVideoManager** tManager=0;
	tManager=(HVideoManager**)::GetProcAddress(t_module,"m_pVieoManager");
	if(!tManager)
		return 0;
	(*tManager)->SearchCameras();
	int tount=(*tManager)->GetCameraCount();
	if (tount>0)
	{
		int nPos=pPath.ReverseFind('\\');
		CString strV=pPath.Mid(nPos+1);//pPath.Right(nPos);
		nPos=strV.ReverseFind('.');
		strV=strV.Left(nPos);
		mVideoType->strCameraType=strV;
		mVideoType->mVideoManager=*tManager;
		return true;
	}
	else
		return false;
}

VideoTypeParas* CVideoType::GetCameraType(int pIndex)
{
	if (mCameraTypes.size()>=pIndex)
		return &mCameraTypes[pIndex];
	else
		return 0;
}


VideoTypeParas* CVideoType::GetCameraType(CString mCameraType)
{
	for (int i=0;i<mCameraTypes.size();i++)
	{
		if (mCameraTypes[i].strCameraType==mCameraType)
		{
			return &mCameraTypes[i];
		}
	}
	return 0;
}

