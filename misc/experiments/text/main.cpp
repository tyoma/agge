#include "text.h"
#include "glyph_access.h"

#include "../common/blenders.h"
#include "../common/color.h"
#include "../common/MainDialog.h"
#include "../common/timing.h"

#include <agge/blenders_simd.h>
#include <agge/clipper.h>
#include <agge/rasterizer.h>
#include <agge/renderer_parallel.h>
#include <functional>
#include <iostream>
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
typedef rasterizer< clipper<int> > my_rasterizer;

namespace demo
{
	struct knuth_hash
	{
		size_t operator ()(int key) const throw() { return key * 2654435761; }
	};

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

	shared_ptr<void> select(HDC hdc, HGDIOBJ hobject)
	{	return shared_ptr<void>(::SelectObject(hdc, hobject), bind(::SelectObject, hdc, _1));	}

	class MemDC : noncopyable
	{
	public:
		MemDC()
			: _dc(::CreateCompatibleDC(NULL), &DeleteDC)
		{	}

		MemDC(::bitmap &surface)
			: _dc(::CreateCompatibleDC(NULL), &DeleteDC)
		{	_selector = select(*this, surface.native());	}

		operator HDC() const
		{	return static_cast<HDC>(_dc.get());	}

	private:
		shared_ptr<void> _dc, _selector;
	};

	class font : noncopyable
	{
	public:
		typedef agg_path_adaptor glyph_outline_path;
		enum {
			precision_shift = 2,

			precision = 1 << precision_shift
		};

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

		const my_rasterizer &get_glyph_raster(uint16_t index, real_t x, real_t y)
		{
			x -= floorf(x);
			y -= floorf(y);

			int precise_index = static_cast<int>(x * precision)
				+ static_cast<int>(y * precision) * precision + index * precision * precision;
			glyph_rasters_cache_t::iterator i = _glyph_rasters.find(precise_index);

			if (_glyph_rasters.end() == i)
			{
				x = static_cast<real_t>(static_cast<int>(x * precision)) / precision;
				y = static_cast<real_t>(static_cast<int>(y * precision)) / precision;
				i = _glyph_rasters.insert(make_pair(precise_index, make_shared<my_rasterizer>())).first;

				font::glyph_outline_path g = get_glyph(index);
				offset_conv<font::glyph_outline_path> og(g, x, y);

				add_path(*i->second, og);
//				i->second->squash();
			}
			return *i->second;
		}

		real_t get_glyph_dx(uint16_t index)
		{
			glyphs_cache_t::const_iterator i = _glyphs.find(index);

			if (_glyphs.end() == i)
				i = _glyphs.insert(make_pair(index, load_glyph(index))).first;
			return i->second.dx;
		}

	private:
		struct glyph_data
		{
			real_t dx;
			glyph_outline outline;
		};

		typedef unordered_map<uint16_t, glyph_data, knuth_hash> glyphs_cache_t;
		typedef unordered_map<int, shared_ptr<my_rasterizer>, knuth_hash> glyph_rasters_cache_t;

	private:
		glyph_data load_glyph(uint16_t index)
		{
			glyph_data gd;

			gd.dx = common::get_glyph_dx(_dc, index);
			get_glyph_outline(_dc, index, gd.outline);
			return gd;
		}

	private:
		MemDC _dc;
		shared_ptr<void> _font, _selector;
		glyphs_cache_t _glyphs;
		glyph_rasters_cache_t _glyph_rasters;
	};

	class TextDrawerGDI : public Drawer
	{
		enum
		{
			font_height = 14
		};

	public:
		TextDrawerGDI()
			: m_font(::CreateFont(font_height, 0, 0, 0, 0, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, _T("tahoma")), &::DeleteObject)
		{	}

	private:
		virtual void draw(::bitmap &surface, Timings &timings)
		{
			const basic_string<TCHAR> &text = c_text2;
			long long timer;
			MemDC dc(surface);
			shared_ptr<void> font_selector = select(dc, m_font.get());
			RECT rc = { 0, 0, surface.width(), surface.height() };

			::SetBkColor(dc, RGB(0, 0, 0));
			::SetTextColor(dc, RGB(255, 255, 255));
			::ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rc, _T(""), 0, 0);

			::SetBkColor(dc, RGB(100, 100, 100));
			::ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rc, _T(""), 0, 0);

			vector<uint16_t> str;
			for (int i = 0; i != 100; ++i)
				str.push_back(i);

			stopwatch(timer);
			for (real_t y = 0; y < 50 * font_height; y += font_height)
			{
				::ExtTextOut(dc, 0, y, ETO_GLYPH_INDEX, NULL, (LPCTSTR)&str[0], str.size(), 0);
			}
			double t = stopwatch(timer);

			timings.rasterization += t;
			timings.stroking += t / str.size() / 50;
		}

		virtual void resize(int /*width*/, int /*height*/)
		{
		}

	private:
		shared_ptr<void> m_font;
	};

	class TextDrawer : public Drawer
	{
		enum
		{
			font_height = 14
		};

	public:
		TextDrawer()
			: _renderer(1), _font(_T("tahoma"), font_height)
		{
		}

	private:
		virtual void draw(::bitmap &surface, Timings &timings)
		{
			long long counter, counter2;
			const rect_i area = { 0, 0, surface.width(), surface.height() };

			stopwatch(counter);
				agge::fill(surface, area, solid_color_brush(rgba8(255, 255, 255)));
			timings.clearing += stopwatch(counter);

			_rasterizer.reset();

			stopwatch(counter2);
			stopwatch(counter);
			for (real_t y = 0; y < 50 * font_height; y += font_height)
			{
				real_t x = 0.0f;

				for (uint16_t i = 0; i != 100; ++i)
				{
					_rasterizer.append(_font.get_glyph_raster(i, x, y), static_cast<int>(x), static_cast<int>(y) + font_height);
					x += _font.get_glyph_dx(i);
				}
			}
			timings.rasterization += stopwatch(counter);

			stopwatch(counter);
				_rasterizer.sort();
			timings.rasterization += stopwatch(counter);

			stopwatch(counter);
				_renderer(surface, 0, _rasterizer, solid_color_brush(rgba8(0, 0, 0, 255)), calculate_alpha<vector_rasterizer::_1_shift>());
			timings.rendition += stopwatch(counter);

			timings.stroking += stopwatch(counter2) / 100.0f / 50;
		}

		virtual void resize(int /*width*/, int /*height*/)
		{
		}

	private:
		typedef blender2<blender_used> solid_color_brush;

	private:
		my_rasterizer _rasterizer;
		__declspec(align(16)) renderer_parallel _renderer;
		font _font;
		vector<uint16_t> _indices;
	};
}

int main()
{
	demo::TextDrawer d;
	MainDialog dlg(d);

	MainDialog::PumpMessages();
}
