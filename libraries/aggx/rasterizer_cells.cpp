#include "rasterizer_cells.h"

#include "basics.h"

#include <algorithm>

namespace aggx
{
	rasterizer_cells::cell rasterizer_cells::cell::initial = { 0, 0, 0x7FFF, 0x7FFF };

	namespace
	{
		void add(rasterizer_cells::cell &c, int x1x2, int delta)
		{
			c.cover += static_cast<short>(delta);
			c.area += x1x2 * delta;
		}

		void seta(rasterizer_cells::cell &c, int area, int delta)
		{
			c.cover = static_cast<short>(delta);
			c.area = area;
		}

		void set(rasterizer_cells::cell &c, int x1x2, int delta)
		{	seta(c, x1x2 * delta, delta);	}
	}

	rasterizer_cells::rasterizer_cells()
		: m_current(cell::initial), m_min_x(0x7FFFFFFF), m_min_y(0x7FFFFFFF),
			m_max_x(-0x7FFFFFFF), m_max_y(-0x7FFFFFFF), m_sorted(false)
	{
	}

	void rasterizer_cells::reset()
	{
		m_cells.clear();
		m_current = cell::initial;
		m_sorted = false;
		m_min_x = 0x7FFFFFFF;
		m_min_y = 0x7FFFFFFF;
		m_max_x = -0x7FFFFFFF;
		m_max_y = -0x7FFFFFFF;
	}

	void rasterizer_cells::sort_cells()
	{
		/*static*/ const sorted_bin initial_sorted_bin = { 0 };

		if(m_sorted)
			return;

		commit_cell();

		m_cells_temporary.resize(m_cells.size());

		unsigned start = 0;
		m_sorted_bins.assign(max_x() - min_x() + 1, initial_sorted_bin);
		for (const cell *i = m_cells.data(), *e = m_cells.data() + m_cells.size(); i != e; ++i)
			++m_sorted_bins[i->x - min_x()].start;
		for (scanline_blocks_container_type::iterator i = m_sorted_bins.begin(); i != m_sorted_bins.end(); ++i)
		{
			unsigned v = i->start;
			i->start = start;
			start += v;
		}
		for (cells_container::const_iterator i = m_cells.begin(), e = m_cells.end(); i != e; ++i)
		{
			sorted_bin& scanline = m_sorted_bins[i->x - min_x()];
			m_cells_temporary[scanline.start + scanline.num++] = *i;
		}

		start = 0;
		m_sorted_bins.assign(max_y() - min_y() + 1, initial_sorted_bin);
		for (const cell *i = m_cells.data(), *e = m_cells.data() + m_cells.size(); i != e; ++i)
			++m_sorted_bins[i->y - min_y()].start;
		for (scanline_blocks_container_type::iterator i = m_sorted_bins.begin(); i != m_sorted_bins.end(); ++i)
		{
			unsigned v = i->start;
			i->start = start;
			start += v;
		}
		for (cells_container::const_iterator i = m_cells_temporary.begin(), e = m_cells_temporary.end(); i != e; ++i)
		{
			sorted_bin& scanline = m_sorted_bins[i->y - min_y()];
			m_cells[scanline.start + scanline.num++] = *i;
		}

		m_sorted = true;
	}

