#include "StdAfx.h"
#include "PattenPoint.h"

PattenPoint::PattenPoint(void)
{
	m_FrameCount=0;
	mParas=0;

	m_pData=0;
	m_pData2=0;

	bS=false;
	strSavePath=L"";
}

PattenPoint::~PattenPoint(void)
{
		if (m_pData) free(m_pData);
		if (m_pData2) free(m_pData2);
}

void PattenPoint::Gernerate(void* Paras)
{
	mParas=(PointPara*)Paras;
	if (m_pData) free(m_pData);
	if (m_pData2) free(m_pData2);
	m_pData=(unsigned char*)malloc(Bit2Byte(m_width) * m_height);
	m_pData2=(unsigned char*)malloc(m_width * m_height);
	m_FrameCount=mParas->iPtPeriod*mParas->iPtPeriod;//方阵结构
}

unsigned char* PattenPoint::GetData( int index,int iGray)
{
	if (!mParas) return 0;
	int iAddSize=mParas->iPtExSize;//(double)m_PointWidth/2+0.6;

	if (!m_pData) return 0;
	if (!m_pData2) return 0;
	int i=index;
	int t_count=mParas->iPtPeriod;
	int t_pixs=mParas->iPtSize;
	unsigned char* pOut=m_pData2;

	int t_locx = i % t_count; //得出当前是x轴格子
	int t_locy = (int)(i / t_count);//得出y轴格子

	int t_locx_l = t_locx * t_pixs;//低边
	int t_locx_h = t_locx_l + t_pixs-1+iAddSize;//高边


	int t_locy_l = t_locy * t_pixs;//低边
	int t_locy_h = t_locy_l + t_pixs-1+iAddSize;//高边 


	int temp = t_count *t_pixs;//段距

	BOOL t_locyyes = false;
	for (int y = 0; y < m_height; y++)
	{

		//if (y == (t_locy + temp))
		if (y >= (t_locy + temp)&&y>t_locy_h)
		{
			t_locy += temp;// 下一样方格
			t_locy_l += temp;//低边
			t_locy_h = t_locy_l + t_pixs-1+iAddSize;//高边 
		}
		if ((y <= t_locy_h)&(y>=t_locy_l))
			t_locyyes = true;
		else
			t_locyyes = false;	

		t_locx = i % t_count;//重置x轴
		t_locx_l = t_locx * t_pixs;//低边
		t_locx_h = t_locx_l + t_pixs-1+iAddSize;//高边 

		for (int x = 0; x < m_width; x++)
		{
			//if (x == (t_locx + temp))
			if (x >= (t_locx + temp)&&x>t_locx_h)
			{
				t_locx += temp; //换下一格
				t_locx_l += temp;//低边
				t_locx_h = t_locx_l + t_pixs-1+iAddSize;//高边 
			}
			if ((x>=t_locx_l)&(x <=t_locx_h) & (t_locyyes))//画黑点
				pOut[0] = 255;
			else
				pOut[0] = 0;
			pOut++;
		}
	}
	if (iGray==1)
		ChangeFormate(m_pData2,m_pData);

	if (bS)
	{
		CString name;
		name.Format(L"%s\\BufferPoint(%dX%d)",strSavePath,mParas->iPtSize,mParas->iPtPeriod);
		BuildDirectory(name);
		name.Format(L"%s\\BufferPoint(%dX%d)\\%03d.bmp",strSavePath,mParas->iPtSize,mParas->iPtPeriod,index);
		SaveBmp(m_pData2,m_width,m_height,8,name.GetBuffer());
	}
	
	if (iGray==1)
		return m_pData;
	else
		return m_pData2;
}

int PattenPoint::GetFrameCount()
{
	return m_FrameCount;
}

int PattenPoint::GetMode()
{
	return PATTERN_POINT;
}

void PattenPoint::ChangeFormate( unsigned char* scr,unsigned char* dest )
{
  int cw = Bit2Byte(m_width);
	for (int ih=0;ih<m_height;ih++)
	{
		for (int iw=0;iw<cw;iw++)
		{
			for (int ik=0;ik<8;ik++)
			{
				char vK=0x0;
				if (scr[m_width*ih + iw*8 + ik]>125)
					vK=0x1;
				else
					vK=0x0;
				dest[cw*ih + iw]=(dest[cw*ih + iw] << 1)|vK;
			}
		}
	}
}

bool PattenPoint::SetSave(bool bSave,CString strPath)
{
	bS=bSave;
	if (strPath!="")
	{	
		strSavePath=strPath;
		return true;
	}
	else
		bS=false;
	return true;
}

