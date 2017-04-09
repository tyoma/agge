#include "text.h"
#include "win32_font.h"

#include "../common/blenders.h"
#include "../common/color.h"
#include "../common/dc.h"

#include <agge/blenders_simd.h>
#include <agge/clipper.h>
#include <agge/filling_rules.h>
#include <agge/rasterizer.h>
#include <agge/renderer_parallel.h>
#include <agge.text/layout.h>
#include <samples/common/shell.h>
#include <samples/common/timing.h>
#include <windows.h>

using namespace agge;
using namespace common;
using namespace std;

typedef simd::blender_solid_color blender_used;
typedef rasterizer< clipper<int> > my_rasterizer;

namespace demo
{
	template <typename BlenderT>
	class blender2 : public BlenderT
	{
	public:
		blender2(rgba8 color)
			: BlenderT(make_pixel(color), color.a)
		{	}

	private:
		typename BlenderT::pixel make_pixel(rgba8 color)
		{
			typename BlenderT::pixel p = { color.b, color.g, color.r, 0 };
			return p;
		}
	};

	class TextDrawerGDI : public application
	{
	public:
		TextDrawerGDI()
			: _font_accessor(new win32_font_accessor(14, L"tahoma", false, false, font_engine_base::gf_none)),
				_font(new font(_font_accessor)), _layout(c_text_long.c_str(), _font), _ddx(0.0f)
		{	}

	private:
		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			long long counter;
			const rect_i area = { 0, 0, surface.width(), surface.height() };
			size_t glyphs = 0;
			dc ctx(&surface);
			dc::handle h = ctx.select(_font_accessor->native());

			stopwatch(counter);
				agge::fill(surface, area, solid_color_brush(rgba8(255, 255, 255)));
			timings.clearing += stopwatch(counter);

//			_ddx += 0.01f;

			_layout.limit_width(surface.width());

			double layouting = stopwatch(counter);
			
			::SetTextAlign(ctx, TA_BASELINE | TA_LEFT);

			for (layout::const_iterator i = _layout.begin(); i != _layout.end(); ++i)
			{
				real_t x = i->reference.x + _ddx;

				_glyph_indices.clear();

				if (i->reference.y > surface.height())
					break;

				glyphs += distance(i->begin, i->end);
				for (layout::positioned_glyphs_container::const_iterator j = i->begin; j != i->end; ++j)
					_glyph_indices.push_back(j->index);
				::ExtTextOut(ctx, static_cast<int>(x), static_cast<int>(i->reference.y), ETO_GLYPH_INDEX /*| ETO_PDY*/, 0,
					reinterpret_cast<LPCTSTR>(&_glyph_indices[0]), static_cast<UINT>(_glyph_indices.size()), 0);
			}

			double rasterizer = stopwatch(counter);

			timings.stroking += (layouting + rasterizer) / glyphs;
			timings.rasterization += rasterizer;
		}

		virtual void resize(int /*width*/, int /*height*/)
		{
		}

	private:
		typedef blender2<blender_used> solid_color_brush;

	private:
		shared_ptr<win32_font_accessor> _font_accessor;
		shared_ptr<font> _font;
		layout _layout;
		float _ddx;
		vector<uint16_t> _glyph_indices;
	};

	class TextDrawer : public application
	{
	public:
		TextDrawer()
			: _renderer(1), _font_engine(_font_loader),
				_font(_font_engine.create_font(L"tahoma", 14, false, false, font_engine_base::gf_strong)),
				_layout(c_text_long.c_str(), _font), _ddx(0.0f)
		{	}

	private:
		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			long long counter;
			const rect_i area = { 0, 0, surface.width(), surface.height() };
			size_t glyphs = 0;

			stopwatch(counter);
				agge::fill(surface, area, solid_color_brush(rgba8(255, 255, 255)));
			timings.clearing += stopwatch(counter);

//			_ddx += 0.01f;

			_rasterizer.reset();

			_layout.limit_width(surface.width());

			double layouting = stopwatch(counter);

			for (layout::const_iterator i = _layout.begin(); i != _layout.end(); ++i)
			{
				real_t x = i->reference.x + _ddx;

				if (i->reference.y > surface.height())
					break;

				glyphs += distance(i->begin, i->end);
				for (layout::positioned_glyphs_container::const_iterator j = i->begin; j != i->end; ++j)
				{
					x += j->dx;
					_font_engine.render_glyph(_rasterizer, *i->glyph_run_font, j->index, x, i->reference.y);
				}
			}

			double append = stopwatch(counter);

			_rasterizer.sort(true);

			double sort = stopwatch(counter);

			_renderer(surface, 0, _rasterizer, solid_color_brush(rgba8(0, 0, 0, 255)), winding<>());

			double render = stopwatch(counter);

			timings.stroking += (layouting + append + sort + render) / glyphs;
			timings.rasterization += append + sort;
			timings.rendition += render;
		}

		virtual void resize(int /*width*/, int /*height*/)
		{
		}

	private:
		typedef blender2<blender_used> solid_color_brush;

	private:
		my_rasterizer _rasterizer;
		renderer_parallel _renderer;
		demo::win32_font_loader _font_loader;
		font_engine<my_rasterizer> _font_engine;
		shared_ptr<font> _font;
		layout _layout;
		float _ddx;
	};
}

application *agge_create_application()
{
	return new demo::TextDrawer;
}
