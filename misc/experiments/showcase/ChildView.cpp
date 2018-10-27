#include "stdafx.h"
#include "ChildView.h"
#include "resource.h"

#include "../common/ellipse.h"
#include "../common/paths.h"

#include <agge/blenders_simd.h>
#include <agge/filling_rules.h>
#include <agge/renderer.h>
#include <agge/stroke_features.h>
#include <samples/common/shell.h>

#include <memory>
#include <functional>
#include <algorithm>

using namespace agge;
using namespace std;
using namespace Gdiplus;
using namespace common;

class __declspec(uuid("ec5ec8a9-c395-4314-9c77-54d7a935ff70")) _IWICImagingFactory;
extern "C" const IID IID_IWICImagingFactory = __uuidof(_IWICImagingFactory);

namespace common
{
	typedef std::pair< std::vector<Gdiplus::PointF>, std::vector<BYTE> > GdipPath;

	void pathStart(HDC /*hdc*/)
	{	}

	void pathMoveTo(HDC hdc, real_t x, real_t y)
	{	::MoveToEx(hdc, static_cast<int>(x), static_cast<int>(y), NULL);	}
	
	void pathLineTo(HDC hdc, real_t x, real_t y)
	{	::LineTo(hdc, static_cast<int>(x), static_cast<int>(y));	}

	void pathEnd(HDC /*hdc*/)
	{	/*::EndPath(hdc);*/	}


	void pathStart(GdipPath &/*path*/)
	{	}

	void pathMoveTo(GdipPath &path, real_t x, real_t y)
	{
		path.first.push_back(PointF(x, y));
		path.second.push_back(PathPointTypeStart);
	}
	
	void pathLineTo(GdipPath &path, real_t x, real_t y)
	{
		path.first.push_back(PointF(x, y));
		path.second.push_back(PathPointTypeLine);
	}

	void pathEnd(GdipPath &/*path*/)
	{	}


	void pathStart(ID2D1GeometrySink &/*path*/)
	{	}

	void pathMoveTo(ID2D1GeometrySink &path, real_t x, real_t y)
	{
		D2D1_POINT_2F p = { x, y };

		path.BeginFigure(p, D2D1_FIGURE_BEGIN_HOLLOW);
	}
	
	void pathLineTo(ID2D1GeometrySink &path, real_t x, real_t y)
	{
		D2D1_POINT_2F p = { x, y };

		path.AddLine(p);
	}

	void pathEnd(ID2D1GeometrySink &path)
	{	path.EndFigure(D2D1_FIGURE_END_OPEN);	}
}

namespace
{
	int random(unsigned __int64 upper_bound)
	{	return static_cast<unsigned>(upper_bound * rand() / RAND_MAX);}

	bar generate(int previous_close)
	{
		bar b = {	previous_close + random(14) - 7	};

		b.h = b.o + random(20);
		b.l = b.o - random(20);
		b.c = b.l + random(b.h - b.l);
		return b;
	};
}

CChildView::CChildView()
	: _drawLines(false), _drawBars(true), _drawEllipses(false), _drawSpiral(false),
		_mode(bufferDIB), _status_bar(0), _drawMode(dmodeGDI), _onpaint_timing(0),
		_fill_timing(0), _drawing_timing(0), _blit_timing(0), _averaging_index(0),
		_agg_bitmap(1, 1), _renderer(4)
{
	::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_factory);
	const D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), 0, 0, D2D1_RENDER_TARGET_USAGE_NONE,
		D2D1_FEATURE_LEVEL_DEFAULT);

	_factory->CreateDCRenderTarget(&properties, &_rtarget);

	D2D1_COLOR_F clr = { 0, 0, 0, 1.0f };

	_rtarget->CreateSolidColorBrush(clr, &_brush);
