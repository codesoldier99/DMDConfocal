// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 IMGPROCESS_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// IMGPROCESS_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。

#ifndef IMGPROCESS_H
#define IMGPROCESS_H


struct PixelRgb
{
	BYTE Red;
	BYTE Green;
	BYTE Blue;
};

typedef struct MAXVALUE{
	int iDist;
	int iAngle;
	int iMax;
}MAXVALUE;

 #define PI 3.1415927

struct ObjectInfo
{
	int sn;
	int left,top,right,bottom;
	int width,height;
	long cx,cy;
	long pixelCount;
};

struct mRECT
{
	long left;//矩形左上角横坐标

	long top;//矩形左上角纵坐标

	long right;//矩形右下角横坐标

	long bottom;//矩形右下角纵坐标

};

struct mSize
{
	int x;
	int y;
};

 bool LoadBmp(LPBYTE* pBuffer,long& IWidth,
	long& IHeight, int& wBitsPerPixel, LPCTSTR lpszFileName);

  bool SaveBmp(LPBYTE pBuffer,long IWidth,
 	long IHeight, int wBitsPerPixel, LPCTSTR lpszFileName);

  void Img8_Thresh2( LPBYTE pBuffer,long IWidth,long IHeight,int Thresh );

  void Img8_And(LPBYTE pDest,LPBYTE pTemplate,long IWidth,long IHeight);

  void RGB2Gray(LPBYTE pBuffer,long IWidth,long IHeight);
 
  void RGB2Gray8( LPBYTE& pBuffer,long IWidth,long IHeight );
 
  void Img24_Sharp(LPBYTE pSource,long IWidth,long IHeight);
 
  void Img24_Sobel(LPBYTE pSource,long IWidth,long IHeight,float cof);

  //void ChangeFormate( unsigned char* scr,unsigned char* dest );

  void Img8_Add(LPBYTE pDest,LPBYTE pScr,long IWidth,long IHeight,int Thresh);

  void Img8_Max(LPBYTE pDest,LPBYTE pScr,long IWidth,long IHeight,int Thresh=0);

  class CImgProcess {
public:
	CImgProcess(void);
	// TODO: 在此添加您的方法。
};


#endif
