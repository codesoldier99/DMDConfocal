#include "StdAfx.h"
#include "LogLib.h"
#include <locale.h>

#define EPOCHFILETIME (116444736000000000ULL)

CLogLib::CLogLib(void)
{
	Max_log_size=1;
	strPath=L".\\";
	mSize=0;
	strAdd = L"";
	InitializeCriticalSection(&m_crtSec);
}


CLogLib::~CLogLib(void)
{
	//CloseTxt();
}


void CLogLib::InitLogging(CString Path,CString Add)
{
	strPath = Path;
	strAdd = Add;
	NewTxt();
	//myFile=mFile;
}

void CLogLib::SetLogSize(int mLogSize)
{
	Max_log_size=mLogSize;
}

void CLogLib::PrintTxt(CString OutString)
{
	double mV=GetCurrentTimeMS();
	CString strM;
	strM.Format(L"%6f: ",mV);
	OutString=strM+OutString;
	mSize+=OutString.GetLength();
	EnterCriticalSection(&m_crtSec);
	if ((mSize>>20)>=Max_log_size)
	{	
		CloseTxt();
		NewTxt(true);	
	}
	myFile.WriteString(OutString+L"\n");
	LeaveCriticalSection(&m_crtSec);
}

void CLogLib::ShutdownLogging()
{
	CloseTxt();
}

void CLogLib::NewTxt(bool bAdd)
{
	//myFile.Close();
	mSize=0;
	CString ImgName;
	SYSTEMTIME st;
	GetLocalTime(&st);
	if (bAdd)
		ImgName.Format(_T("Add%d%02d%02d-%02d%02d%02d%s.txt"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond, strAdd);
	else 
		ImgName.Format(_T("%d%02d%02d-%02d%02d%02d%s.txt"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond, strAdd);
	ImgName=strPath+L"\\"+ImgName;
	CFileException fileException; 
	if(!myFile.Open(ImgName,CFile::modeCreate|CFile::typeText|CFile::modeRead|CFile::modeWrite),&fileException) 
	{ 
		CString strMsg;
		strMsg.Format(L"Can't open file %s, error = %u",ImgName,fileException.m_cause);
		//AfxMessageBox(strMsg);
	}
	CString strOut;
	_time64( &timestamp );
	_localtime64_s(&tm_time, &timestamp);
	strOut.Format(L"Create at %d/%02d/%02d %02d:%02d:%02d\n",1900+tm_time.tm_year,1+tm_time.tm_mon,tm_time.tm_mday,
		tm_time.tm_hour,tm_time.tm_min,tm_time.tm_sec);
	setlocale( LC_CTYPE, "chs" );//Éè¶¨ 
	myFile.WriteString(strOut);
}

void CLogLib::CloseTxt()
{
	CString str=myFile.GetFileName();
	if (myFile.GetFileName()!=L"")
	{
		myFile.Close();
	}	
}

//static int gettimeofday(struct timeval *tv, void* tz) 
int CLogLib::gettimeofday(struct timeval *tv, void* tz)
{
	FILETIME ft;
	LARGE_INTEGER li;
	unsigned int tt;
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;
	tt = (li.QuadPart - EPOCHFILETIME) / 10;
	tv->tv_sec = tt / 1000000;
	tv->tv_usec = tt % 1000000;
	return 0;
}

double CLogLib::GetCurrentTimeMS()    
{    
	struct timeval tv;    
	gettimeofday(&tv,NULL);    
	return tv.tv_sec + (tv.tv_usec+0.0f) / 1000000;    
} 

int CLogLib::GetTID()
{
	// On Linux and MacOSX, we try to use gettid().
	#if defined OS_LINUX || defined OS_MACOSX
	#ifndef __NR_gettid
	#ifdef OS_MACOSX
	#define __NR_gettid SYS_gettid
	#elif ! defined __i386__
	#error "Must define __NR_gettid for non-x86 platforms"
	#else
	#define __NR_gettid 224
	#endif
	#endif
		static bool lacks_gettid = false;
		if (!lacks_gettid) {
			pid_t tid = syscall(__NR_gettid);
			if (tid != -1) {
				return tid;
			}
			// Technically, this variable has to be volatile, but there is a small
			// performance penalty in accessing volatile variables and there should
			// not be any serious adverse effect if a thread does not immediately see
			// the value change to "true".
			lacks_gettid = true;
		}
	#endif  // OS_LINUX || OS_MACOSX

		// If gettid() could not be used, we use one of the following.
	#if defined OS_LINUX
		return getpid();  // Linux:  getpid returns thread ID when gettid is absent
	#elif defined OS_WINDOWS || defined OS_CYGWIN
		return GetCurrentThreadId();
	#else
		// If none of the techniques above worked, we use pthread_self().
		return (pid_t)(uintptr_t)pthread_self();
	#endif
}