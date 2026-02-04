#pragma once
#include "HOpenCVBase.h"

class CMapBuild:public HCoreProcess
{

public:
	CMapBuild(void);
	~CMapBuild(void);

	virtual bool InitProcess(LPVOID p_param=0);//初始化库
	virtual bool UnInitProcess();
	//InputPara指向抓图的X、Y方向计数CPonit,OutPutPara指向总共需要合多少副图
	virtual bool ProcessImg(void* InputPara,void* OutPutPara,HVideoHeader* pHeader);

private:
	MapBuildParas* mMapBuildParas;
	CString strSaveFile,strROIPath,strImgPath;

	CPoint mThumStartPt;//缩略图的像素起点
	HVideoHeader m_MapData,m_ROIData,m_ThumbData,m_MapNarrow;//图像数据

	float m_MoveStep;//单步移动的距离
	void ResetParas();//重新计算

	CPoint GetROIMapPoint( int pXCount,int pYCount);
	CPoint GetMapPoint( int pXCount,int pYCount);
	void GetImgROI( HVideoHeader* pScr,HVideoHeader* pDest );
	void GetImgThumb( HVideoHeader* pScr,HVideoHeader* pDest);
	bool CopyROIImg( HVideoHeader* pScr,HVideoHeader* pDest,CPoint tPoint);
	bool BuildDirectory(CString strPath);

	//拼接参数
	Dir _dr;
	void GetImgROI2( HVideoHeader* pScr,HVideoHeader* pDest,CPoint pt);
	void CalSearchRect(Rect& srcRect,Rect& dstRect);
	bool GetMatchShift(HVideoHeader* pHeader,CPoint pt);
	int iShiftX,iShiftY;//匹配完的XY偏移量
	int iPreShiftX,iPreShiftY;//前一个偏移
	Mat mPreImg;//前一副图像
	CPoint mPrePt;//前一个坐标点
	bool bNewProcess;
};

