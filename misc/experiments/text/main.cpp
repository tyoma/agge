#include "text.h"
#include "font.h"

#include "../common/blenders.h"
#include "../common/color.h"
#include "../common/dc.h"
#include "../common/MainDialog.h"
#include "../common/timing.h"

#include <agge/blenders_simd.h>
#include <agge/clipper.h>
#include <agge/rasterizer.h>
#include <agge/renderer_parallel.h>
#include <agge.text/layout.h>
#include <windows.h>

namespace std { namespace tr1 { } using namespace tr1; }

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

	template <int precision>
	struct calculate_alpha
	{
		uint8_t operator ()(int area) const
		{
			area >>= precision + 1;
			if (area < 0)
				area = -area;
			if (area > 255)
				area = 255;
			return static_cast<uint8_t>(area);
		}
	};

	template <typename LinesSinkT, typename PathT>
	void add_path(LinesSinkT &sink, PathT &path)
	{
		using namespace agge;

		real_t x, y;

		path.rewind(0);
		for (int command; command = path.vertex(&x, &y), path_command_stop != command; )
		{
			if (path_command_line_to == (command & path_command_mask))
				sink.line_to(x, y);
			else if (path_command_move_to == (command & path_command_mask))
				sink.move_to(x, y);
			if (command & path_flag_close)
				sink.close_polygon();
		}
	}

	template <typename SourceT>
	class offset_conv
	{
	public:
		offset_conv(SourceT &source, real_t dx, real_t dy)
			: _source(source), _dx(dx), _dy(dy)
		{	}

		void rewind(unsigned id)
		{	_source.rewind(id);	}

		int vertex(real_t *x, real_t *y)
		{
			int command = _source.vertex(x, y);

			*x += _dx, *y += _dy;
			return command;
		}

	private:
		void operator =(const offset_conv &rhs);

	private:
		SourceT &_source;
		real_t _dx, _dy;
	};

	class glyph_rasters_cache : noncopyable
	{
	private:
		enum {
			precision = 5
		};

	public:
		void draw_glyph(my_rasterizer &r, const agge::font &font_, uint16_t index, real_t x, real_t y)
		{
			const real_t original_x = x;
			const real_t original_y = y;

			x -= floorf(x);
			y -= floorf(y);

			int precise_index = static_cast<int>(x * precision)
				+ static_cast<int>(y * precision) * precision + index * precision * precision;
			glyph_rasters_cache_t::iterator i = _glyph_rasters.find(precise_index);

			if (_glyph_rasters.end() == i)
			{
				x = static_cast<real_t>(static_cast<int>(x * precision)) / precision;
				y = static_cast<real_t>(static_cast<int>(y * precision)) / precision;
				i = _glyph_rasters.insert(make_pair(precise_index, my_rasterizer())).first;
				
				if (const glyph *g = font_.get_glyph_by_index(index))
				{
					offset_conv<glyph::path_iterator> outline(g->get_outline(), x, y);

					add_path(i->second, outline);
					i->second.sort();
				}
			}
			if (i->second.height())
				r.append(i->second, original_x, original_y);
		}

	private:
		typedef unordered_map<int, my_rasterizer, knuth_hash> glyph_rasters_cache_t;

	private:
		glyph_rasters_cache_t _glyph_rasters;
	};

	class TextDrawer : public Drawer
	{
	public:
		TextDrawer()
			: _renderer(1), _font(font::create(-12, L"tahoma", false, false)),
				_layout(c_text_long.c_str(), _font), _ddx(0.0f), _native(false)
		{	}

	private:
		virtual void draw(::bitmap &surface, Timings &timings)
		{
			long long counter;
			const rect_i area = { 0, 0, surface.width(), surface.height() };
			size_t glyphs = 0;
			dc ctx(&surface);
			dc::handle h = ctx.select(_font->native());

			stopwatch(counter);
				agge::fill(surface, area, solid_color_brush(rgba8(255, 255, 255)));
			timings.clearing += stopwatch(counter);

//			_ddx += 0.01f;

			_rasterizer.reset();

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
				if (_native)
				{
					for (layout::positioned_glyphs_container::const_iterator j = i->begin; j != i->end; ++j)
						_glyph_indices.push_back(j->index);
					::ExtTextOut(ctx, static_cast<int>(x), static_cast<int>(i->reference.y), ETO_GLYPH_INDEX /*| ETO_PDY*/, 0,
						reinterpret_cast<LPCTSTR>(&_glyph_indices[0]), static_cast<UINT>(_glyph_indices.size()), 0);
				}
				else
				{
					for (layout::positioned_glyphs_container::const_iterator j = i->begin; j != i->end; ++j)
					{
						x += j->dx;
						_glyph_rasters.draw_glyph(_rasterizer, *i->glyph_run_font, j->index, x, i->reference.y);
					}
				}
			}

			double append = stopwatch(counter);

			_rasterizer.sort();

			double sort = stopwatch(counter);

			_renderer(surface, 0, _rasterizer, solid_color_brush(rgba8(0, 0, 0, 255)), calculate_alpha<vector_rasterizer::_1_shift>());

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
		__declspec(align(16)) renderer_parallel _renderer;
		shared_ptr<font> _font;
		layout _layout;
		float _ddx;
		vector<uint16_t> _indices;
		glyph_rasters_cache _glyph_rasters;
		vector<uint16_t> _glyph_indices;
		bool _native;
	};
}

int main()
{
	demo::TextDrawer d;
	MainDialog dlg(d);

	MainDialog::PumpMessages();
}
