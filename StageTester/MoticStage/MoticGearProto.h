#pragma once
#include "serial.h"
#include "stdlib.h"
#include "stdio.h"
#include "HGearBox.h"
//using stdlib;

//class HGearBoxDispatcher//控制盒的消息在这里
//{
//public: 
//	virtual void Event_CommTest()=0;
//	//virtual void Event_CaptureImage() = 0;
//	//virtual void Event_FastScan() = 0;
//	//virtual void Event_AutoFocus() = 0;
//	//virtual void Event_LightChanged(int val) = 0;
//	//virtual void Event_JoystickObjChanged(bool cw) = 0;// 摇杆改变物镜
//	//virtual void Event_GetAxisCount(int count) = 0;// 得到轴的个数
//};


class HAxisDispatcher//轴的消息单独出来
{
public:
	virtual void Event_CommTest() = 0;
	virtual void Event_GetPosition(int pos) = 0;
	virtual void Event_Moved(int pos) = 0;
	virtual void Event_Error() = 0;
	virtual void Event_Reset() = 0;
	virtual void Event_IsMoving(bool moving) = 0;
	virtual void Event_LightChanged(int val){};
	//virtual void Event_GetEncorder(int pos)=0;
};

class MoticGearProto : public IFeed
{
public:
	MoticGearProto(void);
	~MoticGearProto(void);
	void Test(char* buf, int len)
	{
		int i = 0;
		buf[i++] = _begin;
		buf[i++] = '?';
		buf[i++] = _end;
		buf[i++] = '\0';
	}
	void Reset(char axis, int pos, char* buf, int len)
	{
		int i = 0;
		buf[i++] = _begin;
		buf[i++] = '\"';
		buf[i++] = axis;
		char tmp[32];

		//物镜重置指令为[*"W0$]，数字长度为1，不是8
		if(axis == 'W'||axis=='T')
		{
			sprintf(tmp, "%1X", (unsigned long)pos);
		}
		else
		{
			sprintf(tmp, "%08X", (unsigned long)pos);
		}

		memcpy(buf + i, tmp, strlen(tmp));
		i+= (int)strlen(tmp);

		buf[i++] = _end;
		buf[i++] = '\0';
	}
	void SetSpeed(char axis, int speed, char* buf, int len)
	{
		int i = 0;
		buf[i++] = _begin;
		buf[i++] = 'S';
		buf[i++] = 'P';
		buf[i++] = axis;

		char tmp[32];
		sprintf(tmp, "%04X", (unsigned short)speed);
		memcpy(buf + i, tmp, strlen(tmp));
		i+= (int)strlen(tmp);

		buf[i++] = _end;
		buf[i++] = '\0';
	}
	void Move(char axis, int offset, char* buf, int len)
	{
		if(axis == 'W')
		{
			if(abs(offset) > 4 || offset == 0)
			{
				return;
			}

			int i = 0;
			buf[i++] = _begin;
			buf[i++] = '"';
			buf[i++] = 'L';

			switch(offset)
			{
			case 1:
				buf[i++] = '1';
				buf[i++] = '1';
				break;
			case -1:
				buf[i++] = '1';
				buf[i++] = '0';
				break;
			case 2:
				buf[i++] = '2';
				buf[i++] = '1';
				break;
			case -2:
				buf[i++] = '2';
				buf[i++] = '0';
				break;
			case 3:
				buf[i++] = '3';
				buf[i++] = '1';
				break;
			case -3:
				buf[i++] = '3';
				buf[i++] = '0';
				break;
			default:break;
			}

			buf[i++] = _end;
			buf[i++] = '\0';
			return;
		}

		int i = 0;
		buf[i++] = _begin;
		buf[i++] = 'P';
		buf[i++] = 'R';
		buf[i++] = axis;
		buf[i++] = offset >= 0 ? '+' : '-';
		
		char tmp[32];
		sprintf(tmp, "%08X", abs(offset));
		memcpy(buf + i, tmp, strlen(tmp));
		i+= (int)strlen(tmp);

		buf[i++] = _end;
		buf[i++] = '\0';
	}
	void MoveConst(char axis, int speed, char* buf, int len)
	{
		int i = 0;
		buf[i++] = _begin;
		buf[i++] = 'J';
		buf[i++] = 'G';
		buf[i++] = axis;
		buf[i++] = speed >= 0 ? '+' : '-';

		char tmp[32];
		sprintf(tmp, "%04X", (unsigned short)abs(speed));
		memcpy(buf + i, tmp, strlen(tmp));
		i+= (int)strlen(tmp);

		buf[i++] = _end;
		buf[i++] = '\0';
	}
	void Stop(char axis, char* buf, int len)
	{
		int i = 0;
		buf[i++] = _begin;
		buf[i++] = 'S';
		buf[i++] = 'T';
		buf[i++] = axis;
		buf[i++] = _end;
		buf[i++] = '\0';
	}
	void GetPosition(char axis, char* buf, int len)
	{
		int i = 0;
		buf[i++] = _begin;
		buf[i++] = '^';
		buf[i++] = axis;
		buf[i++] = _end;
		buf[i++] = '\0';
	}
	void IsMoving(char axis, char* buf, int len)
	{
		int i = 0;
		buf[i++] = _begin;
		buf[i++] = '|';
		buf[i++] = axis;
		buf[i++] = _end;
		buf[i++] = '\0';
	}
	void SetPosition(char axis, int pos, char* buf, int len)
	{
		int i = 0;
		buf[i++] = _begin;
		buf[i++] = 'D';
		buf[i++] = 'P';
		buf[i++] = axis;

		char tmp[32];
		sprintf(tmp, "%08X", (unsigned long)pos);
		memcpy(buf + i, tmp, strlen(tmp));
		i+= (int)strlen(tmp);

		buf[i++] = _end;
		buf[i++] = '\0';
	}
	void EnableJoystick(bool enable, char* buf, int len)
	{
		int i = 0;
		buf[i++] = _begin;
		buf[i++] = '[';
		buf[i++] = 'K';
		buf[i++] = enable ? '1' : '0';
		buf[i++] = _end;
		buf[i++] = '\0';
	}
	void SetAxisCount(int cnt, char* buf, int len)
	{
		int i = 0;
		buf[i++] = _begin;
		buf[i++] = '[';
		buf[i++] = 'A';
		char tmp[3];
		sprintf(tmp, "%02X", cnt);
		buf[i++] = tmp[0];
		buf[i++] = tmp[1];
		buf[i++] = _end;
		buf[i++] = '\0';
	}
	void GetAxisCount(char* buf, int len)
	{
		//*[R0A$
		int i = 0;
		buf[i++] = _begin;
		buf[i++] = '[';
		buf[i++] = 'R';
		buf[i++] = '0';
		buf[i++] = 'A';
		buf[i++] = _end;
		buf[i++] = '\0';
	}
	void SetLight(int light, char* buf, int len)
	{
		int i = 0;
		buf[i++] = _begin;
		buf[i++] = '[';
		if (len==6)
		{
			buf[i++] = 'O';
			char tmp[3];
			sprintf(tmp, "%02X", light);
			buf[i++] = tmp[0];
			buf[i++] = tmp[1];
		}
		else if (len==8)
		{
			buf[i++] = 'o';
			char tmp[5];
			sprintf(tmp, "%04X", light);
			buf[i++] = tmp[0];buf[i++] = tmp[1];
			buf[i++] = tmp[2];buf[i++] = tmp[3];
		}
		buf[i++] = _end;
		buf[i++] = '\0';
	}
	void GetLight(char* buf, int len)
	{
		int i = 0;
		buf[i++] = _begin;
		buf[i++] = '[';
		if (len==6)
			buf[i++] = 'I';
		else
			buf[i++] = 'i';		
		buf[i++] = _end;
		buf[i++] = '\0';
	}
	void SetSoftLimit(char axis, int pos, char* buf, int len)
	{
		int i = 0;
		buf[i++] = _begin;
		buf[i++] = '&';
		buf[i++] = axis;
		char tmp[32];
		if(pos >= 0)
		{
			sprintf(tmp, "+%08X", abs(pos));
		}
		else
		{
			sprintf(tmp, "-%08X", abs(pos));
		}
		memcpy(buf + i, tmp, strlen(tmp));
		i+= (int)strlen(tmp);
		buf[i++] = _end;
		buf[i++] = '\0';
	}
	virtual void DataArrival(const char* buf, int len);
	void SetAxis(char p_Axis,HAxisDispatcher* p_Dispatch);
private:
	void Parse();
	void Recv(int s, int e);
	HAxisDispatcher* GetAxis(char p_Axis);
private:
	const char _begin;
	const char _end;
	char _cirbuffer[256];
	int _pos_start;
	int _pos_end;
	HAxisDispatcher* _dispatcher;
	HAxisDispatcher *m_XAxis,*m_YAxis,*m_ZAxis,*m_WAxis,*m_OAxis;
};
