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
		: _cells(cells), _current(cell::empty)
	{	reset();	}

	void vector_rasterizer::reset()
	{	_min_x = 0x7FFF, _min_y = 0x7FFF, _max_x = -0x7FFF, _max_y = -0x7FFF;	}

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

		const int dx = x2 - x1;
		const int dy = y2 - y1;
		const short stepy = dy > 0 ? +1 : -1;
		const int first = dy > 0 ? _1 : 0;
		const int fy1 = y1 & _1_mask;
		const int fy2 = y2 & _1_mask;

		jump(ex1, ey1);

		if (x2 == x1 && ey2 != ey1)
		{
			const int two_fx = 2 * (x1 & _1_mask);

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
		}
		else if (ey2 == ey1)
		{
			if (ex2 == ex1)
				add(_current, (x1 & _1_mask) + (x2 & _1_mask), dy);
			else
				hline(float(_ep * dy) / dx, ey1, x1, x2, dy);
		}
		else
		{
			const int tg = float(_ep * dy) / dx, ctg = float(_ep * dx) / dy;
			int delta, accx_precise, accx, x_to;

			accx_precise = ctg * (first - fy1);
			delta = accx_precise / _ep;
			accx = delta;
			x_to = x1 + delta;
			
			hline(tg, ey1, x1, x_to, first - fy1);
			ey1 += stepy;

			if (ey1 != ey2)
			{
				const int lift = first + first - _1;
				const int delta_precise = ctg * lift;

				do
				{
					accx_precise += delta_precise;
					delta = (accx_precise - _ep * accx) / _ep;
					x1 = x_to;
					accx += delta;
					x_to += delta;

					hline(tg, ey1, x1, x_to, lift);
					ey1 += stepy;
				} while (ey1 != ey2);
			}
			hline(tg, ey1, x_to, x2, fy2 - _1 + first);
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

	__forceinline void vector_rasterizer::hline(int tg, short ey, int x1, int x2, int dy)
	{
		const short ex2 = x2 >> _1_shift;

		if (!dy)
		{
			jump(ex2, ey);	// Untestable optimization.
			return;
		}

		short ex1 = x1 >> _1_shift;
		const int fx1 = x1 & _1_mask;
		const int fx2 = x2 & _1_mask;

		if (ex1 == ex2)
		{
			jump(ex1, ey);
			add(_current, fx1 + fx2, dy);
		}
		else
		{
			const short stepx = x2 > x1 ? +1 : -1;
			const int first = x2 > x1 ? _1 : 0;
			int delta, accy_precise, accy;
				
			jump(ex1, ey);

			accy_precise = tg * (first - fx1);
			delta = accy_precise / _ep;
			accy = delta;

			add(_current, fx1 + first, delta);
			ex1 += stepx;
			jump(ex1, ey);

			if (ex1 != ex2)
			{
				const int delta_precise = (first + first - _1) * tg;

				do
				{
					accy_precise += delta_precise;
					delta = (accy_precise - _ep * accy) / _ep;
					accy += delta;

					set(_current, _1, delta);
					ex1 += stepx;
					jump(ex1, ey);
				} while (ex1 != ex2);
			}
			set(_current, fx2 + _1 - first, dy - accy);
		}
	}

	void vector_rasterizer::jump(short x, short y)
	{
		if (_current.x != x || _current.y != y)
			jumpc(x, y);
	}

	void vector_rasterizer::jumpc(short x, short y)
	{
		commit();
		_current.x = x, _current.y = y;
	}

	void vector_rasterizer::extend_bounds(short x, short y)
	{
		if (x < _min_x) _min_x = x;
		if (x > _max_x) _max_x = x;
		if (y < _min_y) _min_y = y;
		if (y > _max_y) _max_y = y;
	}
}