//	clr.a = 0.36f;
	_rtarget->CreateSolidColorBrush(clr, &_brushTick);

	CComPtr<ID2D1BitmapRenderTarget> rtargetMemory;
	CComPtr<ID2D1Bitmap> pattern;
	D2D1_SIZE_F patternSize = { 15, 15 };
	D2D1_RECT_F pixelRect = { 0, 0, 1, 1 };

	_rtarget->CreateCompatibleRenderTarget(patternSize, &rtargetMemory);
	rtargetMemory->BeginDraw();
	clr.a = 1, clr.r = 240 / 255.0f, clr.g = 1, clr.b = 1;
	rtargetMemory->Clear(clr);
	rtargetMemory->FillRectangle(pixelRect, _brush);
	rtargetMemory->EndDraw();
	rtargetMemory->GetBitmap(&pattern);

	D2D1_BITMAP_BRUSH_PROPERTIES bbprops = {
		D2D1_EXTEND_MODE_WRAP,
		D2D1_EXTEND_MODE_WRAP,
		D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
	};

	_rtarget->CreateBitmapBrush(pattern, bbprops, &_brushBackground);

	srand((unsigned)time(0));

	for (int bucket = 0; bucket < 20; ++bucket)
	{
		bar b = { 0 };

		for (int i = 0; i < 370; ++i)
			_bars[bucket].push_back(b = generate(b.c));
	}

	for (int n = 1500; n; --n)
	{
		RECT r = { random(1780), random(890), 0, 0 };
		
		r.right = r.left + random(140) + 1, r.bottom = r.top + random(140) + 1;
		
		COLORREF c = RGB(random(255), random(255), random(255));

		_ellipses.push_back(make_pair(r, c));
	}
	//RECT r1 = { 10, 10, 999, 850 };
	//RECT r2 = { 10, 10, 1900, 1000 };
	//RECT r3 = { 600, 400, 1900, 1000 };
	//_ellipses.push_back(make_pair(r1, RGB(23, 190, 250)));
	//_ellipses.push_back(make_pair(r2, RGB(23, 23, 250)));
	//_ellipses.push_back(make_pair(r3, RGB(255, 30, 10)));
}

CChildView::~CChildView()
{	}

BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()

	ON_COMMAND(ID_PRIMITIVES_LINES, &CChildView::OnSetPrimitiveLines)
	ON_UPDATE_COMMAND_UI(ID_PRIMITIVES_LINES, &CChildView::OnUpdateMenuPrimitiveLines)
	ON_COMMAND(ID_PRIMITIVES_BARS, &CChildView::OnSetPrimitiveBars)
	ON_UPDATE_COMMAND_UI(ID_PRIMITIVES_BARS, &CChildView::OnUpdateMenuPrimitiveBars)
	ON_COMMAND(ID_PRIMITIVES_ELLIPSES, &CChildView::OnSetPrimitiveEllipses)
	ON_UPDATE_COMMAND_UI(ID_PRIMITIVES_ELLIPSES, &CChildView::OnUpdateMenuPrimitiveEllipses)
	ON_COMMAND(ID_PRIMITIVES_SPIRAL, &CChildView::OnSetPrimitiveSpiral)
	ON_UPDATE_COMMAND_UI(ID_PRIMITIVES_SPIRAL, &CChildView::OnUpdateMenuPrimitiveSpiral)

	ON_COMMAND(ID_BUFFERTYPE_DDB, &CChildView::OnSetBufferTypeDDB)
	ON_UPDATE_COMMAND_UI(ID_BUFFERTYPE_DDB, &CChildView::OnUpdateMenuBufferTypeDDB)
	ON_COMMAND(ID_BUFFERTYPE_DIB, &CChildView::OnSetBufferTypeDIB)
	ON_UPDATE_COMMAND_UI(ID_BUFFERTYPE_DIB, &CChildView::OnUpdateMenuBufferTypeDIB)
	ON_COMMAND(ID_BUFFERTYPE_NONE, &CChildView::OnSetBufferTypeNone)
	ON_UPDATE_COMMAND_UI(ID_BUFFERTYPE_NONE, &CChildView::OnUpdateMenuBufferTypeNone)

	ON_COMMAND(ID_MODE_GDI, &CChildView::OnSetModeGDI)
	ON_UPDATE_COMMAND_UI(ID_MODE_GDI, &CChildView::OnUpdateMenuModeGDI)
	ON_COMMAND(ID_MODE_GDIPLUS, &CChildView::OnSetModeGDIPlus)
	ON_UPDATE_COMMAND_UI(ID_MODE_GDIPLUS, &CChildView::OnUpdateMenuModeGDIPlus)
	ON_COMMAND(ID_MODE_D2D, &CChildView::OnSetModeD2D)
	ON_UPDATE_COMMAND_UI(ID_MODE_D2D, &CChildView::OnUpdateMenuModeD2D)
	ON_COMMAND(ID_MODE_AGG, &CChildView::OnSetModeAGG)
	ON_UPDATE_COMMAND_UI(ID_MODE_AGG, &CChildView::OnUpdateMenuModeAGG)

	ON_MESSAGE(WM_USER + 0x1234, &CChildView::OnPostedInvalidate)
