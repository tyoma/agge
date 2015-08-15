#pragma once

#include "pod_vector.h"
#include "types.h"

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

		struct scanline_cells;

		typedef pod_vector<cell> cells_container;
		typedef cells_container::const_iterator const_cells_iterator;

	public:
		vector_rasterizer();

		void reset();

		void line(int x1, int y1, int x2, int y2);
		void commit();
		const cells_container &cells() const;
		void sort();
		bool sorted() const;

		scanline_cells operator [](int y) const;
		int width() const;
		int min_y() const;
		int height() const;

	private:
		struct sorted_bin;
		typedef pod_vector<sorted_bin> sorted_bins_container;

	private:
		void hline(int tg, int ey, int x1, int x2, int dy);
		void jump_xy(int x, int y);
		void jump_x(int x);
		void jumpc(int x, int y);
		void extend_bounds(int x, int y);

	private:
		cell _current;
		sorted_bins_container _scanlines;
		cells_container _cells, _x_sorted_cells;
		int _min_x, _min_y, _max_x, _max_y;
		bool _sorted;
	};

	struct vector_rasterizer::scanline_cells
	{
		vector_rasterizer::const_cells_iterator first;
		vector_rasterizer::const_cells_iterator second;
	};

	struct vector_rasterizer::sorted_bin
	{
		count_t start;
		count_t length;
	};



	inline const vector_rasterizer::cells_container &vector_rasterizer::cells() const
	{	return _cells;	}

	inline bool vector_rasterizer::sorted() const
	{	return _sorted;	}

	inline vector_rasterizer::scanline_cells vector_rasterizer::operator [](int y) const
	{
		const sorted_bin scanline = _scanlines[y - _min_y];
		const const_cells_iterator start = _cells.begin() + scanline.start;
		const scanline_cells sc = { start, start + scanline.length };

		return sc;
	}

	inline int vector_rasterizer::width() const
	{	return _max_x >= _min_x ? _max_x - _min_x + 1 : 0;	}

	inline int vector_rasterizer::min_y() const
	{	return _min_y;	}

	inline int vector_rasterizer::height() const
	{	return _max_y >= _min_y ? _max_y - _min_y + 1 : 0;	}
}
