#pragma once

#include "IHsmObserver.h"
#include "RenderChain.h"

#define  NOTIFY_RESULT_PIC 5001 //3D扫描输出多层聚焦像
#define  NOTIFY_RESULT_HEIGHT  5002 //3D扫描输出高度图
#define  NOTIFY_RESULT_3D  5003 //3D扫描结束
#define  NOTIFY_COLORBUILD_END 5004	//颜色重组消息
#define  NOTIFY_MAPROISIZE_FLUSH 5005  //地图ROI区域大小消息
#define  NOTIFY_MAPROISTART_FLUSH 5006
#define  NOTIFY_MAPROIEND_FLUSH 5007
#define  NOTIFY_MAP_END 5008 //第二个参数代表文件夹的路径
#define  NOTIFY_CVCal_FLUSH  5009

#define  NOTIFY_3DSCAN_FLUSH  5010 //3D扫图得到最新刷新图  int参数代表现在更新的是第几副图

typedef struct
{
	float x;
	float y;
}CPointF;

typedef enum
{
	PROCESS_3DReBuild,
	PROCESS_ColorBuild,
	PROCESS_MapBuild,
	PROCESS_ImgQuality,
	PROCESS_CalValue,
	PROCESS_FindEdge,
	PROCESS_DACAF
}PROCESS_TYPE;

typedef enum
{
	ImgQuality_CuQuality,
	ImgQuality_XiQuality,
	ImgQuality_CuEnd,
	ImgQuality_CuInvert,
	ImgQuality_XiEnd,
	ImgQuality_XiInvert
}Quality_TYPE;

typedef enum
{
	Scan_Normal,
	Scan_Precision,
	Scan_Multifocus,
	Scan_ZStack
}SCAN_TYPE;

typedef enum
{
	FLIP_NONE,
	FLIP_HORIZONTAL,
	FLIP_VERTICAL,
	FLIP_BOTH
}FLIP_TYPE;

typedef enum
{
	Cal_LightAvg,
	Set_LightAvg,
	Cal_CV
}Cal_TYPE;

typedef struct
{
	HVideoHeader* pCaptureVideo0;
	HVideoHeader* pCaptureVideo1;
	double	coeff_k;
	double	coeff_b;
	bool	bFilter;
}DACAFPARAM;

typedef struct 
{
	long  iScaleV;//尺度缩小
	int iImgSize;//贴图大小，即输出的合成图像大小
	int iIdxSize;//高度图大小
	int iImgStep;//高度图步长
	CString strBKImg;
	float fUpPos,fStepPos,fDownPos;
	float fSpeed;
	bool bSaveImg;
	int iSleep;
	bool bPIZ;
}ReBuildParas;

typedef struct
{
  double	coeff_k;
  double	coeff_b;
  double	defocus;
  double	backlash;
  int		focus_cap_mode;
  int		defocus_cap_mode;
  int		z_ctrl_mode;   //0使用普通Z轴，1使用PI
  bool		bTest;
  bool		bFilter;
  int		iSleep;
}DiffMeasureParam;

struct DiffMeasureResult
{
	DiffMeasureResult()
	{
		width = 0;
		height = 0;
		a = 0;
		b = 0;
		c = 0;
	}
	int width;
	int height;
	unsigned char* a;
	unsigned char* b;
	unsigned char* c;
};

typedef struct 
{
	CPointF StartPt,EndPt;//扫描范围
	CPoint MaxPt,MinPt;//整个载物台移动范围
	int ImageROISize;
	float PixelSize;//单像素多少um
	int iMapSizeX;
	bool bMapSave;
	bool bROIImgSave;
	CString MapSavePath;
	int RowCount;//横纵需要的扫描次数,当成 X？按理 说 X应该是ColCount
	int ColCount;
	int iSleepTime;
	float iScanSpeed;
	SCAN_TYPE mScanType; 

	unsigned int m_rows,m_cols;//行列数 --要取的模板大小，一般取正方形
	int TempleSize;//模板大小，一般等于
	int m_overlap;//相邻图像重叠率（像素）m_rows-TempleSize。
}MapBuildParas;

