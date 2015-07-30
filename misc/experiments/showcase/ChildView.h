#pragma once

#include <agge/renderer_parallel.h>

#include <aggx/blenders.h>
#include <aggx/rasterizer.h>
#include <aggx/win32_bitmap.h>

#include <agg/include/agg_rasterizer_sl_clip.h>

#include <atlbase.h>
#include <vector>
#include <utility>
#include <d2d1.h>

struct bar
{
	int o, h, l, c;
};

namespace Gdiplus
{
	class GraphicsPath;
	class PointF;
}

typedef std::vector< std::pair<std::pair<aggx::real, aggx::real>, unsigned> > AggPath;

class CChildView : public CWnd
{
	class blender;

	typedef std::pair<RECT, COLORREF> ellipse_t;

	enum BufferType {	bufferNone, bufferDIB, bufferDDB	};
	enum DrawMode { dmodeGDI, dmodeGDIP, dmodeD2D, dmodeAGG };

	CComPtr<ID2D1Factory> _factory;
	CComPtr<ID2D1DCRenderTarget> _rtarget;
	CComPtr<ID2D1SolidColorBrush> _brush, _brushTick, _brushBackgroundSolid;
	CComPtr<ID2D1BitmapBrush> _brushBackground;

	std::auto_ptr<aggx::bitmap> _agg_bitmap;
	aggx::rasterizer_scanline_aa<agg::rasterizer_sl_no_clip> _agg_rasterizer;
	agge::renderer_parallel _renderer;

	DrawMode _drawMode;
	bool _drawLines, _drawBars, _drawEllipses, _drawSpiral;
	BufferType _mode;
	CBitmap _buffer;
	CStatusBar *_status_bar;
	double _onpaint_timing, _fill_timing, _drawing_timing, _blit_timing;
	int _averaging_index;

	std::vector<bar> _bars[30];
	std::vector<ellipse_t> _ellipses;

	std::auto_ptr<Gdiplus::GraphicsPath> _gdip_path;
	CComPtr<ID2D1PathGeometry> _d2d_path;
	AggPath _agg_path;
	AggPath _agg_path_flatten;

	DECLARE_MESSAGE_MAP();

	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnSetPrimitiveLines();
	afx_msg void OnUpdateMenuPrimitiveLines(CCmdUI *pCmd);
	afx_msg void OnSetPrimitiveBars();
	afx_msg void OnUpdateMenuPrimitiveBars(CCmdUI *pCmd);
	afx_msg void OnSetPrimitiveEllipses();
	afx_msg void OnUpdateMenuPrimitiveEllipses(CCmdUI *pCmd);
	afx_msg void OnSetPrimitiveSpiral();
	afx_msg void OnUpdateMenuPrimitiveSpiral(CCmdUI *pCmd);

	afx_msg void OnSetBufferTypeDDB();
	afx_msg void OnUpdateMenuBufferTypeDDB(CCmdUI *pCmd);
	afx_msg void OnSetBufferTypeDIB();
	afx_msg void OnUpdateMenuBufferTypeDIB(CCmdUI *pCmd);
	afx_msg void OnSetBufferTypeNone();
	afx_msg void OnUpdateMenuBufferTypeNone(CCmdUI *pCmd);

	afx_msg void OnSetModeGDI();
	afx_msg void OnUpdateMenuModeGDI(CCmdUI *pCmd);
	afx_msg void OnSetModeGDIPlus();
	afx_msg void OnUpdateMenuModeGDIPlus(CCmdUI *pCmd);
	afx_msg void OnSetModeD2D();
	afx_msg void OnUpdateMenuModeD2D(CCmdUI *pCmd);
	afx_msg void OnSetModeAGG();
	afx_msg void OnUpdateMenuModeAGG(CCmdUI *pCmd);


	LRESULT OnPostedInvalidate(WPARAM, LPARAM);

	void drawLines(CDC &dc, const CSize &client, const std::vector<bar> &bars);
	void drawBars(CDC &dc, const CSize &client, const std::vector<bar> &bars);
	void drawEllipses(CDC &dc, const CSize &client, const std::vector<ellipse_t> &ellipses);
	void drawSpiral(CDC &dc, const CSize &client);

	void drawLines(Gdiplus::Graphics &graphics, const CSize &client, const std::vector<bar> &bars);
	void drawBars(Gdiplus::Graphics &graphics, const CSize &client, const std::vector<bar> &bars);
	void drawEllipses(Gdiplus::Graphics &graphics, const CSize &client, const std::vector<ellipse_t> &ellipses);
	void drawSpiral(Gdiplus::Graphics &graphics, const CSize &client);

	void drawLines(ID2D1RenderTarget *graphics, const CSize &client, const std::vector<bar> &bars);
	void drawBars(ID2D1RenderTarget *graphics, const CSize &client, const std::vector<bar> &bars);
	void drawEllipses(ID2D1RenderTarget *graphics, const CSize &client, const std::vector<ellipse_t> &ellipses);
	void drawSpiral(ID2D1RenderTarget *graphics, const CSize &client);

	void drawLines(aggx::bitmap &b, const CSize &client, const std::vector<bar> &bars);
	void drawBars(aggx::bitmap &b, const CSize &client, const std::vector<bar> &bars);
	void drawEllipses(aggx::bitmap &b, const CSize &client, const std::vector<ellipse_t> &ellipses);
	void drawSpiral(aggx::bitmap &b, const CSize &client);

public:
	CChildView();
	virtual ~CChildView();

	void SetStatusBar(CStatusBar *status_bar);
};

inline void CChildView::SetStatusBar(CStatusBar *status_bar)
{	_status_bar = status_bar;	}
