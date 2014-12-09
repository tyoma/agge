#pragma once

#include "basics.h"
#include "pixel_formats.h"

#include <algorithm>

namespace aggx
{
	template <typename PixelT = pixel_format::bgra32>
	class blender_solid_color
	{
	public:
		typedef PixelT pixel;

	public:
		explicit blender_solid_color(const rgba8 &color);

		void operator ()(pixel *pixels, int x, int y, unsigned int n) const;
		void operator ()(pixel *pixels, int x, int y, unsigned int n, cover_type cover) const;
		void operator ()(pixel *pixels, int x, int y, unsigned int n, const cover_type *covers) const;

	private:
		const int _alpha, _r, _g, _b;
		pixel _pixel;
	};

	template <typename PixelT>
	inline blender_solid_color<PixelT>::blender_solid_color(const rgba8 &color)
		: _alpha(color.a + 1), _r(color.r), _g(color.g), _b(color.b)
	{
		_pixel.quad.r = color.r;
		_pixel.quad.g = color.g;
		_pixel.quad.b = color.b;
		_pixel.quad.a = color.a;
	}

	template <typename PixelT>
	inline void blender_solid_color<PixelT>::operator ()(pixel *pixels, int /*x*/, int /*y*/, unsigned int n) const
	{
		std::fill_n(pixels, n, _pixel);
	}

	template <typename PixelT>
	inline void blender_solid_color<PixelT>::operator ()(pixel *pixels, int /*x*/, int /*y*/, unsigned int n, cover_type cover) const
	{
		const int alpha = _alpha * cover >> 8;

		if (alpha == 0xFF)
			std::fill_n(pixels, n, _pixel);
		else
			for (; n; --n, ++pixels)
			{
				pixels->quad.b += (_b - pixels->quad.b) * alpha >> 8;
				pixels->quad.g += (_g - pixels->quad.g) * alpha >> 8;
				pixels->quad.r += (_r - pixels->quad.r) * alpha >> 8;
			}
	}

	template <typename PixelT>
	inline void blender_solid_color<PixelT>::operator ()(pixel *pixels, int /*x*/, int /*y*/, unsigned int n, const cover_type *covers) const
	{
		for (; n; --n, ++pixels, ++covers)
		{
			const int alpha = _alpha * *covers >> 8;

			if (0xFF == alpha)
				*pixels = _pixel;
			else
			{
				pixels->quad.b += (_b - pixels->quad.b) * alpha >> 8;
				pixels->quad.g += (_g - pixels->quad.g) * alpha >> 8;
				pixels->quad.r += (_r - pixels->quad.r) * alpha >> 8;
			}
		}
	}
}
