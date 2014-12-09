#pragma once

#include "agg_basics.h"

#include <vector>

namespace agg2
{
	template<class Cell>
	class rasterizer_cells_aa
	{
	public:
		typedef Cell cell_type;

	public:
		rasterizer_cells_aa();
		~rasterizer_cells_aa();

		void reset();
		void style(const cell_type& style_cell);
		void line(int x1, int y1, int x2, int y2);

		int min_x() const { return m_min_x; }
		int min_y() const { return m_min_y; }
		int max_x() const { return m_max_x; }
		int max_y() const { return m_max_y; }

		void sort_cells();

		unsigned total_cells() const 
		{
			return m_num_cells;
		}

		unsigned scanline_num_cells(unsigned y) const 
		{
			return m_sorted_y[y - m_min_y].num; 
		}

		const cell_type* const* scanline_cells(unsigned y) const
		{
			return m_sorted_cells.data() + m_sorted_y[y - m_min_y].start; 
		}

		bool sorted() const { return m_sorted; }

	private:
		enum cell_block_scale_e
		{
			cell_block_shift = 12,
			cell_block_size  = 1 << cell_block_shift,
			cell_block_mask  = cell_block_size - 1,
			cell_block_pool  = 256,
			cell_block_limit = 1024
		};

		struct sorted_y
		{
			unsigned start;
			unsigned num;
		};

	private:
		void set_curr_cell(int x, int y);
		void add_curr_cell();
		void render_hline(int ey, int x1, int y1, int x2, int y2);
		void allocate_block();

	private:
		unsigned m_num_blocks;
		unsigned m_max_blocks;
		unsigned m_curr_block;
		unsigned m_num_cells;
		cell_type** m_cells;
		cell_type* m_curr_cell_ptr;
		std::vector<const cell_type*> m_sorted_cells;
		std::vector<sorted_y> m_sorted_y;
		cell_type m_curr_cell;
		cell_type m_style_cell;
		int m_min_x;
		int m_min_y;
		int m_max_x;
		int m_max_y;
		bool m_sorted;
	};



	template<class Clip>
	class rasterizer_scanline_aa
	{
	public:
		typedef Clip clip_type;
		typedef typename Clip::conv_type conv_type;
		typedef typename Clip::coord_type coord_type;

		enum aa_scale_e
		{
			aa_shift = 8,
			aa_scale = 1 << aa_shift,
			aa_mask = aa_scale - 1,
			aa_scale2 = aa_scale * 2,
			aa_mask2 = aa_scale2 - 1
		};

		rasterizer_scanline_aa() : 
			m_outline(),
			m_clipper(),
			m_filling_rule(agg::fill_non_zero),
			m_auto_close(true),
			m_start_x(0),
			m_start_y(0),
			m_status(status_initial)
		{
			for(int i = 0; i < aa_scale; i++)
				m_gamma[i] = i;
		}

		void reset(); 
		void reset_clipping();
		void clip_box(double x1, double y1, double x2, double y2);
		void filling_rule(agg::filling_rule_e filling_rule);
		void auto_close(bool flag) { m_auto_close = flag; }

		template<class GammaF>
		void gamma(const GammaF& gamma_function)
		{ 
			int i;
			for(i = 0; i < aa_scale; i++)
			{
				m_gamma[i] = uround(gamma_function(double(i) / aa_mask) * aa_mask);
			}
		}

		unsigned apply_gamma(unsigned cover) const 
		{ 
			return m_gamma[cover]; 
		}

		void move_to(int x, int y);
		void line_to(int x, int y);
		void move_to_d(double x, double y);
		void line_to_d(double x, double y);
		void close_polygon();
		void add_vertex(double x, double y, unsigned cmd);

		template<class VertexSource>
		void add_path(VertexSource& vs, unsigned path_id = 0);

		int min_x() const { return m_outline.min_x(); }
		int min_y() const { return m_outline.min_y(); }
		int max_x() const { return m_outline.max_x(); }
		int max_y() const { return m_outline.max_y(); }

		void sort();
		bool rewind_scanlines(int& scan_y);
		bool navigate_scanline(int y, int& scan_y);

		unsigned calculate_alpha(int area) const;

		template<class Scanline>
		bool sweep_scanline(Scanline& sl, int& scan_y) const;

		bool hit_test(int tx, int ty);

	private:
		enum status
		{
			status_initial,
			status_move_to,
			status_line_to,
			status_closed
		};

		struct cell_aa
		{
			int x;
			int y;
			int cover;
			int area;

