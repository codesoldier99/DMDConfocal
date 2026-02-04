#pragma once

//#include <gl\gl.h>
//#include <gl\GLU.h>
#include "gl.h"
#include "GLU.h"
#include "glaux.h"		// Header File For The Glaux Library

class CGlHeight
{
public:
	typedef struct
	{
		int r;
		int g;
		int b;
	}s_LookUpTable;

public:
	CGlHeight(void);
	~CGlHeight(void);
	void DrawObject(void);
	int Height(int X, int Y);
	void SetVertexColor(int x, int y);
	void CopyObject(LPBYTE p_Buffer,int p_Width,int p_Height);
	void LoadFromBmp(LPCTSTR lpszFileName);
	AUX_RGBImageRec* LoadBMPTex(LPCTSTR Filename) /* Loads A Bitmap Image */;

	//bool LoadBMPTex(LPCTSTR Filename,void* mP) /* Loads A Bitmap Image */;

	int LoadGLTextures(LPCTSTR Filename) /* Load Bitmaps And Convert To Textures */;
	void DrawAxial(void);
	void DrawPans(void);

protected:
	int m_MapWidth,m_MapHeight;
	int m_TexWidth,m_TexHeight;
	int m_MapStep;
	BOOL m_bTexture;// «∑Ò”√Ã˘Õº
	LPBYTE m_MapBuffer;
	s_LookUpTable* m_LookUpTable;
	GLuint	texture[1];			// Storage For One Texture ( NEW )

};

