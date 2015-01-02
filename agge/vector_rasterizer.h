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
#pragma pack(pop)

		typedef std::vector<cell> cells_container;
		typedef unsigned int count_t;
		typedef std::pair<int, int> range;
		typedef std::pair<const cell *, count_t> scanline_cells;

	public:
		vector_rasterizer();
		
		void reset();

		void line(int x1, int y1, int x2, int y2);
		void commit();
		const cells_container &cells() const;
		void sort();
		scanline_cells get_scanline_cells(int y) const;
		bool sorted() const;

		range vrange() const;
		range hrange() const;

	private:
		struct sorted_bin;
		typedef std::vector<sorted_bin> sorted_bins_container;
		typedef cells_container::const_iterator const_cells_iterator;

	private:
		const vector_rasterizer &operator =(const vector_rasterizer &);

		void hline(int tg, int ey, int x1, int x2, int dy);
		void jump_xy(int x, int y);
		void jump_x(int x);
		void jumpc(int x, int y);
		void extend_bounds(int x, int y);

	private:
		bool _sorted;
		cell _current;
		cells_container _cells, _x_sorted_cells;
		sorted_bins_container _scanlines;
		int _min_x, _min_y, _max_x, _max_y;
	};

	struct vector_rasterizer::sorted_bin
	{
		vector_rasterizer::count_t start;
		vector_rasterizer::count_t length;
	};



	inline const vector_rasterizer::cells_container &vector_rasterizer::cells() const
	{	return _cells;	}

	inline vector_rasterizer::scanline_cells vector_rasterizer::get_scanline_cells(int y) const
	{
		const sorted_bin &scanline = _scanlines[y - _min_y];
		return std::make_pair(&_cells[scanline.start], scanline.length);
	}

	inline bool vector_rasterizer::sorted() const
	{	return _sorted;	}

	inline vector_rasterizer::range vector_rasterizer::vrange() const
	{	return std::make_pair(_min_y, _max_y);	}

	inline vector_rasterizer::range vector_rasterizer::hrange() const
	{	return std::make_pair(_min_x, _max_x);	}
}