	void rasterizer_cells::line(int x1, int y1, int x2, int y2)
	{
		const int dx = x2 - x1;

		int dy = y2 - y1;
		const int ex1 = x1 >> aggx::poly_subpixel_shift;
		const int ex2 = x2 >> aggx::poly_subpixel_shift;
		int ey1 = y1 >> aggx::poly_subpixel_shift;
		const int ey2 = y2 >> aggx::poly_subpixel_shift;
		const int fy1 = y1 & aggx::poly_subpixel_mask;
		const int fy2 = y2 & aggx::poly_subpixel_mask;

		const int incr = dy > 0 ? +1 : -1;
		const int first = dy > 0 ? aggx::poly_subpixel_scale : 0;

		if (ex1 < m_min_x) m_min_x = ex1;
		if (ex1 > m_max_x) m_max_x = ex1;
		if (ey1 < m_min_y) m_min_y = ey1;
		if (ey1 > m_max_y) m_max_y = ey1;
		if (ex2 < m_min_x) m_min_x = ex2;
		if (ex2 > m_max_x) m_max_x = ex2;
		if (ey2 < m_min_y) m_min_y = ey2;
		if (ey2 > m_max_y) m_max_y = ey2;


		if (dx == 0)
		{
			const int two_fx = (x1 - (ex1 << aggx::poly_subpixel_shift)) << 1;
			if (ey1 != ey2)
			{
				// Vertical line - we have to calculate start and end cells,
				// and then - the common values of the area and coverage for
				// all cells of the line. We know exactly there's only one 
				// cell, so, we don't have to call render_hline().

				//x_from = x1;
				switch_cell(ex1, ey1);

				//render_hline(ey1, x_from, fy1, x_from, first);
				add(m_current, two_fx, first - fy1);

				ey1 += incr;
				switch_cell(ex1, ey1);

				const int inner_delta = first + first - aggx::poly_subpixel_scale;
				const int inner_area = two_fx * inner_delta;
				while (ey1 != ey2)
				{
					seta(m_current, inner_area, inner_delta);
					ey1 += incr;
					switch_cell(ex1, ey1);
				}
				set(m_current, two_fx, fy2 - aggx::poly_subpixel_scale + first);
			}
			else
				add(m_current, two_fx, y2 - y1);
		}
		else if (ey1 == ey2)
		{
			// Everything is on a single hline.
			render_hline(float(256 * dy) / dx, ey1, x1, fy1, x2, fy2);
		}

		else
		{
			// General case - we have to render several hlines
			const int tg = float(256 * dy) / dx;
			const int ctg = float(256 * dx) / dy;

			short stepy = -1;
			int first = 0;

			if (y2 > y1)
			{
				stepy = +1;
				first = poly_subpixel_scale;
			}

			const int y_from = aggx::poly_subpixel_scale - first;

			int x_to = x1 + static_cast<int>(ctg * (first - fy1)) / 256;

			render_hline(tg, ey1, x1, fy1, x_to, first);

			x1 = x_to;

			ey1 += stepy;

			if (ey1 != ey2)
			{
				const int inner_delta = static_cast<int>((first - y_from) * ctg) / 256;

				do
				{
					x_to += inner_delta;
					render_hline(tg, ey1, x1, y_from, x_to, first);
					x1 = x_to;
					ey1 += stepy;
				} while (ey1 != ey2);
			}
			render_hline(tg, ey1, x_to, y_from, x2, fy2);
			return;
		}
	}

	__forceinline void rasterizer_cells::force_switch_cell(int x, int y)
	{
			commit_cell();
			m_current.x     = x;
			m_current.y     = y;
	}

	 __forceinline void rasterizer_cells::switch_cell(int x, int y)
	{
		if (m_current.x != x || m_current.y != y)
		{
			commit_cell();
			m_current.x     = x;
			m_current.y     = y;
		}
	}

	__forceinline void rasterizer_cells::commit_cell()
	{
		if (m_current.area | m_current.cover)
		{
			m_cells.push_back(m_current);
			m_current.cover = 0;
			m_current.area  = 0;
		}
	}

	 __forceinline void rasterizer_cells::render_hline(int tg, short ey, int x1, int fy1, int x2, int fy2)
	{
		short ex1 = static_cast<short>(x1 >> poly_subpixel_shift);
		const int fx1 = x1 & poly_subpixel_mask;
		const int ex2 = static_cast<short>(x2 >> poly_subpixel_shift);
		const int fx2 = x2 & poly_subpixel_mask;

		if (fy1 == fy2)
		{
			// Trivial case. Happens often.
			switch_cell(ex2, ey);
			return;
		}

		switch_cell(ex1, ey);

		if (ex1 == ex2)
		{
			add(m_current, fx1 + fx2, fy2 - fy1);
			return;
		}

		short stepx = -1;
		int first = 0;

		if (x2 > x1)
		{
			stepx = +1;
			first = poly_subpixel_scale;
		}

		int delta = static_cast<int>(tg * (first - fx1))  / 256;
		add(m_current, fx1 + first, delta);

		fy1 += delta;

		ex1 += stepx;
		force_switch_cell(ex1, ey);

		if (ex1 != ex2)
		{
			const int inner_area = static_cast<int>((first + first - poly_subpixel_scale) * poly_subpixel_scale / 256 * tg), inner_delta = inner_area >> poly_subpixel_shift;

			do
			{
				seta(m_current, inner_area, inner_delta);
				fy1 += inner_delta;
				ex1 += stepx;
				force_switch_cell(ex1, ey);
			} while (ex1 != ex2);
		}
		set(m_current, fx2 + poly_subpixel_scale - first, fy2 - fy1);
	}
}
