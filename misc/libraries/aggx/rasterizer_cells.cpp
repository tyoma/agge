#include "rasterizer_cells.h"

#include "basics.h"

#include <algorithm>

namespace aggx
{
	rasterizer_cells::rasterizer_cells()
		: m_sorted(false), m_vrasterizer(m_cells)
	{
	}

	void rasterizer_cells::sort_cells()
	{
		/*static*/ const sorted_bin initial_sorted_bin = { 0 };

		if(m_sorted)
			return;

		m_vrasterizer.commit();

		m_cells_temporary.resize(m_cells.size());

		unsigned start = 0;
		m_sorted_bins.assign(max_x() - min_x() + 1, initial_sorted_bin);
		for (const cell *i = m_cells.data(), *e = m_cells.data() + m_cells.size(); i != e; ++i)
			++m_sorted_bins[i->x - min_x()].start;
		for (scanline_blocks_container_type::iterator i = m_sorted_bins.begin(); i != m_sorted_bins.end(); ++i)
		{
			unsigned v = i->start;
			i->start = start;
			start += v;
		}
		for (cells_container::const_iterator i = m_cells.begin(), e = m_cells.end(); i != e; ++i)
		{
			sorted_bin& scanline = m_sorted_bins[i->x - min_x()];
			m_cells_temporary[scanline.start + scanline.num++] = *i;
		}

		start = 0;
		m_sorted_bins.assign(max_y() - min_y() + 1, initial_sorted_bin);
		for (const cell *i = m_cells.data(), *e = m_cells.data() + m_cells.size(); i != e; ++i)
			++m_sorted_bins[i->y - min_y()].start;
		for (scanline_blocks_container_type::iterator i = m_sorted_bins.begin(); i != m_sorted_bins.end(); ++i)
		{
			unsigned v = i->start;
			i->start = start;
			start += v;
		}
		for (cells_container::const_iterator i = m_cells_temporary.begin(), e = m_cells_temporary.end(); i != e; ++i)
		{
			sorted_bin& scanline = m_sorted_bins[i->y - min_y()];
			m_cells[scanline.start + scanline.num++] = *i;
		}

		m_sorted = true;
	}
}
