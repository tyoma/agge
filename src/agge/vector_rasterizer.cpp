#include <agge/vector_rasterizer.h>

#include <agge/config.h>
#include <agge/precise_delta.h>

namespace agge
{
	namespace
	{
		typedef vector_rasterizer::cells_container::iterator cells_iterator;

		const vector_rasterizer::cell empty_cell = { 0 };

		template <typename T>
		void update_min(T &value, T candidate)
		{
			if (candidate < value)
				value = candidate;
		}

		template <typename T>
		void update_max(T &value, T candidate)
		{
			if (candidate > value)
				value = candidate;
		}

		void jump_xy(cells_iterator &current, int x, int y)
		{
			if (current->x ^ x | current->y ^ y)
			{
				if (current->cover | current->area)
				{
					++current;
					current->area = current->cover = 0;
				}
				current->x = static_cast<short>(x), current->y = static_cast<short>(y);
			}
		}

		void add(cells_iterator current, int x1x2, int delta)
		{
			current->area += x1x2 * delta;
			current->cover += static_cast<short>(delta);
		}

		void seta(cells_iterator current, int x, int y, int area, int delta)
		{
			current->x = static_cast<short>(x);
			current->y = static_cast<short>(y);
			current->area = area;
			current->cover = static_cast<short>(delta);
		}

		void set(cells_iterator current, int x, int y, int x1x2, int delta)
		{
			seta(current, x, y, x1x2 * delta, delta);
		}

		void add_and_commit(cells_iterator &current, int x1x2, int delta)
		{
			int a = current->area + x1x2 * delta;
			int c = current->cover + delta;

			if (a | c)
			{
				current->area = a;
				current->cover = static_cast<short>(c);
				current++;
			}
		}
	}

	vector_rasterizer::vector_rasterizer()
	{	reset();	}

	void vector_rasterizer::reset()
	{
		_cells.assign(1, empty_cell);
		_histogram_y.clear();
		_min_x = _min_y = 0x7FFF, _max_x = _max_y = -0x7FFF;
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

			return;
		}

		count_t n = _cells.size();

		// Untested: we use top metric of cells required to draw the longest line given the current bounds.
		_cells.resize(n + 2 * agge_max(width(), height()) + 1);

		cells_container::iterator current = _cells.begin() + n - 1;
		const int dx = x2 - x1;
		const int dy = y2 - y1;

