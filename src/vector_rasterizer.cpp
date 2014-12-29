#include "agge/vector_rasterizer.h"

namespace agge
{
	namespace
	{
		const vector_rasterizer::cell empty_cell = { 0 };

		void add(vector_rasterizer::cell &c, int x1x2, int delta)
		{
			c.cover += static_cast<short>(delta);
			c.area += x1x2 * delta;
		}

		void seta(vector_rasterizer::cell &c, int area, int delta)
		{
			c.cover = static_cast<short>(delta);
			c.area = area;
		}

		void set(vector_rasterizer::cell &c, int x1x2, int delta)
		{	seta(c, x1x2 * delta, delta);	}
	}

	vector_rasterizer::vector_rasterizer()
		: _current(empty_cell)
	{	reset();	}

	void vector_rasterizer::reset()
	{
		_min_x = 0x7FFF, _min_y = 0x7FFF, _max_x = -0x7FFF, _max_y = -0x7FFF;
		_cells.clear();
		_scanlines.clear();
	}

	void vector_rasterizer::line(int x1, int y1, int x2, int y2)
	{
		const int ex1 = x1 >> _1_shift;
		int ey1 = y1 >> _1_shift;
		const int ex2 = x2 >> _1_shift;
		const int ey2 = y2 >> _1_shift;

		extend_bounds(ex1, ey1);
		extend_bounds(ex2, ey2);

		if (y2 == y1)
		{
			// Trivial case. Happens often.

			jump_xy(ex2, ey2);
			return;
		}

		const int dx = x2 - x1;
		const int dy = y2 - y1;

		if (ey2 == ey1)
		{
			// Everything is on a single hline.

			if (ex2 == ex1)
			{
				jump_xy(ex1, ey1);
				add(_current, (x1 & _1_mask) + (x2 & _1_mask), dy);
			}
			else
				hline(_ep * dy / dx, ey1, x1, x2, dy);
			return;
		}

		const int fy1 = y1 & _1_mask;
		const int fy2 = y2 & _1_mask;
		const int step = dy > 0 ? +1 : -1;
		const int near = dy > 0 ? _1 : 0;
		const int far = _1 - near;

		jump_xy(ex1, ey1);

		if (x2 == x1)
		{
			// Vertical line - we have to calculate start and end cells,
			// and then - the common values of the area and coverage for
			// all cells of the line. We know exactly there's only one 
			// cell, so, we don't have to call hline().

			const int two_fx = 2 * (x1 & _1_mask);

			add(_current, two_fx, near - fy1);

			ey1 += step;
			jumpc(ex1, ey1);

			if (ey1 != ey2)
			{
				const int inner_delta = near - far, inner_area = two_fx * inner_delta;

				do
				{
					seta(_current, inner_area, inner_delta);
					ey1 += step;
					jumpc(ex1, ey1);
				} while (ey1 != ey2);
			}
			set(_current, two_fx, fy2 - far);
		}
		else
		{
			// Ok, we have to render several hlines.

			const int tg = _ep * dy / dx, ctg = _ep * dx / dy;
			int delta, acc, x_to;

			acc = ctg * (near - fy1);
			delta = acc >> _ep_shift;
			acc -= delta << _ep_shift;
			x_to = x1 + delta;
			
			hline(tg, ey1, x1, x_to, near - fy1);
			ey1 += step;

			if (ey1 != ey2)
			{
				const int lift = near - far;
				const int delta_precise = ctg * lift;

				do
				{
					acc += delta_precise;
					delta = acc >> _ep_shift;
					acc -= delta << _ep_shift;
					x1 = x_to;
					x_to += delta;

					hline(tg, ey1, x1, x_to, lift);
					ey1 += step;
				} while (ey1 != ey2);
			}
			hline(tg, ey1, x_to, x2, fy2 - far);
		}
	}

	void vector_rasterizer::commit()
	{
		if (_current.cover | _current.area)
		{
			_cells.push_back(_current);
			seta(_current, 0, 0);
		}
	}

