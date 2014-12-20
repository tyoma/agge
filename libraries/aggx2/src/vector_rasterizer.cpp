#include "../vector_rasterizer.h"

namespace aggx
{
	const vector_rasterizer::cell vector_rasterizer::cell::empty = { 0 };

	namespace
	{
		void add(vector_rasterizer::cell &c, int x1x2, int delta)
		{
			c.cover += delta;
			c.area += x1x2 * delta;
		}

		void seta(vector_rasterizer::cell &c, int area, int delta)
		{
			c.cover = delta;
			c.area = area;
		}

		void set(vector_rasterizer::cell &c, int x1x2, int delta)
		{	seta(c, x1x2 * delta, delta);	}
	}

	vector_rasterizer::vector_rasterizer(cells_container &cells)
		: _cells(cells), _min_x(0x7FFFFFFF), _min_y(0x7FFFFFFF), _max_x(-0x7FFFFFFF), _max_y(-0x7FFFFFFF),
			_current(cell::empty)
	{	}

	void vector_rasterizer::line(int x1, int y1, int x2, int y2)
	{
		const short ex1 = static_cast<short>(x1 >> _1_shift);
		short ey1 = static_cast<short>(y1 >> _1_shift);
		const short ex2 = static_cast<short>(x2 >> _1_shift);
		const short ey2 = static_cast<short>(y2 >> _1_shift);

		extend_bounds(ex1, ey1);
		extend_bounds(ex2, ey2);

		if (y2 == y1)
		{
			// Trivial case. Happens often.
			jump(ex2, ey2);
			return;
		}

		const int fx1 = x1 & _1_mask;
		const int fy1 = y1 & _1_mask;
		const int fx2 = x2 & _1_mask;
		const int fy2 = y2 & _1_mask;
		short stepy = -1;
		int first = 0;

		if (y2 > y1)
		{
			stepy = +1;
			first = _1;
		}

		jump(ex1, ey1);

		if (x2 == x1 && ey2 != ey1)
		{
			const int two_fx = 2 * fx1;

			add(_current, two_fx, first - fy1);

			ey1 += stepy;
			jumpc(ex1, ey1);

			if (ey1 != ey2)
			{
				const int inner_delta = first + first - _1, inner_area = two_fx * inner_delta;

				do
				{
					seta(_current, inner_area, inner_delta);
					ey1 += stepy;
					jumpc(ex1, ey1);
				} while (ey1 != ey2);
			}
			set(_current, two_fx, fy2 + first - _1);
			return;
		}
		
		if (ey2 == ey1)
		{
			if (ex2 == ex1)
				add(_current, fx1 + fx2, y2 - y1);
			else
				hline(_ep * (y2 - y1) / (x2 - x1), ey1, x1, x2, y2 - y1);
			return;
		}


	}

	void vector_rasterizer::commit()
	{
		if (_current.cover || _current.area)
		{
			_cells.push_back(_current);
			_current.cover = 0;
			_current.area = 0;
		}
	}

	void vector_rasterizer::hline(int tg, short ey, int x1, int x2, int dy)
	{
		int ex1 = x1 >> _1_shift;
		const int fx1 = x1 & _1_mask;
		const int ex2 = x2 >> _1_shift;
		const int fx2 = x2 & _1_mask;
		int stepx, first;
		int delta_precise, delta_approx, accy_precise = 0, accy_approx = 0;

		if (x2 > x1)
			stepx = +1, first = _1;
		else
			stepx = -1, first = 0;
				
		jump(ex1, ey);

		delta_precise = tg * (first - fx1);
		delta_approx = delta_precise / _ep;

		set(_current, fx1 + first, delta_approx);
		accy_precise += delta_precise;
		accy_approx += delta_approx;
		ex1 += stepx;
		jump(ex1, ey);

		if (ex1 != ex2)
		{
			delta_precise = (first + first - _1) * tg;
			do
			{
				accy_precise += delta_precise;
				delta_approx = (accy_precise - _ep * accy_approx) / _ep;
				accy_approx += delta_approx;

				set(_current, _1, delta_approx);
				ex1 += stepx;
				jump(ex1, ey);
			} while (ex1 != ex2);
		}
		set(_current, fx2 + _1 - first, dy - accy_approx);
	}

	void vector_rasterizer::jump(short x, short y)
	{
		if (_current.x - x || _current.y - y)
			jumpc(x, y);
	}

	void vector_rasterizer::jumpc(short x, short y)
	{
		commit();
		_current.x = x;
		_current.y = y;
	}

	void vector_rasterizer::extend_bounds(int x, int y)
	{
		if (x < _min_x) _min_x = x;
		if (x > _max_x) _max_x = x;
		if (y < _min_y) _min_y = y;
		if (y > _max_y) _max_y = y;
	}
}
