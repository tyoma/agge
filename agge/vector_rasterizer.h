#pragma once

#include <utility>
#include <vector>

namespace agge
{
	class vector_rasterizer
	{
	public:
		enum
		{
			_1_shift = 8,
			_ep_shift = _1_shift + 4,

			_1 = 1 << _1_shift,
			_1_mask = _1 - 1,
			_ep = 1 << _ep_shift,
		};

#pragma pack(push, 1)
		struct cell
		{
			short x, y;
			int area;
			short cover;
		};

		struct scanline_cells;
#pragma pack(pop)

		typedef std::vector<cell> cells_container;
		typedef cells_container::const_iterator const_cells_iterator;
		typedef cells_container::iterator cells_iterator;
		typedef std::vector<scanline_cells> scanline_cells_container;
		typedef scanline_cells_container::const_iterator const_iterator;
		typedef std::pair<int, int> range;

	public:
		vector_rasterizer();
		
		void reset();

		void line(int x1, int y1, int x2, int y2);
		void commit();
		const cells_container &cells() const;
		void sort();
		const_iterator scanlines_begin() const;
		const_iterator scanlines_end() const;

		range vrange() const;
		range hrange() const;

	private:
		typedef unsigned int count_t;
		typedef std::vector<count_t> sorted_bins_container;

	private:
		const vector_rasterizer &operator =(const vector_rasterizer &);

		void hline(int tg, int ey, int x1, int x2, int dy);
		void jump_xy(int x, int y);
		void jump_x(int x);
		void jumpc(int x, int y);
		void extend_bounds(int x, int y);

	private:
		cell _current;
		cells_container _cells, _x_sorted_cells;
		int _min_x, _min_y, _max_x, _max_y;
		sorted_bins_container _x_bins, _y_counts;
		scanline_cells_container _scanlines;
	};

	struct vector_rasterizer::scanline_cells
	{
		int y;
		vector_rasterizer::cells_iterator begin, end;
	};



	inline const vector_rasterizer::cells_container &vector_rasterizer::cells() const
	{	return _cells;	}

	inline vector_rasterizer::const_iterator vector_rasterizer::scanlines_begin() const
	{	return _scanlines.begin();	}

	inline vector_rasterizer::const_iterator vector_rasterizer::scanlines_end() const
	{	return _scanlines.end();	}

	inline vector_rasterizer::range vector_rasterizer::vrange() const
	{	return std::make_pair(_min_y, _max_y);	}

	inline vector_rasterizer::range vector_rasterizer::hrange() const
	{	return std::make_pair(_min_x, _max_x);	}
}
