#include "StdAfx.h"
#include "GlHeight.h"
#include "HGloableFunction.h"


CGlHeight::CGlHeight(void)
{
	m_MapBuffer=0;
	m_MapWidth=0;
	m_MapHeight=0;
	m_MapStep=16;//先定16
	m_bTexture=FALSE;

	m_LookUpTable=(s_LookUpTable*)malloc(256*sizeof(s_LookUpTable));
	for(int i=0;i<256;i++)//颜色表
	{
		if(i<=85)
		{
			m_LookUpTable[i].r=i;
			m_LookUpTable[i].g=0;
			m_LookUpTable[i].b=0;
		}
		else if(i<=170)
		{
			m_LookUpTable[i].r=0;
			m_LookUpTable[i].g=i-85;
			m_LookUpTable[i].b=0;
		}
		else
		{
			m_LookUpTable[i].r=0;
			m_LookUpTable[i].g=0;
			m_LookUpTable[i].b=i-170;
		}
	}

	//m_bTexture=LoadGLTextures();//先测试载入 默认图像
}


CGlHeight::~CGlHeight(void)
{
	if(m_MapBuffer)
		delete(m_MapBuffer);
	delete(m_LookUpTable);
}


void CGlHeight::CopyObject( LPBYTE p_Buffer,int p_Width,int p_Height )
{
	if (!p_Buffer)
		return;
	m_MapWidth=p_Width;
	m_MapHeight=p_Height;

	if(m_MapBuffer)
	{
		delete(m_MapBuffer);
		m_MapBuffer=(LPBYTE)malloc(p_Width*p_Height);
		memcpy(m_MapBuffer,p_Buffer,p_Width*p_Height);//拷贝
	}

}

void CGlHeight::LoadFromBmp( LPCTSTR lpszFileName )
{
	int wbit;
	if(HLoadBmp(&m_MapBuffer,m_MapWidth,m_MapHeight,wbit,lpszFileName))
	{
		if (wbit!=8)
		{
			delete(m_MapBuffer);
			m_MapBuffer=0;
			m_MapWidth=0;
			m_MapHeight=0;
			return;
		}

	}
}

AUX_RGBImageRec* CGlHeight::LoadBMPTex(LPCTSTR Filename)				// Loads A Bitmap Image
{
	//FILE *File=NULL;									// File Handle
	if (!Filename)										// Make Sure A Filename Was Given
		return NULL;									// If Not Return NULL
	//File=fopen(Filename,"r");							// Check To See If The File Exists
	HANDLE hFile=CreateFile(Filename,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile)											// Does The File Exist?
	{
		CloseHandle(hFile);									// Close The Handle
		return auxDIBImageLoad(Filename);				// Load The Bitmap And Return A Pointer
	}
	return NULL;										// If Load Failed Return NULL
}

//bool CGlHeight::LoadBMPTex(LPCTSTR Filename,void* mP)
//{
//	if (!Filename)										// Make Sure A Filename Was Given
//		return NULL;									// If Not Return NULL
//	HANDLE hFile=CreateFile(Filename,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
//	if (hFile)											// Does The File Exist?
//	{
//		CloseHandle(hFile);									// Close The Handle
//		//mP=auxDIBImageLoad(Filename);//解析不了
//		return true;				// Load The Bitmap And Return A Pointer
//	}
//	return false;	
//}