END_MESSAGE_MAP()

class CBltDC : public CDC
{
	CBitmap *_previous_bitmap;
	CDC &_target;
	const CSize _size;

public:
	CBltDC(CDC &target, const CSize &size, CBitmap &buffer, bool use_dib);
	~CBltDC();
};


class CPerformance
{
	static const __int64 _frequency;
	static __int64 GetFrequency();
	
	__int64 _at_start;
	double &_value;

public:
	CPerformance(double &value);
	~CPerformance();
};


const __int64 CPerformance::_frequency(CPerformance::GetFrequency());

CPerformance::CPerformance(double &value)
	: _value(value)
{	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&_at_start));	}

CPerformance::~CPerformance()
{
	__int64 at_end;

	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&at_end));
	_value += 1000.0 * double(at_end - _at_start) / _frequency;
}

__int64 CPerformance::GetFrequency()
{
	__int64 f;

	::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&f));
	return f;
}


CBltDC::CBltDC(CDC &target, const CSize &size, CBitmap &buffer, bool use_dib)
	: _target(target), _size(size)
{
	CreateCompatibleDC(&target);

	if (NULL == (HBITMAP)buffer)
		if (use_dib)
		{
			void *bits = 0;
			BITMAPINFO bi = { 0 };

			bi.bmiHeader.biSize           = sizeof(BITMAPINFOHEADER);
			bi.bmiHeader.biWidth          = size.cx;
			bi.bmiHeader.biHeight         = -size.cy;
			bi.bmiHeader.biPlanes         = 1;
			bi.bmiHeader.biBitCount       = 32; 
			bi.bmiHeader.biCompression    = BI_RGB;
			bi.bmiHeader.biSizeImage      = 0;

			buffer.Attach(::CreateDIBSection(target, &bi, DIB_RGB_COLORS, &bits, NULL, 0));
		}
		else
			buffer.CreateCompatibleBitmap(&target, size.cx, size.cy);
	_previous_bitmap = SelectObject(&buffer);
}

CBltDC::~CBltDC()
{
	BLENDFUNCTION bf = { 0 };

	bf.BlendOp = AC_SRC_OVER;
	bf.SourceConstantAlpha = 120;
	bf.AlphaFormat = AC_SRC_ALPHA;

//	_target.AlphaBlend(0, 0, _size.cx, _size.cy, this, 0, 0, _size.cx, _size.cy, bf);
	_target.BitBlt(0, 0, _size.cx, _size.cy, this, 0, 0, SRCCOPY);
	SelectObject(_previous_bitmap);
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0;          // number of image encoders
	UINT size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if(size == 0)
	{
		return -1;  // Failure
	}

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
	{
		return -1;  // Failure
	}

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}    
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

