#pragma once

#include "math.h"
#include "vector_rasterizer.h"

namespace agge
{
	template <typename ClipperT>
	class rasterizer : private vector_rasterizer
	{
	public:
		using vector_rasterizer::_1_shift;
		using vector_rasterizer::scanline_cells;

	public:
		using vector_rasterizer::reset;

		void move_to(real_t x, real_t y);
		void line_to(real_t x, real_t y);
		void close_polygon();

		using vector_rasterizer::sort;
		using vector_rasterizer::operator [];
		using vector_rasterizer::width;
		using vector_rasterizer::min_y;
		using vector_rasterizer::height;

	private:
		void line(real_t x1, real_t y1, real_t x2, real_t y2);

	private:
		ClipperT _clipper;
		real_t _start_x, _start_y;

	private:
		friend ClipperT;
	};



	template <typename ClipperT>
	inline void rasterizer<ClipperT>::move_to(real_t x, real_t y)
	{	_clipper.move_to(_start_x = x, _start_y = y);	}

	template <typename ClipperT>
	inline void rasterizer<ClipperT>::line_to(real_t x, real_t y)
	{	_clipper.line_to(*this, x, y);	}

	template <typename ClipperT>
	inline void rasterizer<ClipperT>::close_polygon()
	{	line_to(_start_x, _start_y);	}

	template <typename ClipperT>
	inline void rasterizer<ClipperT>::line(real_t x1, real_t y1, real_t x2, real_t y2)
	{	vector_rasterizer::line(real2fixed<_1>(x1), real2fixed<_1>(y1), real2fixed<_1>(x2), real2fixed<_1>(y2));	}
}