			void initial()
			{
				x = 0x7FFFFFFF;
				y = 0x7FFFFFFF;
				cover = 0;
				area  = 0;
			}

			void style(const cell_aa&) {}

			int not_equal(int ex, int ey, const cell_aa&) const
			{
				return (ex - x) | (ey - y);
			}
		};

	private:
		rasterizer_cells_aa<cell_aa> m_outline;
		clip_type m_clipper;
		int m_gamma[aa_scale];
		agg::filling_rule_e m_filling_rule;
		bool m_auto_close;
		coord_type m_start_x;
		coord_type m_start_y;
		unsigned m_status;
	};



	template<class Cell> 
	inline rasterizer_cells_aa<Cell>::rasterizer_cells_aa()
		: m_num_blocks(0), m_max_blocks(0), m_curr_block(0), m_num_cells(0), m_cells(0), m_curr_cell_ptr(0),
			m_sorted_cells(), m_sorted_y(), m_min_x(0x7FFFFFFF), m_min_y(0x7FFFFFFF), m_max_x(-0x7FFFFFFF),
			m_max_y(-0x7FFFFFFF), m_sorted(false)
	{
		m_style_cell.initial();
		m_curr_cell.initial();
	}

	template<class Cell> 
	inline rasterizer_cells_aa<Cell>::~rasterizer_cells_aa()
	{
		if(m_num_blocks)
		{
			cell_type** ptr = m_cells + m_num_blocks - 1;
			while(m_num_blocks--)
			{
				delete[] *ptr;
				ptr--;
			}
			delete[] m_cells;
		}
	}

	template<class Cell> 
	inline void rasterizer_cells_aa<Cell>::reset()
	{
		m_num_cells = 0; 
		m_curr_block = 0;
		m_curr_cell.initial();
		m_style_cell.initial();
		m_sorted = false;
		m_min_x =  0x7FFFFFFF;
		m_min_y =  0x7FFFFFFF;
		m_max_x = -0x7FFFFFFF;
		m_max_y = -0x7FFFFFFF;
	}

	template<class Cell> 
	void rasterizer_cells_aa<Cell>::sort_cells()
	{
		if(m_sorted) return; //Perform sort only the first time.

		add_curr_cell();
		m_curr_cell.x     = 0x7FFFFFFF;
		m_curr_cell.y     = 0x7FFFFFFF;
		m_curr_cell.cover = 0;
		m_curr_cell.area  = 0;

		if(m_num_cells == 0) return;

		// DBG: Check to see if min/max works well.
		//for(unsigned nc = 0; nc < m_num_cells; nc++)
		//{
		//    cell_type* cell = m_cells[nc >> cell_block_shift] + (nc & cell_block_mask);
		//    if(cell->x < m_min_x || 
		//       cell->y < m_min_y || 
		//       cell->x > m_max_x || 
		//       cell->y > m_max_y)
		//    {
		//        cell = cell; // Breakpoint here
		//    }
		//}
		// Allocate the array of cell pointers
		m_sorted_cells.resize(m_num_cells);

		// Allocate and zero the Y array
		static const sorted_y initial_sorted_y = { 0 };
		m_sorted_y.assign(m_max_y - m_min_y + 1, initial_sorted_y);

		// Create the Y-histogram (count the numbers of cells for each Y)
		cell_type** block_ptr = m_cells;
		cell_type*  cell_ptr;
		unsigned nb = m_num_cells >> cell_block_shift;
		unsigned i;
		while(nb--)
		{
			cell_ptr = *block_ptr++;
			i = cell_block_size;
			while(i--) 
			{
				m_sorted_y[cell_ptr->y - m_min_y].start++;
				++cell_ptr;
			}
		}

		cell_ptr = *block_ptr++;
		i = m_num_cells & cell_block_mask;
		while(i--) 
		{
			m_sorted_y[cell_ptr->y - m_min_y].start++;
			++cell_ptr;
		}

		// Convert the Y-histogram into the array of starting indexes
		unsigned start = 0;
		for(i = 0; i < m_sorted_y.size(); i++)
		{
			unsigned v = m_sorted_y[i].start;
			m_sorted_y[i].start = start;
			start += v;
		}

		// Fill the cell pointer array sorted by Y
		block_ptr = m_cells;
		nb = m_num_cells >> cell_block_shift;
		while(nb--)
		{
			cell_ptr = *block_ptr++;
			i = cell_block_size;
			while(i--)
			{
				sorted_y& curr_y = m_sorted_y[cell_ptr->y - m_min_y];
				m_sorted_cells[curr_y.start + curr_y.num] = cell_ptr;
				++curr_y.num;
				++cell_ptr;
			}
		}

		cell_ptr = *block_ptr++;
		i = m_num_cells & cell_block_mask;
		while(i--) 
		{
			sorted_y& curr_y = m_sorted_y[cell_ptr->y - m_min_y];
			m_sorted_cells[curr_y.start + curr_y.num] = cell_ptr;
			++curr_y.num;
			++cell_ptr;
		}

		std::for_each(m_sorted_y.begin(), m_sorted_y.end(), [&](const sorted_y& i) {
			std::sort(m_sorted_cells.data() + i.start, m_sorted_cells.data() + i.start + i.num, [](const Cell* lhs, const Cell* rhs) {
				return lhs->x < rhs->x;
			});
		});
		m_sorted = true;
	}