void CChildView::OnPaint() 
{
	void *bits = 0;
	BITMAPINFO bi = { 0 };

	bi.bmiHeader.biSize           = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth          = 15;
	bi.bmiHeader.biHeight         = 15;
	bi.bmiHeader.biPlanes         = 1;
	bi.bmiHeader.biBitCount       = 32; 
	bi.bmiHeader.biCompression    = BI_RGB;
	bi.bmiHeader.biSizeImage      = 0;



	CPerformance p1(_onpaint_timing);

	CPaintDC dc(this);

	CBitmap pattern;

	pattern.Attach(::CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, NULL, 0));

	Bitmap patternGDIP(15, 15, (15 * 32 + 7) / 8, PixelFormat32bppRGB, (unsigned char*)bits);

	{
		Graphics g(&patternGDIP);
		SolidBrush b(Color(0, 0, 0));

		g.SetPixelOffsetMode(PixelOffsetModeHalf);
		g.Clear(Color(240, 255, 255));
		g.FillRectangle(&b, 0, 0, 1, 1);
	}

	CBrush brushBackgroundGDI;
	brushBackgroundGDI.CreatePatternBrush(&pattern);
	TextureBrush brushBackgroundGDIP(&patternGDIP, WrapModeTile);

	CRect client;
	GetClientRect(&client);
	CSize size = client.Size();
	

	if (dmodeGDIP == _drawMode)
	{
		auto_ptr<CDC> memdc(_mode != bufferNone ? new CBltDC(dc, size, _buffer, _mode == bufferDIB) : 0);
		CDC &target = _mode != bufferNone ? *memdc : dc;
		Graphics g(target);

		g.SetPixelOffsetMode(PixelOffsetModeHalf);

		{
			CPerformance p3(_fill_timing);
			g.Clear(Color(240, 255, 255));
//			g.FillRectangle(&brushBackgroundGDIP, 0, 0, client.Width(), client.Height());
		}

		{
			CPerformance p2(_drawing_timing);

			g.SetSmoothingMode(SmoothingModeHighQuality);
//			g.SetSmoothingMode(SmoothingModeHighSpeed);
//			g.SetInterpolationMode(InterpolationModeNearestNeighbor);
			if (_drawEllipses)
				drawEllipses(g, client.Size(), _ellipses);
			if (_drawLines)
				for (int i = 0; i < 20; ++i)
					drawLines(g, client.Size(), _bars[i]);
			if (_drawBars)
				for (int i = 0; i < 20; ++i)
					drawBars(g, client.Size(), _bars[i]);
			if (_drawSpiral && _gdip_path.get())
			{
				Pen p(Color(0, 150, 255), 3.0f);

				g.DrawPath(&p, _gdip_path.get());
			}
		}

		CPerformance p4(_blit_timing);

		memdc.reset();
	}
	else if (dmodeGDI == _drawMode)
	{
		auto_ptr<CDC> memdc(_mode != bufferNone ? new CBltDC(dc, size, _buffer, _mode == bufferDIB) : 0);
		CDC &target = _mode != bufferNone ? *memdc : dc;

		{
			CPerformance p3(_fill_timing);
			target.FillSolidRect(&client, RGB(0xf0, 0xff, 0xff));
//			target.FillRect(&client, &brushBackgroundGDI);
			GdiFlush();
		}

		{
			CPerformance p2(_drawing_timing);

			if (_drawEllipses)
				drawEllipses(target, client.Size(), _ellipses);
			if (_drawLines)
				for (int i = 0; i < 20; ++i)
					drawLines(target, client.Size(), _bars[i]);
			if (_drawBars)
			{
				for (int i = 0; i < 20; ++i)
					drawBars(target, client.Size(), _bars[i]);
			}
			if (_drawSpiral)
			{
				CPen pen(PS_SOLID | PS_GEOMETRIC, 3, RGB(0, 150, 255));

				CPen *previousPen = target.SelectObject(&pen);

				spiral(target, client.Width() / 2, client.Height() / 2, 5, (std::min)(client.Width(), client.Height()) / 2 - 10, 1, 0);
				dc.SelectObject(previousPen);
			}
		}

		CPerformance p4(_blit_timing);

		memdc.reset();
	}
	else if (dmodeD2D == _drawMode)
	{
		_rtarget->BindDC(dc, &client);

		{
			CPerformance p3(_fill_timing);
			D2D1_RECT_F whole = { client.left, client.top, client.right, client.bottom };
			D2D1_COLOR_F clr = { 240 / 255.0f, 1, 1, 1 };

			_rtarget->BeginDraw();
//			_rtarget->FillRectangle(whole, _brushBackground);
			_rtarget->Clear(clr);
		}

		{
			CPerformance p2(_drawing_timing);

			if (_drawEllipses)
				drawEllipses(_rtarget, client.Size(), _ellipses);
			if (_drawLines)
				for (int i = 0; i < 20; ++i)
					drawLines(_rtarget, client.Size(), _bars[i]);
			if (_drawBars)
			{
				for (int i = 0; i < 20; ++i)
					drawBars(_rtarget, client.Size(), _bars[i]);
			}
			if (_drawSpiral && _d2d_path)
				_rtarget->DrawGeometry(_d2d_path, _brush, 3.0f);
			_rtarget->Flush();
		}

		CPerformance p4(_blit_timing);

		_rtarget->EndDraw();
	}
	else if (dmodeAGG == _drawMode)
	{
		{
			CPerformance p3(_fill_timing);
			agge::rect_i area = { 0, 0, _agg_bitmap.width(), _agg_bitmap.height() };
			agge::fill(_agg_bitmap, area, platform_blender_solid_color(color::make(240, 255, 255)));
		}

		{
			CPerformance p2(_drawing_timing);

			if (_drawEllipses)
				drawEllipses(_agg_bitmap, client.Size(), _ellipses);
			if (_drawLines)
				for (int i = 0; i < 20; ++i)
					drawLines(_agg_bitmap, client.Size(), _bars[i]);
			if (_drawBars)
			{
				for (int i = 0; i < 20; ++i)
					drawBars(_agg_bitmap, client.Size(), _bars[i]);
			}
			if (_drawSpiral)
			{
				_agg_rasterizer.reset();
				add_path(_agg_rasterizer, agg_path_adaptor(_agg_path_flatten));
				_agg_rasterizer.sort();
				_renderer(_agg_bitmap, 0, _agg_rasterizer, platform_blender_solid_color(color::make(0, 150, 255)), winding<>());
			}
		}

		CPerformance p4(_blit_timing);
		_agg_bitmap.blit(dc, 0, 0, client.Width(), client.Height());
	}

	PostMessage(WM_USER + 0x1234);
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	if (cx && cy)
	{
		_agg_bitmap.resize(cx, cy);

		GdipPath spiralGdip;

		spiral(spiralGdip, cx / 2, cy / 2, 5, (std::min)(cx, cy) / 2 - 10, 1, 0);
		_gdip_path.reset(new GraphicsPath(&spiralGdip.first[0], &spiralGdip.second[0], spiralGdip.first.size()));

		_d2d_path.Release();
		_factory->CreatePathGeometry(&_d2d_path);

		CComPtr<ID2D1GeometrySink> sink;
		_d2d_path->Open(&sink);
		spiral(*sink, cx / 2, cy / 2, 5, (std::min)(cx, cy) / 2 - 10, 1, 0);
		sink->Close();

		_agg_path.clear();
		spiral(_agg_path, cx / 2, cy / 2, 5, (std::min)(cx, cy) / 2 - 10, 1, 0);

		agg_path_adaptor p(_agg_path);

		_stroke.width(3);
		_stroke.set_cap(agge::caps::butt());
		_stroke.set_join(agge::joins::bevel());

		agge::path_generator_adapter<agg_path_adaptor, agge::stroke> stroke_path(p, _stroke);

		_agg_path_flatten.clear();
		flatten<real_t>(_agg_path_flatten, stroke_path);
	}

	_buffer.DeleteObject();
	CWnd::OnSize(nType, cx, cy);
}

