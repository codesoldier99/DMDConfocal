#pragma once

//不能写入中文，除非在写的时候切换语言环境
//http://www.xuebuyuan.com/1560792.html
//http://blog.csdn.net/python/article/details/5663184
class CLogLib
{
public:
	CLogLib(void);
	~CLogLib(void);
	void InitLogging(CString Path, CString Add = L"");
	void SetLogSize(int mLogSize);
	void PrintTxt(CString OutString);
	void ShutdownLogging(); 

private:
	int Max_log_size;
	int mSize;
	struct ::tm tm_time;
	time_t timestamp;
	CString strPath;
	CStdioFile myFile;
	CString strAdd;

	void NewTxt(bool bAdd=false);
	void CloseTxt();
	double GetCurrentTimeMS();

	int gettimeofday(struct timeval *tv, void* tz);
	int GetTID();
	CRITICAL_SECTION m_crtSec;
};

