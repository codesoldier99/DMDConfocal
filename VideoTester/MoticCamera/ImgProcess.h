
#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H




struct mRECT
{
	long left;//矩形左上角横坐标

	long top;//矩形左上角纵坐标

	long right;//矩形右下角横坐标

	long bottom;//矩形右下角纵坐标

};


bool Cam_AutoAWB(LPBYTE pBuffer,long IWidth, long IHeight,double& iRed,double& iGreen,double& iBlue);
void selectWhiteBlance(LPBYTE pBuffer,RECT pRect,int iWidth, int iHeight,double &kR,double &kG,double &kB);
int SmoothMax( float* pValue, int w, int h, int matrix, float* max );
float GetSmooth(int w,int h,const RECT& r,unsigned char *pData);





#endif