typedef struct 
{
	double dRGain,dGGain,dBGain;
}AWBParas;

enum Dir//方向定义
{
	none = -1
	,left = 0
	,right = 1
	,top = 2
	,bottom = 3
};

typedef struct 
{
	float UpLimit;
	float DownLimit;
	int ScanCount;
	int PreCount;//预定义次数

	float mDrStep;
	float mCuStep;
	float mXiStep;
	float mCuScale;
	float mXiScale;
	CString strSavePath;
	bool bRecord;
	float iZSpeed;
	int iSleepTime;
	bool bDebug;
}AutoFocusParas;//可以在里面添加选用的调焦特征量模式

typedef struct
{
	CString strBKImg;//背景图像位置
	int iBKErode;//背景图被腐蚀的像素数
	int iUseImg;//用于灰度均衡化的图像数目
}CalCVParas;

typedef struct 
{
	CRect mROIRec;
	double MaxValue;
	double MinValue;
	double MeanValue;
	double SDValue;
	double CVValue;
	int GrayHist[256];
	int iTh;
}CalValueParas;

class HCoreProcess:public IHsmObserver,public IHsmSubject
{
public:
	virtual bool InitProcess(LPVOID p_param=0)=0;//初始化库
	virtual bool UnInitProcess() {return false;};
	virtual HVideoRender* GetVideoRender() {return 0;};//得到视频渲染接口--入口
	virtual bool ProcessImg(void* InputPara,void* OutPutPara,HVideoHeader* pHeader){return false;};//传入数据
	virtual void SavePic( HVideoHeader* pHeader,CString pName,int pType=0){return;};
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0,float fParam=0.0f,
		void* mParam=0) {return 0;};
};

typedef struct 
{
	CString strParas[10];
	float fParas[10];
	int iParas[10];
	bool bDebug;
	bool bParas[10];
}ProcessParas;

class HImgPreProcess
{
public:
	virtual bool InitProcess(LPVOID p_param=0)=0;//初始化库
	virtual bool UnInitProcess() {return false;};
	//virtual bool PreProcessImg(HVideoHeader* pHeader){return false;};//处理图像，并且返回
	virtual bool PreProcessImg(HVideoHeader* pOriginHeader,HVideoHeader* pEndHeader){return false;};//处理图像，并且返回
	virtual void SavePic( HVideoHeader* pHeader,CString pName,int pType=0){return;};
	virtual void SetDebug(bool bV)=0;
};

class HImgProcess
{
public:
	virtual bool InitProcess(LPVOID p_param=0)=0;//初始化库
	virtual bool UnInitProcess() {return false;};
	//virtual bool ProcessImg(HVideoHeader* pHeader,float fPiexlSize,void* OutPutPara=0,void* InputPara=0){return false;};//处理图像，返回Good或者NotGood
	virtual long ProcessImg(HVideoHeader* pOriginHeader,HVideoHeader* pEndHeader,float fPiexlSize,void* OutPutPara=0){return false;};//处理图像，返回Good或者NotGood
	virtual void SavePic( HVideoHeader* pHeader,CString pName,int pType=0){return;};
	virtual void SetDebug(bool bV)=0;
};

class HEasyFunction
{
public:
	virtual bool OpenFile(CString strPath,HVideoHeader* mHeader,int iColor=-2,int iW=0,int iH=0)=0;
	virtual bool SavePic( HVideoHeader* pHeader,CString pName)=0;
  virtual bool ShowImage(HVideoHeader* header, CString title, double scale) = 0;
};

class HCoreProcessMg:public IHsmObserver,public IHsmSubject
{
public:
	virtual HCoreProcess* GetProcessCore(PROCESS_TYPE mType)=0;//{return 0;};
	virtual bool InitMg(void* vParam){return true;};
	virtual bool UnInitMg(){return true;};
	virtual	int	OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam = 0,void* pParam = 0,float fParam=0.0f,
		void* mParam=0) {return 0;};

	//Station Add
	virtual HImgPreProcess* GetImgPreProcess(CString strType)=0;
	virtual HImgProcess* GetImgProcess(CString strType)=0;
	virtual HEasyFunction* GetFunction()=0;
};