	template<class Cell> 
	inline void rasterizer_cells_aa<Cell>::line(int x1, int y1, int x2, int y2)
	{
		enum dx_limit_e { dx_limit = 16384 << agg::poly_subpixel_shift };

		int dx = x2 - x1;

		if (dx >= dx_limit || dx <= -dx_limit)
		{
			int cx = (x1 + x2) >> 1;
			int cy = (y1 + y2) >> 1;
			line(x1, y1, cx, cy);
			line(cx, cy, x2, y2);
		}

		int dy = y2 - y1;
		int ex1 = x1 >> agg::poly_subpixel_shift;
		int ex2 = x2 >> agg::poly_subpixel_shift;
		int ey1 = y1 >> agg::poly_subpixel_shift;
		int ey2 = y2 >> agg::poly_subpixel_shift;
		int fy1 = y1 & agg::poly_subpixel_mask;
		int fy2 = y2 & agg::poly_subpixel_mask;

		int x_from, x_to;
		int p, rem, mod, lift, delta, first, incr;

		if(ex1 < m_min_x) m_min_x = ex1;
		if(ex1 > m_max_x) m_max_x = ex1;
		if(ey1 < m_min_y) m_min_y = ey1;
		if(ey1 > m_max_y) m_max_y = ey1;
		if(ex2 < m_min_x) m_min_x = ex2;
		if(ex2 > m_max_x) m_max_x = ex2;
		if(ey2 < m_min_y) m_min_y = ey2;
		if(ey2 > m_max_y) m_max_y = ey2;

		set_curr_cell(ex1, ey1);

		//everything is on a single hline
		if (ey1 == ey2)
		{
			render_hline(ey1, x1, fy1, x2, fy2);
			return;
		}

		//Vertical line - we have to calculate start and end cells,
		//and then - the common values of the area and coverage for
		//all cells of the line. We know exactly there's only one 
		//cell, so, we don't have to call render_hline().
		incr  = 1;
		if (dx == 0)
		{
			int ex = x1 >> agg::poly_subpixel_shift;
			int two_fx = (x1 - (ex << agg::poly_subpixel_shift)) << 1;
			int area;

			first = agg::poly_subpixel_scale;
			if (dy < 0)
			{
				first = 0;
				incr  = -1;
			}

			x_from = x1;

			//render_hline(ey1, x_from, fy1, x_from, first);
			delta = first - fy1;
			m_curr_cell.cover += delta;
			m_curr_cell.area  += two_fx * delta;

			ey1 += incr;
			set_curr_cell(ex, ey1);

			delta = first + first - agg::poly_subpixel_scale;
			area = two_fx * delta;
			while (ey1 != ey2)
			{
				//render_hline(ey1, x_from, poly_subpixel_scale - first, x_from, first);
				m_curr_cell.cover = delta;
				m_curr_cell.area  = area;
				ey1 += incr;
				set_curr_cell(ex, ey1);
			}
			//render_hline(ey1, x_from, poly_subpixel_scale - first, x_from, fy2);
			delta = fy2 - agg::poly_subpixel_scale + first;
			m_curr_cell.cover += delta;
			m_curr_cell.area  += two_fx * delta;
			return;
		}

		//ok, we have to render several hlines
		p     = (agg::poly_subpixel_scale - fy1) * dx;
		first = agg::poly_subpixel_scale;

		if (dy < 0)
		{
			p     = fy1 * dx;
			first = 0;
			incr  = -1;
			dy    = -dy;
		}

		delta = p / dy;
		mod   = p % dy;

		if (mod < 0)
		{
			delta--;
			mod += dy;
		}

		x_from = x1 + delta;
		render_hline(ey1, x1, fy1, x_from, first);

		ey1 += incr;
		set_curr_cell(x_from >> agg::poly_subpixel_shift, ey1);

		if (ey1 != ey2)
		{
			p     = agg::poly_subpixel_scale * dx;
			lift  = p / dy;
			rem   = p % dy;

			if (rem < 0)
			{
				lift--;
				rem += dy;
			}
			mod -= dy;

			while (ey1 != ey2)
			{
				delta = lift;
				mod  += rem;
				if (mod >= 0)
				{
					mod -= dy;
					delta++;
				}

				x_to = x_from + delta;
				render_hline(ey1, x_from, agg::poly_subpixel_scale - first, x_to, first);
				x_from = x_to;

				ey1 += incr;
				set_curr_cell(x_from >> agg::poly_subpixel_shift, ey1);
			}
		}
		render_hline(ey1, x_from, agg::poly_subpixel_scale - first, x2, fy2);
	}

