#pragma once

#include "basics.h"

namespace aggx
{
	template <class PixelFormat>
	class renderer_base
	{
	public:
		typedef PixelFormat pixfmt_type;
		typedef typename pixfmt_type::color_type color_type;
		typedef typename pixfmt_type::pixel_type pixel_type;

	public:
		explicit renderer_base(pixfmt_type& target);

		void clear(const color_type& c);

		void blend_solid_hspan(int x, int y, int len, const pixel_type& c, const aggx::cover_type* covers);
		void blend_hline(int x1, int y, int x2, const pixel_type& c, aggx::cover_type cover);

		int xmin() const
		{
			return m_clip_box.x1;
		}

		int ymin() const
		{
			return m_clip_box.y1;
		}

		int xmax() const
		{
			return m_clip_box.x2;
		}

		int ymax() const
		{
			return m_clip_box.y2;
		}

		unsigned width() const
		{
			return m_target.width();
		}

		unsigned height() const
		{
			return m_target.height();
		}

	private:
		pixfmt_type& m_target;
		aggx::rect_i m_clip_box;
	};



	template <class PixelFormat>
	inline renderer_base<PixelFormat>::renderer_base(pixfmt_type& target)
		: m_target(target), m_clip_box(0, 0, target.width() - 1, target.height() - 1)
	{
	}

	template <class PixelFormat>
	inline void renderer_base<PixelFormat>::clear(const color_type& c)
	{
		unsigned y;

		pixel_type cp = pixel_type::from_rgba(c);

		if (width())
			for(y = 0; y < height(); y++)
				m_target.copy_hline(0, y, width(), cp);
	}

	template <class PixelFormat>
	inline void renderer_base<PixelFormat>::blend_solid_hspan(int x, int y, int len, const pixel_type& c, const aggx::cover_type* covers)
	{
		if (y < ymin() || ymax() < y)
			return;
		if (x < xmin())
		{
			len -= xmin() - x;
			if (len <= 0)
				return;
			covers += xmin() - x;
			x = xmin();
		}
		if(x + len > xmax())
		{
			len = xmax() - x + 1;
			if (len <= 0)
				return;
		}
		m_target.blend_solid_hspan(x, y, len, c, covers);
	}

	template <class PixelFormat>
	inline void renderer_base<PixelFormat>::blend_hline(int x1, int y, int x2, const pixel_type& c, aggx::cover_type cover)
	{
		if (y < ymin() || ymax() < y)
			return;
		if (x1 > x2)
			std::swap(x1, x2);
		if (x2 < xmin() || xmax() < x1)
			return;
		if (x1 < xmin())
			x1 = xmin();
		if (x2 > xmax())
			x2 = xmax();

		m_target.blend_hline(x1, y, x2 - x1 + 1, c, cover);
	}



	template<class Scanline, class BaseRenderer, class ColorT> 
	inline void render_scanline_aa_solid(const Scanline& sl, int y, BaseRenderer& ren, const ColorT& color)
	{
		unsigned int spans = sl.num_spans();

		for (typename Scanline::const_iterator span = sl.begin(); spans; ++span, --spans)
		{
			if (span->len > 0)
				ren.blend_solid_hspan(span->x, y, (unsigned)span->len, color, span->covers);
			else
				ren.blend_hline(span->x, y, (unsigned)(span->x - span->len - 1), color, *(span->covers));
		}
	}

	template<class Rasterizer, class Scanline, class BaseRenderer, class ColorT>
	inline void render_scanlines_aa_solid(Rasterizer& ras, Scanline& sl, BaseRenderer& ren, const ColorT& color)
	{
		// Explicitly convert "color" to the BaseRenderer color type.
		// For example, it can be called with color type "rgba", while
		// "rgba8" is needed. Otherwise it will be implicitly 
		// converted in the loop many times.
		//----------------------
		typename BaseRenderer::pixel_type ren_color(BaseRenderer::pixel_type::from_rgba(color));

		sl.reset(ras.min_x(), ras.max_x());
		ras.prepare();
		for (int y = ras.min_y(); y <= ras.max_y(); ++y)
		{
			ras.sweep_scanline(sl, y);
			render_scanline_aa_solid(sl, y, ren, ren_color);
		}
	}
}
