// MutiBuild.cpp : 实现文件
//

#include "stdafx.h"
#include "ImgProcessTaster.h"
#include "MutiBuild.h"
#include "afxdialogex.h"


// CMutiBuild 对话框

IMPLEMENT_DYNAMIC(CMutiBuild, CDialogEx)

CMutiBuild::CMutiBuild(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMutiBuild::IDD, pParent)
{

}

CMutiBuild::~CMutiBuild()
{
}

void CMutiBuild::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMutiBuild, CDialogEx)
END_MESSAGE_MAP()


// CMutiBuild 消息处理程序