	template<class Cell> 
	inline void rasterizer_cells_aa<Cell>::set_curr_cell(int x, int y)
	{
		if (m_curr_cell.not_equal(x, y, m_style_cell))
		{
			add_curr_cell();
			m_curr_cell.style(m_style_cell);
			m_curr_cell.x     = x;
			m_curr_cell.y     = y;
			m_curr_cell.cover = 0;
			m_curr_cell.area  = 0;
		}
	}

	template<class Cell> 
	inline void rasterizer_cells_aa<Cell>::add_curr_cell()
	{
		if (m_curr_cell.area | m_curr_cell.cover)
		{
			if ((m_num_cells & cell_block_mask) == 0)
			{
				if (m_num_blocks >= cell_block_limit)
					return;
				allocate_block();
			}
			*m_curr_cell_ptr++ = m_curr_cell;
			++m_num_cells;
		}
	}

	template<class Cell> 
	inline void rasterizer_cells_aa<Cell>::render_hline(int ey, int x1, int y1, int x2, int y2)
	{
		int ex1 = x1 >> agg::poly_subpixel_shift;
		int ex2 = x2 >> agg::poly_subpixel_shift;
		int fx1 = x1 & agg::poly_subpixel_mask;
		int fx2 = x2 & agg::poly_subpixel_mask;

		int delta, p, first, dx;
		int incr, lift, mod, rem;

		//trivial case. Happens often
		if(y1 == y2)
		{
			set_curr_cell(ex2, ey);
			return;
		}

		//everything is located in a single cell.  That is easy!
		if(ex1 == ex2)
		{
			delta = y2 - y1;
			m_curr_cell.cover += delta;
			m_curr_cell.area  += (fx1 + fx2) * delta;
			return;
		}

		//ok, we'll have to render a run of adjacent cells on the same
		//hline...
		p     = (agg::poly_subpixel_scale - fx1) * (y2 - y1);
		first = agg::poly_subpixel_scale;
		incr  = 1;

		dx = x2 - x1;

		if(dx < 0)
		{
			p     = fx1 * (y2 - y1);
			first = 0;
			incr  = -1;
			dx    = -dx;
		}

		delta = p / dx;
		mod   = p % dx;

		if(mod < 0)
		{
			delta--;
			mod += dx;
		}

		m_curr_cell.cover += delta;
		m_curr_cell.area  += (fx1 + first) * delta;

		ex1 += incr;
		set_curr_cell(ex1, ey);
		y1  += delta;

		if(ex1 != ex2)
		{
			p     = agg::poly_subpixel_scale * (y2 - y1 + delta);
			lift  = p / dx;
			rem   = p % dx;

			if (rem < 0)
			{
				lift--;
				rem += dx;
			}

			mod -= dx;

			while (ex1 != ex2)
			{
				delta = lift;
				mod  += rem;
				if(mod >= 0)
				{
					mod -= dx;
					delta++;
				}

				m_curr_cell.cover += delta;
				m_curr_cell.area  += agg::poly_subpixel_scale * delta;
				y1  += delta;
				ex1 += incr;
				set_curr_cell(ex1, ey);
			}
		}
		delta = y2 - y1;
		m_curr_cell.cover += delta;
		m_curr_cell.area  += (fx2 + agg::poly_subpixel_scale - first) * delta;
	}

