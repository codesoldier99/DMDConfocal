

#pragma once

#include <list>
#include "CarlVideo.h"

using namespace std;

class HVideoRenderChain : public HVideoRender,
	public list<HVideoRender*>
{

public:
	void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer)
	{
		list<HVideoRender*>::iterator pos;
		for(pos = begin();pos!=end();++pos)
		{
			if(!*pos)break;
			_ASSERT(*pos);
			(*pos)->Renderer(pHeader,pBuffer);				
		}
	}

	virtual ~HVideoRenderChain() {};

	list<HVideoRender*>::iterator Find(HVideoRender* key)
	{
		list<HVideoRender*>::iterator pos;
		for(pos = begin();pos!=end();++pos)
		{
			if(*pos==key)
			{
				return pos;
			}				
		}
		return pos;
	}

};