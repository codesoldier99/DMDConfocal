// MsgViewBox.cpp : 实现文件
//

#include "stdafx.h"
#include "ConfocalUILib.h"
#include "MsgViewBox.h"
#include "afxdialogex.h"


// CMsgViewBox 对话框

IMPLEMENT_DYNAMIC(CMsgViewBox, CDialogEx)

CMsgViewBox::CMsgViewBox(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMsgViewBox::IDD, pParent)
	, m_StatusStr(_T("Status: Undefined"))
	, m_TimeStr(_T("TimeUse: 0.00 sec"))
	, m_InfoStr(_T("Info: Undefined"))
{

}

CMsgViewBox::~CMsgViewBox()
{
}

bool CMsgViewBox::InitPanel(LPVOID p_Param, LPVOID p_Param2)
{
	fCount=0;
	return true;
}

void CMsgViewBox::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_StatusView, m_StatusStr);
	DDX_Text(pDX, IDC_TimeView, m_TimeStr);
	DDX_Text(pDX, IDC_InfoStatic, m_InfoStr);
}

int CMsgViewBox::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if (ID==NOTIFY_MSGVIEW_SHOW)
	{
		m_StatusStr=*(CString*)pParam;
		//GetDlgItem(IDC_StatusView)->SetWindowTextW(*(CString*)pParam);
		//UpdateData(FALSE);
	}
	else if (ID==NOTIFY_MSGVIEW_Info)
	{
		m_InfoStr=*(CString*)pParam;
	}
	else if (ID==NOTIFY_MSGVIEW_END)
	{
		if (pParam == 0)
		{
			m_StatusStr = L"Status:   Finished！";
		}
		else
		{
			m_StatusStr = *(CString*)pParam;
		}
		Sleep(100);
		KillTimer(110);
		//CDialogEx::OnCancel();
	}
	return 0;
}


BEGIN_MESSAGE_MAP(CMsgViewBox, CDialogEx)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_ENDBtn, &CMsgViewBox::OnBnClickedEndbtn)
END_MESSAGE_MAP()


// CMsgViewBox 消息处理程序


void CMsgViewBox::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	fCount++;
	m_TimeStr.Format(L"TimeUsed:   %.01f sec",fCount*0.1);
	UpdateData(FALSE);
	__super::OnTimer(nIDEvent);
}



void CMsgViewBox::OnBnClickedEndbtn()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


BOOL CMsgViewBox::OnInitDialog()
{
	__super::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetTimer(110,100,NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CMsgViewBox::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类
	KillTimer(110);
	__super::OnCancel();
}