	template<class Cell> 
	inline void rasterizer_cells_aa<Cell>::allocate_block()
	{
		if (m_curr_block >= m_num_blocks)
		{
			if (m_num_blocks >= m_max_blocks)
			{
				cell_type** new_cells = new cell_type*[m_max_blocks + cell_block_pool];

				if (m_cells)
				{
					std::copy_n(m_cells, m_max_blocks, new_cells);
					delete[] m_cells;
				}
				m_cells = new_cells;
				m_max_blocks += cell_block_pool;
			}
			m_cells[m_num_blocks++] = new cell_type[cell_block_size];
		}
		m_curr_cell_ptr = m_cells[m_curr_block++];
	}




	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::reset() 
	{ 
		m_outline.reset(); 
		m_status = status_initial;
	}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::move_to(int x, int y)
	{
		if (m_outline.sorted())
			reset();
		if (m_auto_close)
			close_polygon();
		m_clipper.move_to(m_start_x = conv_type::downscale(x), m_start_y = conv_type::downscale(y));
		m_status = status_move_to;
	}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::line_to(int x, int y)
	{
		m_clipper.line_to(m_outline, conv_type::downscale(x), conv_type::downscale(y));
		m_status = status_line_to;
	}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::move_to_d(double x, double y) 
	{ 
		if(m_outline.sorted())
			reset();
		if(m_auto_close)
			close_polygon();
		m_clipper.move_to(m_start_x = conv_type::upscale(x), m_start_y = conv_type::upscale(y)); 
		m_status = status_move_to;
	}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::line_to_d(double x, double y) 
	{ 
		m_clipper.line_to(m_outline, conv_type::upscale(x), conv_type::upscale(y)); 
		m_status = status_line_to;
	}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::close_polygon()
	{
		if(m_status == status_line_to)
		{
			m_clipper.line_to(m_outline, m_start_x, m_start_y);
			m_status = status_closed;
		}
	}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::add_vertex(double x, double y, unsigned cmd)
	{
		if (agg::is_move_to(cmd)) 
			move_to_d(x, y);
		else 
			if (agg::is_vertex(cmd))
				line_to_d(x, y);
			else if(agg::is_close(cmd))
				close_polygon();
	}

	template<class Clip>
	template<class VertexSource>
	inline void rasterizer_scanline_aa<Clip>::add_path(VertexSource& vs, unsigned path_id)
	{
		double x;
		double y;

		unsigned cmd;
		vs.rewind(path_id);
		if(m_outline.sorted()) reset();
		while (!agg::is_stop(cmd = vs.vertex(&x, &y)))
			add_vertex(x, y, cmd);
	}

	template<class Clip>
	inline unsigned rasterizer_scanline_aa<Clip>::calculate_alpha(int area) const
	{
		int cover = area >> (agg::poly_subpixel_shift*2 + 1 - aa_shift);

		if(cover < 0) cover = -cover;
		if(m_filling_rule == agg::fill_even_odd)
		{
			cover &= aa_mask2;
			if(cover > aa_scale)
			{
				cover = aa_scale2 - cover;
			}
		}
		if(cover > aa_mask) cover = aa_mask;
		return m_gamma[cover];
	}

	template<class Clip> 
	inline bool rasterizer_scanline_aa<Clip>::rewind_scanlines(int& scan_y)
	{
		if (m_auto_close)
			close_polygon();
		m_outline.sort_cells();
		if (m_outline.total_cells() == 0) 
			return false;
		scan_y = m_outline.min_y();
		return true;
	}

	template<class Clip>
	template<class Scanline>
	inline bool rasterizer_scanline_aa<Clip>::sweep_scanline(Scanline& sl, int& scan_y) const
	{
		if (scan_y > m_outline.max_y())
			return false;
		sl.reset_spans();
		unsigned num_cells = m_outline.scanline_num_cells(scan_y);
		const cell_aa* const* cells = m_outline.scanline_cells(scan_y);
		int cover = 0;

		while (num_cells)
		{
			const cell_aa* cur_cell = *cells;
			int x = cur_cell->x;
			int area = cur_cell->area;

			cover += cur_cell->cover;

			//accumulate all cells with the same X
			while (--num_cells)
			{
				cur_cell = *++cells;
				if (cur_cell->x != x)
					break;
				area += cur_cell->area;
				cover += cur_cell->cover;
			}

			if (area)
			{
				if (unsigned int alpha = calculate_alpha((cover << (agg::poly_subpixel_shift + 1)) - area))
					sl.add_cell(x, alpha);
				x++;
			}

			if (num_cells && cur_cell->x > x)
			{
				if (unsigned int alpha = calculate_alpha(cover << (agg::poly_subpixel_shift + 1)))
					sl.add_span(x, cur_cell->x - x, alpha);
			}
		}
		sl.finalize(scan_y);
		++scan_y;
		return true;
	}
}