LRESULT CChildView::OnPostedInvalidate(WPARAM, LPARAM)
{
	if (++_averaging_index == 50 && _status_bar)
	{
		CString status;

		status.Format(_T("total=%gms, fill=%gms, draw=%gms, blit=%gms"), _onpaint_timing / _averaging_index,
			_fill_timing / _averaging_index, _drawing_timing / _averaging_index, _blit_timing / _averaging_index);
		_status_bar->SetPaneText(0, status);
		_averaging_index = 0;
		_onpaint_timing = 0;
		_fill_timing = 0;
		_drawing_timing = 0;
		_blit_timing = 0;
	}
	Invalidate();
	return 0;
}

void CChildView::drawLines(CDC &dc, const CSize &client, const vector<bar> &bars)
{
	const int d = 2, v = client.cy / 2;
	int x = 0;

	CPen pen(PS_SOLID | PS_GEOMETRIC, 2, RGB(0, 0, 0));

	CPen *previousPen = dc.SelectObject(&pen);

	dc.MoveTo(x, v + bars.front().c);
	for_each(bars.begin(), bars.end(), [&] (const bar &b) {
		dc.LineTo(x, v + b.c);

		x += 2 * d + 1;
	});
	dc.SelectObject(previousPen);
}

void CChildView::drawBars(CDC &dc, const CSize &client, const vector<bar> &bars)
{
	const int d = 2, v = client.cy / 2;
	int x = 0;

		CBrush br;
		br.CreateSolidBrush(RGB(0, 0, 0));

		dc.SetBkColor(RGB(0, 0, 0));

	for_each(bars.begin(), bars.end(), [&] (const bar &b) {
		CRect highlow(x, v + b.h, x + 1, v + b.l);
		CRect opentick(x - d, v + b.o, x, v + b.o + 1);
		CRect closetick(x, v + b.c, x + d + 1, v + b.c + 1);

		dc.ExtTextOut(0, 0, ETO_OPAQUE, &opentick, NULL, 0, NULL);
		dc.ExtTextOut(0, 0, ETO_OPAQUE, &closetick, NULL, 0, NULL);
		dc.ExtTextOut(0, 0, ETO_OPAQUE, &highlow, NULL, 0, NULL);

		//dc.MoveTo(CPoint(x - d, v + b.o));
		//dc.LineTo(CPoint(x, v + b.o));

		//dc.MoveTo(CPoint(x + d, v + b.c));
		//dc.LineTo(CPoint(x, v + b.c));

		//dc.MoveTo(CPoint(x, v + b.h - 1));
		//dc.LineTo(CPoint(x, v + b.l - 1));
		x += 2 * d + 1;
	});
}

