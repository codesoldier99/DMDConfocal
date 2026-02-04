#include "StdAfx.h"
#include "MoticGearProto.h"

MoticGearProto::MoticGearProto(void):_begin('*'),_end('$')
{
	_pos_start = 0;
	_pos_end = 0;
	_dispatcher = 0;
	m_XAxis=0;
	m_YAxis=0;
	m_ZAxis=0;
	m_WAxis=0;
	m_OAxis=0;
}

MoticGearProto::~MoticGearProto(void)
{

}

void MoticGearProto::DataArrival(const char* buf, int len)
{
	const int cirlen = sizeof(_cirbuffer);
	int length = _pos_end - _pos_start;

	if(length + len > cirlen)
	{
		len = cirlen - length;
		if(len == 0)
		{
			Parse();
			return;
		}
	}
	int e = _pos_end % cirlen;
	int delta = e + len - cirlen;
	if(delta <= 0)
	{
		memcpy(_cirbuffer + e, buf, len);
	}
	else
	{
		memcpy(_cirbuffer + e, buf, cirlen - e);
		memcpy(_cirbuffer, buf + cirlen - e, delta);
	}
	_pos_end += len;
	Parse();
}

void MoticGearProto::Parse()
{
	int s(-1), e(-1);
	int tmp(-1);
	int k;
	for(int i = _pos_start; i < _pos_end; i++)
	{
		k = i % sizeof(_cirbuffer);
		if(_cirbuffer[k] == _begin)
		{
			if(s != -1)
			{
				tmp = i;
			}
			s = i;
		}
		else if(_cirbuffer[k] == _end)
		{
			if(s != -1)
			{
				e = i;
			}
			tmp = i;
		}
		else if(s == -1)
		{
			tmp = i;
		}

		if(s != -1 && e != -1)
		{
			Recv(s + 1, e);
			s = -1;
			e = -1;
		}
	}

	if(tmp != -1)
	{
		_pos_start = tmp + 1;
	}
}

void MoticGearProto::Recv(int s, int e)
{
	char buf[64];
	int len = e - s;
	if(len == 0)return;

	s = s % sizeof(_cirbuffer);
	e = e % sizeof(_cirbuffer);

	if(s < e)
	{
		memcpy(buf, _cirbuffer + s, len);
	}
	else if(s > e)
	{
		memcpy(buf, _cirbuffer + s, sizeof(_cirbuffer) - s);
		memcpy(buf + sizeof(_cirbuffer) - s, _cirbuffer, len - sizeof(_cirbuffer) + s);
	}
	else
	{
		return;
	}

	buf[len] = '\0';

	switch(buf[0])
	{
	case '=':
		_dispatcher=GetAxis('X');
		if(_dispatcher)_dispatcher->Event_CommTest();
		break;
	case '^':
		if(len == 10)
		{
			char axis = buf[1];
			int pos;
			sscanf(buf + 2, "%08X", &pos);
			_dispatcher=GetAxis(buf[1]);
			if(_dispatcher)_dispatcher->Event_GetPosition(pos);
			
		}
		break;
	case '(':
		if(len == 10)
		{
			char axis = buf[1];
			int pos;
			sscanf(buf + 2, "%08X", &pos);
			//TRACE("Moved Axis %c, Pos %d\n", axis, pos);
			_dispatcher=GetAxis(buf[1]);
			if(_dispatcher)_dispatcher->Event_Moved(pos);
		}
		break;
	case '!':
		if(len == 3 && buf[2] == 'W')
		{
			//TRACE("Wrong Axis %c moving\n", buf[1]);
			_dispatcher=GetAxis(buf[1]);
			if(_dispatcher)_dispatcher->Event_Error();
		}
		break;
	case '@':
		if(len == 2)
		{
			if(buf[1] == 'K')//物镜复位完成收到[*@K$]
			{
				_dispatcher=GetAxis('W');
				if(_dispatcher)_dispatcher->Event_Reset();
			}
			else
			{
				char axis = 'X' + buf[1] - 'H';
				//TRACE("Axis %c Reseted\n", axis);
				_dispatcher=GetAxis(axis);
				if(_dispatcher)_dispatcher->Event_Reset();
			}
		}
		break;
	case '|':
		if(len == 3)
		{
			char axis = buf[1];
			int i = buf[2] - '0';
			if(i == 0)
			{
				//TRACE("Axis Static\n");
				_dispatcher=GetAxis(buf[1]);
				if(_dispatcher)_dispatcher->Event_IsMoving(false);
			}
			else if(i == 1)
			{
				//TRACE("Axis Moving\n");
				_dispatcher=GetAxis(buf[1]);
				if(_dispatcher)_dispatcher->Event_IsMoving(true);
			}
		}
		break;
	case '[':
		if(len == 2)
		{
			switch(buf[1])
			{
			case 'C':
				//if(_dispatcher)_dispatcher->Event_CaptureImage();
				break;
			case 'S':
				//if(_dispatcher)_dispatcher->Event_FastScan();
				break;
			case 'A':
				//if(_dispatcher)_dispatcher->Event_AutoFocus();
				break;
			case '+':
				//if(_dispatcher)_dispatcher->Event_JoystickObjChanged(true);
				break;
			case '-':
				//if(_dispatcher)_dispatcher->Event_JoystickObjChanged(false);
				break;
			}
		}
		else if(len == 3)
		{
			if(((buf[1] >= '0' && buf[1] <= '9') || (buf[1] >= 'A' || buf[1] <= 'F')) &&
				((buf[2] >= '0' && buf[2] <= '9') || (buf[2] >= 'A' || buf[2] <= 'F')))
			{
				int axiscount;
				sscanf_s(buf + 1, "%02X", &axiscount);
				//if(_dispatcher)_dispatcher->Event_GetAxisCount(axiscount);
			}
		}
		else if(len == 4)
		{
			_dispatcher=GetAxis('O');
			if(buf[1] == 'L')
			{
				int pos;
				sscanf(buf + 2, "%02X", &pos);
				if(_dispatcher)_dispatcher->Event_LightChanged(pos);
			}
		}
		else if (len == 6)
		{
			_dispatcher=GetAxis('O');
			if(buf[1] == 'L')
			{
				int pos;
				sscanf(buf + 2, "%04X", &pos);
				if(_dispatcher)_dispatcher->Event_LightChanged(pos);
			}
		}
		break;
	default:;
		//TRACE(buf);
		//TRACE("\n");
	}
}

HAxisDispatcher* MoticGearProto::GetAxis( char p_Axis )
{
	switch(p_Axis)
	{
	case 'X':
		return m_XAxis;
		break;
	case 'Y':
		return m_YAxis;
		break;
	case 'Z':
		return m_ZAxis;
		break;
	case 'W':
		return m_WAxis;
		break;
	case 'O':
		return m_OAxis;
	}
	return 0;
}

void MoticGearProto::SetAxis( char p_Axis,HAxisDispatcher* p_Dispatch )
{
	switch(p_Axis)
	{
	case 'X':
		m_XAxis=p_Dispatch;
		break;
	case 'Y':
		m_YAxis=p_Dispatch;
		break;
	case 'Z':
		m_ZAxis=p_Dispatch;
		break;
	case 'W':
		m_WAxis=p_Dispatch;
		break;
	case 'O':
		m_OAxis=p_Dispatch;
	}
}
