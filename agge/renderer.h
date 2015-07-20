#pragma once

#include "config.h"

namespace agge
{
	template <typename BitmapT, typename BlenderT>
	class renderer
	{
	public:
		typedef typename BlenderT::cover_type cover_type;

	public:
		renderer(BitmapT &bitmap, const BlenderT &blender);

		bool set_y(int y);
		void operator ()(int x, int length, const cover_type *covers);

	private:
		const renderer &operator =(const renderer &rhs);

	private:
		BitmapT &_bitmap;
		const BlenderT &_blender;
		int _y;
		const int _width;
		typename BitmapT::pixel *_row;
	};



	template <typename BitmapT, typename BlenderT>
	inline renderer<BitmapT, BlenderT>::renderer(BitmapT &bitmap, const BlenderT &blender)
		: _bitmap(bitmap), _blender(blender), _width(bitmap.width())
	{	}

	template <typename BitmapT, typename BlenderT>
	inline bool renderer<BitmapT, BlenderT>::set_y(int y)
	{
		if (y < 0 || static_cast<int>(_bitmap.height()) <= y)
			return false;
		_y = y;
		_row = _bitmap.row_ptr(y);
		return true;
	}

	template <typename BitmapT, typename BlenderT>
	inline void renderer<BitmapT, BlenderT>::operator ()(int x, int length, const cover_type *covers)
	{
		if (x < 0)
		{
			length += x;
			covers -= x;
			x = 0;
		}
		if (x + length >= _width)
			length = _width - x;
		if (length > 0)
			_blender(_row + x, x, _y, length, covers);
	}


	template <unsigned _1_shift, typename ScanlineT, typename CellsIteratorT, typename AlphaFn>
	AGGE_INLINE void sweep_scanline(ScanlineT &scanline, CellsIteratorT begin, CellsIteratorT end, const AlphaFn &alpha)
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


	template <typename ScanlineT, typename RasterSourceT, typename AlphaFn>
	inline void render(ScanlineT &scanline, const RasterSourceT &raster, const AlphaFn &alpha, int offset, int step)
	{
		typename RasterSourceT::range vrange = raster.vrange();

		for (int y = vrange.first + offset; y <= vrange.second; y += step)
		{			
			typename RasterSourceT::scanline_cells cells = raster.get_scanline_cells(y);

			if (scanline.begin(y))
			{
				sweep_scanline<RasterSourceT::_1_shift>(scanline, cells.first, cells.second, alpha);
				scanline.commit();
			}
		}
	}


	template <typename BitmapT, typename BlenderT>
	inline void fill(BitmapT &bitmap, const BlenderT &blender)
	{
		const int width = bitmap.width();
		const int height = bitmap.height();

		for (int y = 0; y != height; ++y)
			blender(bitmap.row_ptr(y), 0, y, width);
	}
}
