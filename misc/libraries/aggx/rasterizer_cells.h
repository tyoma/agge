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
		agge::vector_rasterizer m_vrasterizer;
		bool m_sorted;
	};



	inline rasterizer_cells::rasterizer_cells()
		: m_sorted(false)
	{	}

	inline void rasterizer_cells::reset()
	{
		m_sorted = false;
		m_vrasterizer.reset();
	}

	inline void rasterizer_cells::line(int x1, int y1, int x2, int y2)
	{	m_vrasterizer.line(x1, y1, x2, y2);	}

	inline void rasterizer_cells::sort_cells()
	{
		if (!m_sorted)
			m_vrasterizer.sort();
		m_sorted = true;
	}

	inline unsigned rasterizer_cells::scanline_num_cells(unsigned y) const 
	{	return m_vrasterizer.get_scanline_cells(y).second;	}

	inline const rasterizer_cells::cell *rasterizer_cells::scanline_cells(unsigned y) const
	{	return m_vrasterizer.get_scanline_cells(y).first;	}
}