void CChildView::drawEllipses(CDC &dc, const CSize &client, const vector<ellipse_t> &ellipses)
{
	CGdiObject *previous = dc.SelectStockObject(NULL_PEN);
	for_each(ellipses.begin(), ellipses.end(), [&] (const ellipse_t &e) {
		CBrush br;
		RECT ellipse = e.first;

		++ellipse.right, ++ellipse.bottom;

		br.CreateSolidBrush(e.second);
		CGdiObject* previous = dc.SelectObject(&br);
		dc.Ellipse(&ellipse);
		dc.SelectObject(previous);
	});
	dc.SelectObject(previous);
}


void CChildView::drawLines(Graphics &graphics, const CSize &client, const vector<bar> &bars)
{
	const int d = 2, v = client.cy / 2;
	int x = 0;
	Point previous(x, v + bars.front().c);
	Pen pen(Color(0, 0, 0), 2.0f);

	for_each(bars.begin(), bars.end(), [&] (const bar &b) {
		Point point(x, v + b.c);

		graphics.DrawLine(&pen, previous, point);

		previous = point;
		x += 2 * d + 1;
	});
}

void CChildView::drawBars(Graphics &graphics, const CSize &client, const vector<bar> &bars)
{
	Pen p(Color(0, 0, 0));
	SolidBrush brush(Color(0, 0, 0));
	SolidBrush brushTick(Color(255, 0, 0, 0));

	const float d = 2.0f, v = client.cy / 2;
	float x = 0.0f;

	for_each(bars.begin(), bars.end(), [&] (const bar &b) {

		graphics.FillRectangle(&brush, x, v + b.l, 1.0f, float(b.h - b.l));
		graphics.FillRectangle(&brushTick, x - d, v + b.o, d, 1.0f);
		graphics.FillRectangle(&brushTick, x, v + b.c, d + 1, 1.0f);

		//graphics.DrawLine(&p, x - d, v + b.o, x, v + b.o);
		//graphics.DrawLine(&p, x + d, v + b.c, x, v + b.c);
//		graphics.DrawLine(&p, x, v + b.h, x, v + b.l);
		x += 2 * d + 1;
	});
}

void CChildView::drawLines(ID2D1RenderTarget *graphics, const CSize &client, const vector<bar> &bars)
{
	const int d = 2, v = client.cy / 2;
	int x = 0;
	D2D1_POINT_2F previous = { x, v + bars.front().c };

	for_each(bars.begin(), bars.end(), [&] (const bar &b) {
		D2D1_POINT_2F point = { x, v + b.c };

		graphics->DrawLine(previous, point, _brush, 2.0f);

		previous = point;
		x += 2 * d + 1;
	});
}

void CChildView::drawEllipses(Graphics &graphics, const CSize &client, const vector<ellipse_t> &ellipses)
{
	for_each(ellipses.begin(), ellipses.end(), [&] (const ellipse_t &e) {
		SolidBrush br(Color(224, GetRValue(e.second), GetGValue(e.second), GetBValue(e.second)));
		graphics.FillEllipse(&br, e.first.left, e.first.top, e.first.right - e.first.left, e.first.bottom - e.first.top);
	});
}


