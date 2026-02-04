#include "StdAfx.h"
#include "FindCell.h"
#include "GraphUtils.h"
#include "XscFitLines.h"

CFindCell::CFindCell(void)
{
}

CFindCell::~CFindCell(void)
{
}

bool CFindCell::InitProcess(LPVOID p_param)
{
	if (p_param!=0)
	{
		return true;
	}
	return false;
}

bool CFindCell::UnInitProcess()
{

	return true;
}

bool CFindCell::ProcessImg(void* InputPara,void* OutPutPara,HVideoHeader* pHeader)
{
	return true;
}

void CFindCell::SavePic(HVideoHeader* pHeader, CString pName, int pType)
{
	SaveImage(pHeader, pName, FLIP_VERTICAL);
}





