#include "StdAfx.h"
#include "DiffMeasureView.h"
#include "Resource.h"
#include "HGloableFunction.h"

DiffMeasureView::DiffMeasureView(void)
{
  _width = 0;
  _height = 0;
}

bool DiffMeasureView::InitPanel(LPVOID p_Param, LPVOID p_Param2)
{
	m_ConfocalCore=(HConfocalCore*)p_Param;
	//初始化载物台
	if(m_ConfocalCore)
	{
		m_ConfocalCore->GetPlugin(PLUGIN_DIFFMEASURE)->Attach(this);
		m_ConfocalCore->GetPlugin(PLUGIN_DIFFMEASUREPRO)->Attach(this);
	}
	return true;
}

int DiffMeasureView::OnSubjectNotified(IPtnSubject * pSubject,int ID,long wParam,void* pParam,float fParam,void* mParam)
{
	if(ID== NOTIFY_DIFF_MEASURE)
	{
		// 根据获取到的图像A, B, C计算高度图，并显示结果
		if (!pParam || !mParam)// 测试
		{
			if (mParam)
			{
				DiffMeasureParam* dmp = (DiffMeasureParam*)mParam;
				_dmp = *dmp;
			}
			int width = 0;
			int height = 0;
			int wbit = 0;
			LPBYTE mLoadBuf = 0;
			HLoadBmp(&mLoadBuf, width, height, wbit, HGetStartPath() + L"//DiffMeasureResult//defocus_a.bmp");
			if (width > 0 && height > 0)
			{
				_width = width;
				_height = height;
				_imgs[1].resize(_width * _height);
				memcpy(_imgs[1].data(), mLoadBuf, _width * _height);
				free(mLoadBuf);
			}
			mLoadBuf = 0;
			HLoadBmp(&mLoadBuf, width, height, wbit, HGetStartPath() + L"//DiffMeasureResult//defocus_b.bmp");
			if (width > 0 && height > 0)
			{
				_width = width;
				_height = height;
				_imgs[2].resize(_width * _height);
				memcpy(_imgs[2].data(), mLoadBuf, _width * _height);
				free(mLoadBuf);
			}

			_imgs[0].resize(_width * _height);
			memset(_imgs[0].data(), 125, _width * _height);
		}
		else// 复制结果
		{
			DiffMeasureResult* dmr = (DiffMeasureResult*)pParam;
			DiffMeasureParam* dmp = (DiffMeasureParam*)mParam;
			_width = dmr->width;
			_height = dmr->height;
			_imgs[0].resize(_width * _height);
			memcpy(_imgs[0].data(), dmr->a, _width * _height);
			_imgs[1].resize(_width * _height);
			memcpy(_imgs[1].data(), dmr->b, _width * _height);
			_imgs[2].resize(_width * _height);
			memcpy(_imgs[2].data(), dmr->c, _width * _height);
			_dmp = *dmp;
		}

		// 根据公式计算高度图
		_height_img.resize(_width * _height);

		unsigned char* a = _imgs[1].data();
		unsigned char* b = _imgs[2].data();
		double* h = _height_img.data();
		for (int i = 0; i < _height; i++)
		{
		  for (int j = 0; j < _width; j++)
		  {
			int p = i * _width + j;
			double zU = (a[p] - b[p]);// 255.0;
			double zD = (a[p] + b[p]);
			double z = zU / zD;
			if (a[p] + b[p] > 18)
			{ 
				if (fabs(_dmp.coeff_k) > 1e-5)
				{
					z = -(z + _dmp.coeff_b) / _dmp.coeff_k;
				}
				else
				{
					z = -(z + _dmp.coeff_b);
				}
			}
			else
				z = -(0.5 + _dmp.coeff_b) / _dmp.coeff_k;//-1
			//if (z == 1)z = -(1 + _dmp.coeff_b) / _dmp.coeff_k;
			h[p] = z;
		  }
		}
		/*double* h1 = _height_img1.data();*/
		if (_dmp.bFilter)
		{
			medianFilter(h, h, 5, 5);
		}
		
		_panel.SetDiffData(_width, _height, _imgs[0].data(),
		  _imgs[1].data(), _imgs[2].data(), _height_img.data(), _dmp);
	}
	return 0;
}

void DiffMeasureView::medianFilter(double* corrupted, double* smooth, int width, int height)
{

	memcpy(smooth, corrupted, width*height * sizeof(unsigned char));
	for (int j = 1; j < height - 1; j++)
	{
		for (int i = 1; i < width - 1; i++)
		{
			int k = 0;
			double window[9];
			for (int jj = j - 1; jj < j + 2; ++jj)
				for (int ii = i - 1; ii < i + 2; ++ii)
					window[k++] = corrupted[jj * width + ii];
			//   Order elements (only half of them)  
			for (int m = 0; m < 5; ++m)
			{
				int min = m;
				for (int n = m + 1; n < 9; ++n)
					if (window[n] < window[min])
						min = n;
				//   Put found minimum element in its place  
				double temp = window[m];
				window[m] = window[min];
				window[min] = temp;
			}
			smooth[j*width + i] = window[4];
		}
	}
}

void DiffMeasureView::FlushView()
{
}

DiffMeasureView::~DiffMeasureView(void)
{
}

BOOL DiffMeasureView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

BEGIN_MESSAGE_MAP(DiffMeasureView, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_CREATE()
END_MESSAGE_MAP()

BOOL DiffMeasureView::OnEraseBkgnd(CDC* pDC)
{
	return CWnd::OnEraseBkgnd(pDC);
}

void DiffMeasureView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

  if (GetSafeHwnd() && _panel.GetSafeHwnd())
  {
    CRect r;
    GetClientRect(&r);
    _panel.MoveWindow(&r);
  }
}

void DiffMeasureView::Render(LPBYTE pBuffer, int width,int heidth,int wbit )
{
}

void DiffMeasureView::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent==529)//刷新进程
	{
	}
}

void DiffMeasureView::Renderer( HVideoHeader* pHeader,LPBYTE pBuffer )
{
}

int DiffMeasureView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

  _panel.Create(IDD_SIVIEWPANEL, this);
  _panel.ShowWindow(SW_SHOW);
	return 0;
}