void CChildView::drawBars(ID2D1RenderTarget *graphics, const CSize &client, const vector<bar> &bars)
{
	const float d = 2.0f, v = client.cy / 2;
	float x = 0.0f;

	for_each(bars.begin(), bars.end(), [&] (const bar &b) {
		D2D1_RECT_F highlow = { x, v + b.h, x + 1, v + b.l };
		D2D1_RECT_F opentick = { x - d, v + b.o, x, v + b.o + 1 };
		D2D1_RECT_F closetick = { x, v + b.c, x + d + 1, v + b.c + 1 };


		graphics->FillRectangle(highlow, _brush);
		graphics->FillRectangle(opentick, _brushTick);
		graphics->FillRectangle(closetick, _brushTick);
		x += 2 * d + 1;
	});
}

void CChildView::drawEllipses(ID2D1RenderTarget *graphics, const CSize &client, const vector<ellipse_t> &ellipses)
{
	for_each(ellipses.begin(), ellipses.end(), [&] (const ellipse_t &e) {
		CComPtr<ID2D1SolidColorBrush> br;
		D2D1_COLOR_F c = { GetRValue(e.second) / 255.0, GetGValue(e.second) / 255.0, GetBValue(e.second) / 255.0, 224 / 255.0f };
		graphics->CreateSolidColorBrush(c, &br);
		D2D1_ELLIPSE ellipse = {
			{ 0.5 * (e.first.left + e.first.right), 0.5 * (e.first.top + e.first.bottom) },
			0.5 * (e.first.right - e.first.left),
			0.5 * (e.first.bottom - e.first.top)
		};
		graphics->FillEllipse(ellipse, br);
	});
}

void CChildView::drawLines(::bitmap &b, const CSize &client, const std::vector<bar> &bars)
{
	const int d = 2, v = client.cy / 2;
	int x = 0;
	D2D1_POINT_2F previous = { -1, v + bars.front().c };

	_stroke.set_cap(agge::caps::butt());
	_stroke.set_join(agge::joins::bevel());
	_stroke.width(2);

	_agg_rasterizer.reset();

	for_each(bars.begin(), bars.end(), [&] (const bar &b) {
		D2D1_POINT_2F point = { x, v + b.c };

		line_adaptor l(previous.x, previous.y, point.x, point.y);

		agge::path_generator_adapter<line_adaptor, agge::stroke> stroke_path(l, _stroke);

		add_path(_agg_rasterizer, stroke_path);

		previous = point;
		x += 2 * d + 1;
	});

	_agg_rasterizer.sort();
	_renderer(_agg_bitmap, 0, _agg_rasterizer, platform_blender_solid_color(color::make(0, 0, 0)), winding<>());
}

void CChildView::drawBars(::bitmap &b, const CSize &client, const vector<bar> &bars)
{
	const float d = 2.0f, v = client.cy / 2;
	float x = 0.0f;

	_agg_rasterizer.reset();

	for_each(bars.begin(), bars.end(), [&] (const bar &b) {
		D2D1_RECT_F highlow = { x, v + b.h, x + 1, v + b.l };
		D2D1_RECT_F opentick = { x - d, v + b.o, x, v + b.o + 1 };
		D2D1_RECT_F closetick = { x, v + b.c, x + d + 1, v + b.c + 1 };

		_agg_rasterizer.move_to(highlow.left, highlow.top);
		_agg_rasterizer.line_to(highlow.left, highlow.bottom);
		_agg_rasterizer.line_to(highlow.right, highlow.bottom);
		_agg_rasterizer.line_to(highlow.right, highlow.top);
		_agg_rasterizer.line_to(highlow.left, highlow.top);

		_agg_rasterizer.move_to(opentick.left, opentick.top);
		_agg_rasterizer.line_to(opentick.left, opentick.bottom);
		_agg_rasterizer.line_to(opentick.right, opentick.bottom);
		_agg_rasterizer.line_to(opentick.right, opentick.top);
		_agg_rasterizer.line_to(opentick.left, opentick.top);

		_agg_rasterizer.move_to(closetick.left, closetick.top);
		_agg_rasterizer.line_to(closetick.left, closetick.bottom);
		_agg_rasterizer.line_to(closetick.right, closetick.bottom);
		_agg_rasterizer.line_to(closetick.right, closetick.top);
		_agg_rasterizer.line_to(closetick.left, closetick.top);

		x += 2 * d + 1;
	});

	_agg_rasterizer.sort();
	_renderer(_agg_bitmap, 0, _agg_rasterizer, platform_blender_solid_color(color::make(0, 0, 0, 96)), winding<>());
}