		if (ey2 == ey1)
		{
			// Everything is on a single hline.

			if (ex2 == ex1)
			{
				jump_xy(current, ex1, ey1);
				add(current, (x1 & _1_mask) + (x2 & _1_mask), dy);
			}
			else
			{
				precise_delta tg_delta(dy, dx);

				hline(current, tg_delta, ey1, x1, x2, dy);
			}
		}
		else
		{
			const int fy1 = y1 & _1_mask;
			const int fy2 = y2 & _1_mask;
			const int step = dy > 0 ? +1 : -1;
			const int near = dy > 0 ? _1 : 0;
			const int far = _1 - near;

			if (x2 == x1)
			{
				// Vertical line - we have to calculate start and end cells,
				// and then - the common values of the area and coverage for
				// all cells of the line. We know exactly there's only one 
				// cell, so, we don't have to call hline().

				const int two_fx = 2 * (x1 & _1_mask);

				jump_xy(current, ex1, ey1);
				add_and_commit(current, two_fx, near - fy1);
				ey1 += step;

				if (ey1 != ey2)
				{
					const int inner_delta = near - far, inner_area = two_fx * inner_delta;

					do
					{
						seta(current++, ex1, ey1, inner_area, inner_delta);
						ey1 += step;
					} while (ey1 != ey2);
				}
				set(current, ex1, ey1, two_fx, fy2 - far);
			}
			else
			{
				// Ok, we have to render several hlines.

				const int lift = near - fy1;
				precise_delta ctg_delta(dx, dy), tg_delta(dy, dx);

				ctg_delta.multiply(lift);

				int x_to = x1 + ctg_delta.next();
			
				if (lift)
					hline(current, tg_delta, ey1, x1, x_to, lift);
				ey1 += step;

				if (ey1 != ey2)
				{
					const int lift = near - far;

					ctg_delta.multiply(lift);

					do
					{
						x1 = x_to;
						x_to += ctg_delta.next();
						hline(current, tg_delta, ey1, x1, x_to, lift);
						ey1 += step;
					} while (ey1 != ey2);
				}
				if (int dy = fy2 - far)
					hline(current, tg_delta, ey1, x_to, x2, dy);
			}
		}
		_cells.resize(static_cast<count_t>(current - _cells.begin() + 1));
	}

	void vector_rasterizer::append(const vector_rasterizer &source, int dx_, int dy_)
	{
		const short dx = static_cast<short>(dx_), dy = static_cast<short>(dy_);
		count_t start = _cells.size() - 1; // Relying on a guarantee that we had at least one cell prior this call.

		if (_cells[start].area | _cells[start].cover)
			++start;
		_cells.resize(start + source._cells.size()); // May throw, so no state change prior this call.
		_sorted = 0;

		cells_container::iterator w = _cells.begin() + start;

		for (const_cells_iterator i = source._cells.begin(), end = source._cells.end(); i != end; ++i)
		{
			cell c = *i;

			c.x += dx, c.y += dy;
			*w++ = c;
		}

		update_min(_min_x, source._min_x + dx_);
		update_min(_min_y, source._min_y + dy_);
		update_max(_max_x, source._max_x + dx_);
		update_max(_max_y, source._max_y + dy_);
	}

	void vector_rasterizer::sort()
	{
		if (_sorted || _min_y > _max_y)
			return;

		_x_sorted_cells.resize(_cells.size());

		_histogram_x.assign(width() + 1, 0);
		_histogram_y.assign(height() + 2, 0);

		count_t *phistogram_x = &_histogram_x[0] - _min_x + 1;
		count_t *phistogram_y = &_histogram_y[0] - _min_y + 2;

		for (const_cells_iterator i = _cells.begin(); i != _cells.end(); ++i)
		{
			phistogram_x[i->x]++;
			phistogram_y[i->y]++;
		}
		for (histogram::iterator i = _histogram_x.begin() + 1; i != _histogram_x.end(); ++i)
			*i += *(i - 1);
		for (histogram::iterator i = _histogram_y.begin() + 2; i != _histogram_y.end(); ++i)
			*i += *(i - 1);
		phistogram_x--;
		phistogram_y--;
		for (cells_container::iterator i = _cells.begin(), j = _x_sorted_cells.begin(), e = _cells.end(); i != e; ++i)
			*(j + phistogram_x[i->x]++) = *i;
		for (cells_container::iterator i = _x_sorted_cells.begin(), j = _cells.begin(), e = _x_sorted_cells.end(); i != e; ++i)
			*(j + phistogram_y[i->y]++) = *i;
		_sorted = 1;
	}

	AGGE_INLINE void vector_rasterizer::hline(cells_container::iterator &current, precise_delta &tg_delta, int ey, int x1, int x2, int dy)
	{
		int ex1 = x1 >> _1_shift;
		const int ex2 = x2 >> _1_shift;
		const int fx1 = x1 & _1_mask;
		const int fx2 = x2 & _1_mask;

		jump_xy(current, ex1, ey);

		if (ex1 == ex2)
		{
			// Everything is located in a single cell. That is easy!

			add(current, fx1 + fx2, dy);
		}
		else
		{
			// Ok, we'll have to render a run of adjacent cells on the same hline...

			const int step = x2 > x1 ? +1 : -1;
			const int near = x2 > x1 ? _1 : 0;
			const int far = _1 - near;

			tg_delta.multiply(near - fx1);

			int y_to = tg_delta.next();

			add_and_commit(current, fx1 + near, y_to);
			ex1 += step;

			if (ex1 != ex2)
			{
				tg_delta.multiply(near - far);

				do
				{
					int d = tg_delta.next();

					y_to += d;
					if (d)
						set(current++, ex1, ey, _1, d);
					ex1 += step;
				} while (ex1 != ex2);
			}
			set(current, ex1, ey, fx2 + far, dy - y_to);
		}
	}

	void vector_rasterizer::extend_bounds(int x, int y)
	{
		update_min(_min_x, x);
		update_min(_min_y, y);
		update_max(_max_x, x);
		update_max(_max_y, y);
	}
}