int CGlHeight::LoadGLTextures(LPCTSTR Filename)									// Load Bitmaps And Convert To Textures
{
	int Status=FALSE;									// Status Indicator

	AUX_RGBImageRec *TextureImage[1];					// Create Storage Space For The Texture

	memset(TextureImage,0,sizeof(void *)*1);           	// Set The Pointer To NULL

	//Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
	if (TextureImage[0]=LoadBMPTex(Filename))//L"F://Tim.bmp"))
	{
		Status=TRUE;									// Set The Status To TRUE
		m_TexHeight=TextureImage[0]->sizeY;
		m_TexWidth=TextureImage[0]->sizeX;//记录贴图的大小

		glGenTextures(1, &texture[0]);					// Create The Texture

		// Typical Texture Generation Using Data From The Bitmap
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}

	if (TextureImage[0])									// If Texture Exists
	{
		if (TextureImage[0]->data)							// If Texture Image Exists
		{
			free(TextureImage[0]->data);					// Free The Texture Image Memory
		}

		free(TextureImage[0]);								// Free The Image Structure
	}
	m_bTexture=Status;
	return Status;										// Return The Status
}

int CGlHeight::Height(int X, int Y)				// This Returns The Height From A Height Map Index
{
	int x = X % m_MapWidth;								// Error Check Our x Value
	int y = Y % m_MapHeight;								// Error Check Our y Value

	if(!m_MapBuffer) return 0;							// Make Sure Our Data Is Valid

	return m_MapBuffer[x + (y * m_MapWidth)]*2;				// Index Into Our Height Array And Return The Height
}

void CGlHeight::SetVertexColor(int x, int y)		// Sets The Color Value For A Particular Index, Depending On The Height Index
{
	if(!m_MapBuffer) return;								// Make Sure Our Height Data Is Valid

	if(m_bTexture)//贴图
	{
		glTexCoord2f((float)x/(float)m_MapWidth,(float)y/(float)m_MapHeight);
		//glTexCoord2f(0,1);
	}
	else//常规用等高线绘
	{
		int index=Height(x, y );	
		glColor3f(m_LookUpTable[index].r/255.0, m_LookUpTable[index].g/255.0, m_LookUpTable[index].b/255.0 );
	}
}

void CGlHeight::DrawAxial(void)
{


	glBegin(GL_LINES);

	glColor4f(0.0,1.0,0.0,0.5);

	int axis[3]={0,0,0};

	int AxisHeight=256,AxisWidth=m_MapWidth;

		int lenth=AxisHeight;
		int step=AxisWidth/8;
		for(int j=0;j<9;j++)
		{
			glVertex3i(axis[0],0,0);
			
			//axis[1]+=128;
			glVertex3i(axis[0],lenth,0);
			axis[0]+=step;
		}

		axis[0]=0;
		lenth=AxisWidth;
		step=AxisHeight/8;
		for(int j=0;j<9;j++)
		{
			glVertex3i(0,axis[0],0);
			
			//axis[1]+=128;
			glVertex3i(lenth,axis[0],0);
			axis[0]+=step;
		}//xy轴


		axis[0]=0;
		lenth=AxisHeight;
		step=AxisWidth/8;
		for(int j=0;j<9;j++)
		{
			glVertex3i(0,0,axis[0]);

			//axis[1]+=128;
			glVertex3i(0,lenth,axis[0]);
			axis[0]+=step;
		}

		axis[0]=0;
		lenth=AxisWidth;
		step=AxisHeight/8;
		for(int j=0;j<9;j++)
		{
			glVertex3i(0,axis[0],0);
			
			//axis[1]+=128;
			glVertex3i(0,axis[0],lenth);
			axis[0]+=step;
		}//yz轴

		//下面是底轴
		axis[0]=0;
		lenth=AxisWidth;
		step=AxisWidth/8;
		for(int j=0;j<9;j++)
		{
			glVertex3i(0,0,axis[0]);

			//axis[1]+=128;
			glVertex3i(lenth,0,axis[0]);
			axis[0]+=step;
		}

		axis[0]=0;
		lenth=AxisWidth;
		step=AxisWidth/8;
		for(int j=0;j<9;j++)
		{
			glVertex3i(axis[0],0,0);

			//axis[1]+=128;
			glVertex3i(axis[0],0,lenth);
			axis[0]+=step;
		}//xz轴



// 	glVertex3i(0, 0, 0);
// 	glVertex3i(0, 0, 1024);//z轴
// 	glVertex3i(0, 0, 0);
// 	glVertex3i(0, 1024,0);//y轴
// 	glVertex3i(0, 0, 0);
// 	glVertex3i(1024,0,0);//x轴

	glEnd();
}

