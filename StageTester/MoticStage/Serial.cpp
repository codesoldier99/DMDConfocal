#include "StdAfx.h"
#include ".\serial.h"
#include "stdio.h"

DWORD WINAPI SerialReadProc(LPVOID lp)
{
	((Serial*)lp)->ReadProc();
	return 0;
}

Serial::Serial(void)
{
	_com = INVALID_HANDLE_VALUE;
	_thread = 0;
	_feed = 0;
	InitializeCriticalSection(&m_crtSec);
}

Serial::~Serial(void)
{

}

int Serial::Open(int portnum, int baudrates)
{
	if (IsOpened())//已经打开了
		Close();
	
	//char port[32];
	CString port;
	port.Format(L"\\\\.\\COM%d",portnum);

	LPCTSTR t=port;
	//sprintf(port, "COM%d", portnum);
	_com = CreateFile(t,//port,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);

	if(_com == INVALID_HANDLE_VALUE)return -1;

	DCB dcb;
	if(!GetCommState(_com, &dcb))return -1;

	dcb.BaudRate = baudrates;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	if(!SetCommState(_com, &dcb))return -1;

	SetCommMask(_com,EV_RXCHAR);

	_ow.hEvent = CreateEvent(0, FALSE, FALSE, 0);
	_ow.Internal = 0;
	_ow.InternalHigh = 0;
	_ow.Offset = 0;
	_ow.OffsetHigh = 0;
	_ow.Pointer = 0;

	_omask.hEvent = CreateEvent(0, FALSE, FALSE, 0);
	_omask.Internal = 0;
	_omask.InternalHigh = 0;
	_omask.Offset = 0;
	_omask.OffsetHigh = 0;
	_omask.Pointer = 0;

	_or.hEvent = CreateEvent(0, FALSE, FALSE, 0);
	_or.Internal = 0;
	_or.InternalHigh = 0;
	_or.Offset = 0;
	_or.OffsetHigh = 0;
	_or.Pointer = 0;

	// 注意顺序，先创建事件，然后创建线程
	_thread = CreateThread(0, 0, SerialReadProc, this, 0, 0);
	return 0;
}

int Serial::Open(LPCTSTR portname, int baudrates)
{
	if (IsOpened())//已经打开了
		Close();

	//char port[32];
	//CString port;
	//port.Format(L"COM%d",portnum);

	//LPCTSTR t=port;
	//sprintf(port, "COM%d", portnum);
	_com = CreateFile(portname,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);

	if(_com == INVALID_HANDLE_VALUE)return -1;

	DCB dcb;
	if(!GetCommState(_com, &dcb))return -1;

	dcb.BaudRate = baudrates;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	if(!SetCommState(_com, &dcb))return -1;

	SetCommMask(_com,EV_RXCHAR);

	_ow.hEvent = CreateEvent(0, FALSE, FALSE, 0);
	_ow.Internal = 0;
	_ow.InternalHigh = 0;
	_ow.Offset = 0;
	_ow.OffsetHigh = 0;
	_ow.Pointer = 0;

	_omask.hEvent = CreateEvent(0, FALSE, FALSE, 0);
	_omask.Internal = 0;
	_omask.InternalHigh = 0;
	_omask.Offset = 0;
	_omask.OffsetHigh = 0;
	_omask.Pointer = 0;

	_or.hEvent = CreateEvent(0, FALSE, FALSE, 0);
	_or.Internal = 0;
	_or.InternalHigh = 0;
	_or.Offset = 0;
	_or.OffsetHigh = 0;
	_or.Pointer = 0;

	// 注意顺序，先创建事件，然后创建线程
	_thread = CreateThread(0, 0, SerialReadProc, this, 0, 0);
	return 0;
}

void Serial::Close()
{
	if(!IsOpened())return;

	SetEvent(_com);

	CloseHandle(_ow.hEvent);
	CloseHandle(_omask.hEvent);
	CloseHandle(_or.hEvent);
	_ow.hEvent = INVALID_HANDLE_VALUE;
	_omask.hEvent = INVALID_HANDLE_VALUE;
	_or.hEvent = INVALID_HANDLE_VALUE;

	SetEvent(_com);

	CloseHandle(_com);
	_com = INVALID_HANDLE_VALUE;

	SetEvent(_thread);
	
	if(INVALID_HANDLE_VALUE != _thread)
		WaitForSingleObject(_thread,INFINITE);
	//ExitThread()
	_thread = INVALID_HANDLE_VALUE;
}

bool Serial::IsOpened()
{
	return (_com != INVALID_HANDLE_VALUE);
}

int Serial::Send(const char *buf)
{
	//ASSERT(_com != INVALID_HANDLE_VALUE);
	EnterCriticalSection(&m_crtSec);
	WriteFile(_com, buf, strlen(buf), 0, &_ow);
	DWORD len;
	if(GetOverlappedResult(_com, &_ow, &len, TRUE))
	{
		LeaveCriticalSection(&m_crtSec);
		return 0;
	}
	LeaveCriticalSection(&m_crtSec);
	return -1;
}

void Serial::ReadProc()
{
	COMSTAT stat;
	DWORD err;
	DWORD len;
	char buf[1024];
	DWORD mask;
	while(1)
	{
		mask = 0;
		//WaitCommEvent(_com, &mask, &_omask);//140124删这个--这个删了是usb转串的专属
		if(_omask.hEvent == INVALID_HANDLE_VALUE)break;
		GetOverlappedResult(_com, &_omask, &len, TRUE);
		//if(mask & EV_RXCHAR)//140124删这个
		{
			ClearCommError(_com, &err, &stat);
			if(stat.cbInQue > 0)
			{
				ReadFile(_com, buf, stat.cbInQue, &len, &_or);
				GetOverlappedResult(_com, &_or, &len, TRUE);
				Sleep(1); // 依次 Reset 各轴会失败
				if(_feed)_feed->DataArrival(buf, len);
			}
			else
				Sleep(3);
		}
	}
}

void Serial::SetFeed(IFeed* feed)
{
	_feed = feed;
}