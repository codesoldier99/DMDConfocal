#pragma once
#include "HOpenCVBase.h"
#include <vector>
using std::vector;

//针对正四方形的图片，非正方形图片还需要再重新设计

class CReBuild:public HCoreProcess,public HVideoRender
{
public:
	CReBuild(void);
	~CReBuild(void);
	//初始化的是ReBuildParas结构体参数
	virtual bool InitProcess(LPVOID p_param=0);//初始化库
	virtual bool UnInitProcess();
	virtual HVideoRender* GetVideoRender() {return this;};//得到视频渲染接口--入口
	//图像从这边传进去，其中pHeader->VnCount;//代表有多少张图合成
	virtual void Renderer(HVideoHeader* pHeader,LPBYTE pBuffer);
	//ProcessImg是去取单张层的图片
	//InputPara，输入图像次序；OutPutPara返回图像总数，pHeader去取图，获取图像，
	virtual bool ProcessImg(void* InputPara,void* OutPutPara,HVideoHeader* pHeader);
	virtual void SavePic( HVideoHeader* pHeader,CString pName,int pType=0);

protected:
	ReBuildParas mParas;
	vector<cv::Mat> mImgs;
	vector<cv::Mat> mGradImgs;
	cv::Mat mGlobeImg;
	cv::Mat mIdxImg;
	cv::Mat mBinaryImg;

private:
	struct OptionMerge
	{
		float fMax;
		int iMaxIdx;
		float fmean;
		float fstd;
	};

	void ReBuildImgs(HVideoHeader* pHeader,LPBYTE pBuffer);
	bool SelectWindows(vector<Mat>& Imgs,vector<Mat>& GradImg,
		Mat GlobeImg,Mat IdxImg,OptionMerge mOptionMerge,int iDev);
	void ResultNotify();//示例，出结果，通过消息模式传出图像结果

	LPBYTE m_HeightBuffer;//高度图缓存
	LPBYTE m_RenderBuffer;//帖图缓存 
	HVideoHeader* m_Header;//图大小 
	bool bNewBegin;
	int iTotal;
};

