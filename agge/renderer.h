#pragma once

namespace agge
{
	template <unsigned _1_shift, typename CellsIteratorT, typename ScanlineT, typename CalculateAlphaFn>
	inline void sweep_scanline(CellsIteratorT begin, CellsIteratorT end, ScanlineT& scanline, CalculateAlphaFn alpha)
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

			if (cover_m)
				scanline.add_span(x, i->x - x, alpha(cover_m));
		}
	}
}
