#pragma once

#include <agge/vector_rasterizer.h>

#include <vector>
#include "pod_vector.h"

namespace aggx
{
	class rasterizer_cells
	{
	public:
		typedef agge::vector_rasterizer::cell cell;

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
		agge::vector_rasterizer m_vrasterizer;
		bool m_sorted;
	};

	struct rasterizer_cells::sorted_bin
	{
		unsigned start;
		unsigned num;
	};



	inline rasterizer_cells::rasterizer_cells()
		: m_sorted(false)
	{	}

	inline void rasterizer_cells::reset()
	{
		m_cells.clear();
		m_sorted = false;
		m_vrasterizer.reset();
	}

	inline void rasterizer_cells::line(int x1, int y1, int x2, int y2)
	{	m_vrasterizer.line(x1, y1, x2, y2);	}

	inline void rasterizer_cells::sort_cells()
	{
		m_vrasterizer.sort();
		m_sorted = true;
	}

	inline unsigned rasterizer_cells::scanline_num_cells(unsigned y) const 
	{
		const agge::vector_rasterizer::scanline_cells &scanline = *(m_vrasterizer.scanlines_begin() + (y - m_vrasterizer.vrange().first));
		return scanline.end - scanline.begin;
	}

	inline const rasterizer_cells::cell *rasterizer_cells::scanline_cells(unsigned y) const
	{
		const agge::vector_rasterizer::scanline_cells &scanline = *(m_vrasterizer.scanlines_begin() + (y - m_vrasterizer.vrange().first));
		return &*scanline.begin;
	}
}