void CGlHeight::DrawPans(void)
{
	glBegin(GL_QUADS);

	glColor4f(0,1,0,0.3);

	glVertex3i(512, 0, 0);
	glVertex3i(512,128,0);
	glVertex3i(512,128,1024);
	glVertex3i(512,0,1024);

	glEnd();
}

void CGlHeight::DrawObject( void )
{

	int X,Y,x,y,z;
//	GLfloat texX,texY;//贴图的坐标
	if(!m_MapBuffer) return;								// Make Sure Our Height Data Is Valid

	//glLoadIdentity();//重置矩阵
	glEnable(GL_BLEND);//开启alpha通道

	

	//下面画坐标
	glDisable(GL_TEXTURE_2D);
	DrawAxial();//画坐标轴
	//DrawPans();//画面板
	glEnable(GL_TEXTURE_2D);

	

	//下面画物体
	if(m_bTexture)
		glBindTexture(GL_TEXTURE_2D, texture[0]);//要在begin之前哦!
 	glColor3f(1,1,1);

// 	glBegin(GL_QUADS);
// 	// Front Face
// 	glTexCoord2f(0.0f, 0.0f); glVertex3f(-100.0f, -100.0f, -100.0f);
// 	glTexCoord2f(1.0f, 0.0f); glVertex3f(-100.0f, -100.0f,  100.0f);
// 	glTexCoord2f(1.0f, 1.0f); glVertex3f(-100.0f,  100.0f,  100.0f);
// 	glTexCoord2f(0.0f, 1.0f); glVertex3f(-100.0f,  100.0f, -100.0f);
// 
// 	glEnd();
// 	DrawAxial();//画坐标轴
// 	return;

	glBegin( GL_QUADS );							// Render Polygons

	//glBegin( GL_LINES );							// Render Lines Instead

	for (X = 0; X < (m_MapWidth-m_MapStep); X += m_MapStep )
		for ( Y = 0; Y < (m_MapWidth-m_MapStep); Y += m_MapStep )
		{
			

			// Get The (X, Y, Z) Value For The Bottom Left Vertex
			x = X;							
			y = Height(X, Y );	
			z = Y;							



			//glTexCoord2f(texX,texY);
			// Set The Color Value Of The Current Vertex
			SetVertexColor( x, z);

			glVertex3i(x, y, z);						// Send This Vertex To OpenGL To Be Rendered (Integer Points Are Faster)

			// Get The (X, Y, Z) Value For The Top Left Vertex
			x = X;										
			y = Height(X, Y + m_MapStep );  
			z = Y + m_MapStep ;							

			// Set The Color Value Of The Current Vertex
			SetVertexColor(x, z);

			glVertex3i(x, y, z);						// Send This Vertex To OpenGL To Be Rendered

			// Get The (X, Y, Z) Value For The Top Right Vertex
			x = X + m_MapStep; 
			y = Height(X + m_MapStep, Y + m_MapStep ); 
			z = Y + m_MapStep ;

			// Set The Color Value Of The Current Vertex
			SetVertexColor(x, z);

			glVertex3i(x, y, z);						// Send This Vertex To OpenGL To Be Rendered

			// Get The (X, Y, Z) Value For The Bottom Right Vertex
			x = X + m_MapStep; 
			y = Height(X + m_MapStep, Y ); 
			z = Y;

			// Set The Color Value Of The Current Vertex
			SetVertexColor( x, z);

			glVertex3i(x, y, z);						// Send This Vertex To OpenGL To Be Rendered
		}
		glEnd();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);					// Reset The Color

	//glFlush();
}




