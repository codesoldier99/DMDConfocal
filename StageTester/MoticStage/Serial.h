
#pragma once

class IFeed
{
public:
	virtual void DataArrival(const char* buf, int len) = 0;
};

// _DEBUG_SERIAL_PORT µ÷ÊÔ´®¿Ú¶¨Òå
#define _DEBUG_SERIAL_PORT
#undef _DEBUG_SERIAL_PORT

class Serial
{
public:
	Serial(void);
	~Serial(void);
	int Open(int portnum, int baudrates);
	int Open(LPCTSTR portname, int baudrates);
	void Close();
	bool IsOpened();
	int Send(const char *buf);
	void SetFeed(IFeed* feed);
private:
	friend DWORD WINAPI SerialReadProc(LPVOID lp);
	void ReadProc();
private:
	HANDLE _com;
	HANDLE _thread;
	OVERLAPPED _ow;
	OVERLAPPED _omask;
	OVERLAPPED _or;
	IFeed* _feed;
	CRITICAL_SECTION m_crtSec;
#ifdef _DEBUG_SERIAL_PORT
	// ¶Á»º³å
	FILE *_readlog;
	char _readbuf[100000];
	int _readpos;
	// Ð´»º³å
	FILE *_writelog;
	char _writebuf[100000];
	int _writepos;
#endif
};
