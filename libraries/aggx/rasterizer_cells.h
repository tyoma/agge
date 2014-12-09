#pragma once

#include <vector>
#include "pod_vector.h"

namespace aggx
{
	class rasterizer_cells
	{
	public:
#pragma pack(push, 1)
		struct cell
		{
			int cover, area;
			short x, y;

			static cell initial;
		};
#pragma pack(pop)

	public:
		rasterizer_cells();

		void reset();
		void line(int x1, int y1, int x2, int y2);

		int min_x() const { return m_min_x; }
		int min_y() const { return m_min_y; }
		int max_x() const { return m_max_x; }
		int max_y() const { return m_max_y; }

		void sort_cells();

		unsigned scanline_num_cells(unsigned y) const;
		const cell *scanline_cells(unsigned y) const;

		bool sorted() const { return m_sorted; }

	private:
		struct sorted_bin;

		typedef std::vector<cell> cells_container;
		typedef std::vector<sorted_bin> scanline_blocks_container_type;

	private:
		void switch_cell(int x, int y);
		void force_switch_cell(int x, int y);
		void commit_cell();
		void render_hline(int tg, short ey, int x1, int fy1, int x2, int fy2);

	private:
		cell m_current;
		cells_container m_cells, m_cells_temporary;
		scanline_blocks_container_type m_sorted_bins;
		int m_min_x;
		int m_min_y;
		int m_max_x;
		int m_max_y;
		bool m_sorted;
	};

	struct rasterizer_cells::sorted_bin
	{
		unsigned start;
		unsigned num;
	};



	inline unsigned rasterizer_cells::scanline_num_cells(unsigned y) const 
	{	return m_sorted_bins[y - m_min_y].num;	}

	inline const rasterizer_cells::cell *rasterizer_cells::scanline_cells(unsigned y) const
	{	return m_cells.data() + m_sorted_bins[y - m_min_y].start;	}
}
