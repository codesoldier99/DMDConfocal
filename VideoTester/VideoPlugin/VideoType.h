#pragma once
#include "stdafx.h"
#include <vector>
#include "CarlVideo.h"


class CVideoType : public HVideoType
{
public:
	CVideoType(void);
	~CVideoType(void);
	virtual int FindCameraType();
	virtual int GetTypeCount();
	virtual VideoTypeParas* GetCameraType(int pIndex);
	virtual VideoTypeParas* GetCameraType(CString mCameraType);

private:
	std::vector<VideoTypeParas>  mCameraTypes;
	bool OpenVideoLib(CString pPath,VideoTypeParas* mVideoType);
};

