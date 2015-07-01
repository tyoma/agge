#pragma once

#include "config.h"

namespace agge
{
	template <unsigned _1_shift, typename CellsIteratorT, typename ScanlineT, typename CalculateAlphaFn>
	AGGE_INLINE void sweep_scanline(CellsIteratorT begin, CellsIteratorT end, ScanlineT &scanline, const CalculateAlphaFn &alpha)
	{
		int cover = 0;

		if (begin == end)
			return;

		for (CellsIteratorT i = begin; ; )
		{
			int x = i->x, area = 0;

			do
			{
				area += i->area;
				cover += i->cover;
				++i;
			} while (i != end && i->x == x);

			int cover_m = cover << (1 + _1_shift);

			if (area)
			{
				scanline.add_cell(x, alpha(cover_m - area));
				++x;
			}

			if (i == end)
				break;

			int len = i->x - x;

			if (len && cover_m)
				scanline.add_span(x, len, alpha(cover_m));
		}
	}


	template <typename RendererT, typename RasterSourceT, typename CalculateAlphaFn>
	inline void render(RendererT &renderer, const RasterSourceT &raster, const CalculateAlphaFn &alpha,
		int start, int step)
	{
		typename RasterSourceT::range vrange = raster.vrange();

		for (int y = vrange.first + start; y <= vrange.second; y += step)
		{
			typename RasterSourceT::scanline_cells cells = raster.get_scanline_cells(y);
			
			renderer.begin(y);
			sweep_scanline<RasterSourceT::_1_shift>(cells.first, cells.second, renderer, alpha);
			renderer.commit();
		}
	}
}
