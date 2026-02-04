
#pragma  once

typedef struct 
{
	int iPtSize;
	int iPtPeriod;
	int iPtExSize;
}PointPara;

typedef struct 
{
	int iLineW;
	int iLPeriod;
	int iLExW;
	int iLType;
}LinePara;

namespace WlpDMDControl
{
	class IPatternGenerate
	{
		#define PATTERN_LINE 1
		#define PATTERN_POINT 2

		#define PATTERN_LINE_HORIZ 0
		#define PATTERN_LINE_VERTIAL 1

		public:
			virtual ~IPatternGenerate(){};
			virtual void Gernerate(void* Paras)=0;
			virtual unsigned char* GetData(int index,int iGray)=0;//得到当前数据
			virtual int GetFrameCount()=0;//得到这个模式下的总帧数
			virtual int GetMode()=0;//得到这个模式类型
			virtual bool SetSave(bool bSave,CString strPath)=0;
      virtual int SetSize(int width, int height) = 0;
      virtual void GetSize(int* width, int* height) = 0;
	};

}