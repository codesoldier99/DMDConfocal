#pragma once
#include "HOpenCVBase.h"
#include <vector>
using std::vector;


class CColorBuild:public HCoreProcess,public HVideoRender
{
public:
	CColorBuild(void);
	~CColorBuild(void);

	virtual bool InitProcess(LPVOID p_param=0);//初始化库
	virtual bool UnInitProcess() {return false;};
	virtual HVideoRender* GetVideoRender() {return this;};//得到视频渲染接口--入口
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);

protected:
	vector<cv::Mat> mImgs;
	cv::Mat mGlobeImg;
	void ReBuildImgs(HVideoHeader* pHeader,LPBYTE pBuffer);

private:
	void ResultNotify();
	//LPBYTE m_RenderBuffer;//颜色图像缓存
};

