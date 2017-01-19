#include <agge/vector_rasterizer.h>

#include "precise_delta.h"

#include <agge/config.h>
#include <agge/tools.h>

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
	{	reset();	}

	void vector_rasterizer::reset()
	{
		_current = empty_cell;
		_cells.clear();
		_scanlines.clear();
		_min_x = 0x7FFF, _min_y = 0x7FFF, _max_x = -0x7FFF, _max_y = -0x7FFF;
		_sorted = 0;
	}

	void vector_rasterizer::line(int x1, int y1, int x2, int y2)
	{
		const int ex1 = x1 >> _1_shift;
		int ey1 = y1 >> _1_shift;
		const int ex2 = x2 >> _1_shift;
		const int ey2 = y2 >> _1_shift;

		extend_bounds(ex1, ey1);
		extend_bounds(ex2, ey2);

		_sorted = 0;

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
			{
				precise_delta tg_delta(dy, dx);

				hline(tg_delta, ey1, x1, x2, dy);
			}
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

			const int lift = near - fy1;
			precise_delta ctg_delta(dx, dy), tg_delta(dy, dx);

			ctg_delta.multiply(lift);
			ctg_delta.next();

			int x_to = x1 + ctg_delta.get();
			
			hline(tg_delta, ey1, x1, x_to, lift);
			ey1 += step;

			if (ey1 != ey2)
			{
				const int lift = near - far;

				ctg_delta.multiply(lift);

				do
				{
					ctg_delta.next();
					x1 = x_to;
					x_to += ctg_delta.get();

					hline(tg_delta, ey1, x1, x_to, lift);
					ey1 += step;
				} while (ey1 != ey2);
			}
			hline(tg_delta, ey1, x_to, x2, fy2 - far);
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
		const sorted_bin empty_bin = { 0 };

		if (_sorted || _min_y > _max_y)
			return;

		commit();
		_x_sorted_cells.resize(_cells.size());

		const int max_length = agge_max(_max_x - _min_x + 1, _max_y - _min_y + 1);
		const int min_x_ = _min_x, min_y_ = _min_y;

		_scanlines.assign(max_length, empty_bin);

		for (const_cells_iterator i = _cells.begin(); i != _cells.end(); ++i)
		{
			++_scanlines[i->x - min_x_].length;
			++_scanlines[i->y - min_y_].start;
		}
		count_t start_x = 0, start_y = 0;
		for (sorted_bins_container::iterator i = _scanlines.begin(); i != _scanlines.end(); ++i)
		{
			const count_t vx = i->length, vy = i->start;

			i->length = start_x;
			i->start = start_y;
			start_x += vx;
			start_y += vy;
		}
		for (const_cells_iterator i = _cells.begin(); i != _cells.end(); ++i)
		{
			const count_t j = _scanlines[i->x - min_x_].length++;
			_x_sorted_cells[j] = *i;
		}
		for (sorted_bins_container::iterator i = _scanlines.begin(); i != _scanlines.end(); ++i)
			i->length = 0;
		for (const_cells_iterator i = _x_sorted_cells.begin(); i != _x_sorted_cells.end(); ++i)
		{
			sorted_bin &bin = _scanlines[i->y - min_y_];
			const count_t j = bin.length++;
			_cells[bin.start + j] = *i;
		}

		_sorted = 1;
	}


	AGGE_INLINE void vector_rasterizer::hline(precise_delta &tg_delta, int ey, int x1, int x2, int dy)
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

			tg_delta.multiply(near - fx1);
			tg_delta.next();

			int y_to = tg_delta.get();

			add(_current, fx1 + near, y_to);
			ex1 += step;
			jump_x(ex1);

			if (ex1 != ex2)
			{
				tg_delta.multiply(near - far);

				do
				{
					tg_delta.next();
					y_to += tg_delta.get();
					set(_current, _1, tg_delta.get());
					ex1 += step;
					jump_x(ex1);
				} while (ex1 != ex2);
			}
			set(_current, fx2 + far, dy - y_to);
		}
	}

	AGGE_INLINE void vector_rasterizer::jump_xy(int x, int y)
	{
		if (_current.x - x | _current.y - y)
			jumpc(x, y);
	}

	AGGE_INLINE void vector_rasterizer::jump_x(int x)
	{
		commit();
		_current.x = static_cast<short>(x);
	}

	AGGE_INLINE void vector_rasterizer::jumpc(int x, int y)
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
