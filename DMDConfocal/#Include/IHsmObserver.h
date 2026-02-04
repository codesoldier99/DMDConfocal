#pragma once


#include <Windows.h>
#include <set>



class IPtnSubject;

//观察者参考接口
class IHsmObserver
{
public:

	//响应观察主体的通知		
	//外部观察者实现类重载后以捕获观察对象消息响应.
	//通过pSubject区分多个观察对象		
	virtual int //由观察对象实现类约定。
		OnSubjectNotified(
		IPtnSubject * pSubject, //发出通知的观察主体
		int ID, //消息ID
		long wParam = 0, //参数一
		void* pParam = 0,//参数二
		float fParam=0.0f,
		void* mParam=0
		)=0;

	virtual ~IHsmObserver(){}

};

//观察对象参考接口
class IPtnSubject
{
public:

	virtual ~IPtnSubject(){}

	//绑定一个观察者		
	//不能重复绑定同一个对象		
	virtual int //0:成功，其他:失败
		Attach(
		IHsmObserver * pObserver//[in]观察者指针,不能为NULL
		)=0;

	//解除观察者的绑定
	virtual int //0:成功，其他:失败
		Detach(
		IHsmObserver * pObserver//[in]观察者指针,如果为NULL则解除所有观察者的绑定
		)=0;

	//向观察者发出提示
	//观察者通过pSubject区分多个观察对象
	virtual int Notify(
		IPtnSubject * pSubject,//提示观察者的观察对象指针
		int ID,//信息ID
		long wParam = 0 ,//参数一
		void* pParam = 0 ,//参数二
		float fParam=0.0f,
		void* mParam=0
		)=0;

};

using namespace std;
typedef set<IHsmObserver *> IHsmObserverList;

class IHsmSubject:public IPtnSubject
{
public:
	IHsmSubject(void)
	{
		::InitializeCriticalSection(&m_cs);
	}

	~IHsmSubject(void)
	{
		::DeleteCriticalSection(&m_cs);
	}
public:
	virtual int Attach(IHsmObserver * pObserver)
	{
		EnterCriticalSection(&m_cs);
		try
		{		
			if(pObserver!=NULL&&m_observers.find(pObserver)==m_observers.end())
			{
				m_observers.insert(pObserver);
			}
		}
		catch(...)
		{
		}
		LeaveCriticalSection(&m_cs);
		return 0;
	}

	virtual int Detach(IHsmObserver * pObserver)
	{
		EnterCriticalSection(&m_cs);
		if(pObserver==NULL)				
			m_observers.clear();				
		else
			m_observers.erase(pObserver);
		LeaveCriticalSection(&m_cs);
		return 0;
	}


	virtual int Notify(IPtnSubject * pSubject,
		int ID,
		long wParam = 0 ,
		void* pParam = 0,
		float fParam=0.0f,
		void* mParam=0)
	{
		int res = 0;
		EnterCriticalSection(&m_cs);
		IHsmObserverList::iterator pos; 		
		for(pos=m_observers.begin();pos!=m_observers.end();++pos)
		{
			res |= (*pos)->OnSubjectNotified(pSubject, ID, wParam, pParam,fParam,mParam);
		}
		LeaveCriticalSection(&m_cs);
		return res;

	}
protected:
	CRITICAL_SECTION m_cs;
	IHsmObserverList m_observers;
};


