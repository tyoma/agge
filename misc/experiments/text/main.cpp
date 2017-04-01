#include "text.h"
#include "glyph_access.h"

#include "../common/color.h"

#include <agge/blenders_simd.h>
#include <agge/clipper.h>
#include <agge/rasterizer.h>
#include <functional>
#include <samples/common/shell.h>
#include <samples/common/timing.h>
#include <tchar.h>
#include <unordered_map>
#include <vector>
#include <windows.h>

namespace std { namespace tr1 { } using namespace tr1; }

using namespace agge;
using namespace common;
using namespace std;
using namespace std::placeholders;

typedef simd::blender_solid_color blender_used;

namespace
{
	simd::blender_solid_color::pixel make_pixel(rgba8 color)
	{
		simd::blender_solid_color::pixel p = { color.b, color.g, color.r, 0 };
		return p;
	}

	template <typename BlenderT>
	class blender2 : public BlenderT
	{
	public:
		blender2(rgba8 color)
			: BlenderT(make_pixel(color), color.a)
		{	}
	};

	shared_ptr<void> select(HDC hdc, HGDIOBJ hobject)
	{	return shared_ptr<void>(::SelectObject(hdc, hobject), bind(::SelectObject, hdc, _1));	}

	class MemDC : noncopyable
	{
	public:
		MemDC()
			: _dc(::CreateCompatibleDC(NULL), &DeleteDC)
		{	}

		MemDC(platform_bitmap &surface)
			: _dc(::CreateCompatibleDC(NULL), &DeleteDC)
		{	_selector = select(*this, surface.native());	}

		operator HDC() const
		{	return static_cast<HDC>(_dc.get());	}

	private:
		shared_ptr<void> _dc, _selector;
	};

	class font
	{
	public:
		typedef agg_path_adaptor glyph_outline_path;

	public:
		font(const TCHAR *typeface, int height)
			: _font(::CreateFont(height, 0, 0, 0, 0, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, typeface), &::DeleteObject),
				_selector(select(_dc, _font.get()))
		{	}

		void get_glyph_indices(const TCHAR *text, vector<uint16_t> &indices)
		{
			common::get_glyph_indices(_dc, text, indices);
		}

		glyph_outline_path get_glyph(uint16_t index)
		{
			glyphs_cache_t::const_iterator i = _glyphs.find(index);

			if (_glyphs.end() == i)
				i = _glyphs.insert(make_pair(index, load_glyph(index))).first;
			return glyph_outline_path(i->second.outline);
		}

	private:
		struct glyph_data
		{
			real_t dx;
			glyph_outline outline;
		};

		typedef unordered_map<uint16_t, glyph_data> glyphs_cache_t;

	private:
		glyph_data load_glyph(uint16_t index)
		{
			glyph_data gd;

			gd.dx = get_glyph_dx(_dc, index);
			get_glyph_outline(_dc, index, gd.outline);
			return gd;
		}

	private:
		MemDC _dc;
		shared_ptr<void> _font, _selector;
		glyphs_cache_t _glyphs;
	};

	class TextDrawerGDI : public shell::application
	{
	public:
		TextDrawerGDI()
			: m_font(::CreateFont(25, 0, 0, 0, 0, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, _T("Verdana")), &::DeleteObject)
		{	}

	private:
		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			const basic_string<TCHAR> &text = c_text2;
			long long timer;
			MemDC dc(surface);
			shared_ptr<void> font_selector = select(dc, m_font.get());
			RECT rc = { 0, 0, surface.width(), surface.height() };

			::SetBkColor(dc, RGB(0, 0, 0));
			::SetTextColor(dc, RGB(255, 255, 255));
			::ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rc, _T(""), 0, 0);

			stopwatch(timer);
			::DrawText(dc, text.c_str(), (int)text.size(), &rc, DT_WORDBREAK | DT_CALCRECT);
			timings.stroking += stopwatch(timer);

			::SetBkColor(dc, RGB(100, 100, 100));
			::ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rc, _T(""), 0, 0);

			stopwatch(timer);
			::DrawText(dc, text.c_str(), (int)text.size(), &rc, DT_WORDBREAK);
			double t = stopwatch(timer);
			timings.rasterization += t;
			timings.rendition += t / c_text2.size();
		}

		virtual void resize(int /*width*/, int /*height*/)
		{
		}

	private:
		shared_ptr<void> m_font;
	};

	class TextDrawer : public shell::application
	{
	public:
		TextDrawer()
			: _font(_T("Verdana"), 16)
		{	}

	private:
		virtual void draw(platform_bitmap &/*surface*/, timings &timings)
		{
			long long counter;
//			const rect_i area = { 0, 0, surface.width(), surface.height() };

			_rasterizer.reset();

			stopwatch(counter);
//				fill(surface, area, solid_color_brush(rgba8(255, 255, 255)));
			timings.clearing += stopwatch(counter);

			
		}

		virtual void resize(int /*width*/, int /*height*/)
		{
		}

	private:
		typedef blender2<blender_used> solid_color_brush;

	private:
		rasterizer< clipper<int> > _rasterizer;
//		__declspec(align(16)) renderer_parallel _renderer;
		font _font;
		vector<uint16_t> _indices;
	};
}

void agge_sample_main(shell &sh)
{
	TextDrawer d;

	sh.present(d);
}