void CChildView::drawEllipses(::bitmap &b, const CSize &client, const vector<ellipse_t> &ellipses)
{
	for_each(ellipses.begin(), ellipses.end(), [&] (const ellipse_t &e) {

		ellipse ellipse(0.5 * (e.first.left + e.first.right), 0.5 * (e.first.top + e.first.bottom),
			0.5 * (e.first.right - e.first.left), 0.5 * (e.first.bottom - e.first.top));

		_agg_rasterizer.reset();
		add_path(_agg_rasterizer, ellipse);
		_agg_rasterizer.sort();
		_renderer(_agg_bitmap, 0, _agg_rasterizer, platform_blender_solid_color(color::make(GetRValue(e.second),
			GetGValue(e.second), GetBValue(e.second), 224)), winding<>());
	});
}


void CChildView::OnSetPrimitiveLines()
{	_drawLines = !_drawLines;	}
void CChildView::OnUpdateMenuPrimitiveLines(CCmdUI *pCmd)
{	pCmd->SetCheck(_drawLines ? 1 : 0);	}
void CChildView::OnSetPrimitiveBars()
{	_drawBars = !_drawBars;	}
void CChildView::OnUpdateMenuPrimitiveBars(CCmdUI *pCmd)
{	pCmd->SetCheck(_drawBars ? 1 : 0);	}
void CChildView::OnSetPrimitiveEllipses()
{	_drawEllipses = !_drawEllipses;	}
void CChildView::OnUpdateMenuPrimitiveEllipses(CCmdUI *pCmd)
{	pCmd->SetCheck(_drawEllipses ? 1 : 0);	}
void CChildView::OnSetPrimitiveSpiral()
{	_drawSpiral = !_drawSpiral;	}
void CChildView::OnUpdateMenuPrimitiveSpiral(CCmdUI *pCmd)
{	pCmd->SetCheck(_drawSpiral ? 1 : 0);	}

void CChildView::OnSetBufferTypeDDB()
{	_buffer.DeleteObject(), _mode = bufferDDB;	}
void CChildView::OnUpdateMenuBufferTypeDDB(CCmdUI *pCmd)
{	pCmd->SetRadio(_mode == bufferDDB);	}

void CChildView::OnSetBufferTypeDIB()
{	_buffer.DeleteObject(), _mode = bufferDIB;	}
void CChildView::OnUpdateMenuBufferTypeDIB(CCmdUI *pCmd)
{	pCmd->SetRadio(_mode == bufferDIB);	}

void CChildView::OnSetBufferTypeNone()
{	_buffer.DeleteObject(), _mode = bufferNone;	}
void CChildView::OnUpdateMenuBufferTypeNone(CCmdUI *pCmd)
{	pCmd->SetRadio(_mode == bufferNone);	}

void CChildView::OnSetModeGDI()
{	_drawMode = dmodeGDI;	}
void CChildView::OnUpdateMenuModeGDI(CCmdUI *pCmd)
{	pCmd->SetRadio(_drawMode == dmodeGDI);	}
void CChildView::OnSetModeGDIPlus()
{	_drawMode = dmodeGDIP;	}
void CChildView::OnUpdateMenuModeGDIPlus(CCmdUI *pCmd)
{	pCmd->SetRadio(_drawMode == dmodeGDIP);	}
void CChildView::OnSetModeD2D()
{	_drawMode = dmodeD2D;	}
void CChildView::OnUpdateMenuModeD2D(CCmdUI *pCmd)
{	pCmd->SetRadio(_drawMode == dmodeD2D);	}
void CChildView::OnSetModeAGG()
{	_drawMode = dmodeAGG;	}
void CChildView::OnUpdateMenuModeAGG(CCmdUI *pCmd)
{	pCmd->SetRadio(_drawMode == dmodeAGG);	}
