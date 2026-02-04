#pragma once
#include "HOpenCVBase.h"
#include "BlockScan.h"

class CCalImgValue:public HCoreProcess
{
public:
	CCalImgValue(void);
	~CCalImgValue(void);

	virtual bool InitProcess(LPVOID p_param=0);//初始化库
	virtual bool UnInitProcess(){return true;};
	//InputPara输入，为了获取某个图像质量评估值;OutPutPara输出返回结果，可能结构图或者数据
	virtual bool ProcessImg(void* InputPara,void* OutPutPara,HVideoHeader* pHeader);
	virtual void SavePic( HVideoHeader* pHeader,CString pName,int pType=0){};
private:
	bool CalCVValue(HVideoHeader* pHeader,LPBYTE pBuffer,CalValueParas* Paras);
	void CalImgValue(CalValueParas* Paras,Mat mImg);
	void GetValue(CalValueParas* Paras,Mat mCalV);	

	void GetMask(HVideoHeader* pHeader,int iAreaPer,int iScale,int iCur);//iAreaPer 代表去面积的最高百分比区域做为做大值，iCur代表第几副图
	void CalMask(cv::Mat mImg,int iAreaPer,int iScale);

	void UseMask(HVideoHeader* pHeader);

	//cv::Mat mCVCalMat;//算出来平均CV值的32F型模板矩阵
	cv::Mat mCurImg;//最新传递进来的用于计算模板的图像
	cv::Mat mBKBinaryImg;//背景图像

	CBlockScan mBolockScan;
	CalCVParas* mCVParas;

	vector<Mat> ReadCVMats;//存储已经记录的CV模板图像
	void ReadExistCVMat();//读取CV模板图像

	int iUseMax;
};