	void vector_rasterizer::sort()
	{
		count_t start;
		int y;

		_x_sorted_cells.resize(_cells.size());

		if (_x_sorted_cells.empty())
			return;

		_x_bins.assign(_max_x - _min_x + 1, 0);
		_y_counts.assign(_max_y - _min_y + 1, 0);
		_scanlines.clear();
		_scanlines.reserve(_y_counts.size());
		for (const_cells_iterator i = _cells.begin(); i != _cells.end(); ++i)
		{
			++_x_bins[i->x - _min_x];
			++_y_counts[i->y - _min_y];
		}
		start = 0;
		for (sorted_bins_container::iterator i = _x_bins.begin(); i != _x_bins.end(); ++i)
		{
			count_t v = *i;

			*i = start;
			start += v;
		}
		start = 0;
		y = _min_y;
		for (sorted_bins_container::const_iterator i = _y_counts.begin(); i != _y_counts.end(); ++i, ++y)
		{
			count_t v = *i;
			cells_iterator j = _cells.begin();

			advance(j, start);

			const scanline_cells s = { y, j, j };

			_scanlines.push_back(s);

			start += v;
		}
		for (const_cells_iterator i = _cells.begin(); i != _cells.end(); ++i)
		{
			const count_t j = _x_bins[i->x - _min_x]++;
			_x_sorted_cells[j] = *i;
		}
		for (const_cells_iterator i = _x_sorted_cells.begin(); i != _x_sorted_cells.end(); ++i)
		{
			const cells_iterator j = _scanlines[i->y - _min_y].end++;
			*j = *i;
		}
	}


	__forceinline void vector_rasterizer::hline(int tg, int ey, int x1, int x2, int dy)
	{
		const int ex2 = x2 >> _1_shift;

		if (!dy)
		{
			// Trivial case. Happens often.

			jump_xy(ex2, ey);
			return;
		}

		int ex1 = x1 >> _1_shift;
		const int fx1 = x1 & _1_mask;
		const int fx2 = x2 & _1_mask;

		jump_xy(ex1, ey);

		if (ex1 == ex2)
		{
			// Everything is located in a single cell. That is easy!

			add(_current, fx1 + fx2, dy);
		}
		else
		{
			// Ok, we'll have to render a run of adjacent cells on the same hline...

			const int step = x2 > x1 ? +1 : -1;
			const int near = x2 > x1 ? _1 : 0;
			const int far = _1 - near;
			int delta, acc, y_to;

			acc = tg * (near - fx1);
			delta = acc >> _ep_shift;
			acc -= delta << _ep_shift;
			y_to = delta;

			add(_current, fx1 + near, delta);
			ex1 += step;
			jump_x(ex1);

			if (ex1 != ex2)
			{
				const int delta_precise = (near - far) * tg;

				do
				{
					acc += delta_precise;
					delta = acc >> _ep_shift;
					acc -= delta << _ep_shift;
					y_to += delta;

					set(_current, _1, delta);
					ex1 += step;
					jump_x(ex1);
				} while (ex1 != ex2);
			}
			set(_current, fx2 + far, dy - y_to);
		}
	}

	void vector_rasterizer::jump_xy(int x, int y)
	{
		if (_current.x != x || _current.y != y)
			jumpc(x, y);
	}

	void vector_rasterizer::jump_x(int x)
	{
		commit();
		_current.x = static_cast<short>(x);
	}

	void vector_rasterizer::jumpc(int x, int y)
	{
		commit();
		_current.x = static_cast<short>(x), _current.y = static_cast<short>(y);
	}

	void vector_rasterizer::extend_bounds(int x, int y)
	{
		if (x < _min_x) _min_x = x;
		if (x > _max_x) _max_x = x;
		if (y < _min_y) _min_y = y;
		if (y > _max_y) _max_y = y;
	}
}
