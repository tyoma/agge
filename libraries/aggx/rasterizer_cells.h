#pragma once

#include <aggx2/vector_rasterizer.h>

#include <vector>
#include "pod_vector.h"

namespace aggx
{
	class rasterizer_cells
	{
	public:
		typedef vector_rasterizer::cell cell;

	public:
		rasterizer_cells();

		void reset();
		void line(int x1, int y1, int x2, int y2);

		int min_x() const { return m_vrasterizer.hrange().first; }
		int min_y() const { return m_vrasterizer.vrange().first; }
		int max_x() const { return m_vrasterizer.hrange().second; }
		int max_y() const { return m_vrasterizer.vrange().second; }

		void sort_cells();

		unsigned scanline_num_cells(unsigned y) const;
		const cell *scanline_cells(unsigned y) const;

		bool sorted() const { return m_sorted; }

	private:
		struct sorted_bin;

		typedef std::vector<cell> cells_container;
		typedef std::vector<sorted_bin> scanline_blocks_container_type;

	private:
		cells_container m_cells, m_cells_temporary;
		scanline_blocks_container_type m_sorted_bins;
		vector_rasterizer m_vrasterizer;
		bool m_sorted;
	};

	struct rasterizer_cells::sorted_bin
	{
		unsigned start;
		unsigned num;
	};


	inline void rasterizer_cells::reset()
	{
		m_cells.clear();
		m_sorted = false;
		m_vrasterizer.reset();
	}

	inline void rasterizer_cells::line(int x1, int y1, int x2, int y2)
	{	m_vrasterizer.line(x1, y1, x2, y2);	}

	inline unsigned rasterizer_cells::scanline_num_cells(unsigned y) const 
	{	return m_sorted_bins[y - m_vrasterizer.vrange().first].num;	}

	inline const rasterizer_cells::cell *rasterizer_cells::scanline_cells(unsigned y) const
	{	return m_cells.data() + m_sorted_bins[y - m_vrasterizer.vrange().first].start;	}
}
