#pragma once

#include "pixel.h"

namespace agge
{
	template <typename PixelT, typename OrderT>
	class blender_solid_color_rgb
	{
	public:
		typedef PixelT pixel;
		typedef uint8_t cover_type;

	public:
		blender_solid_color_rgb(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF);

		void operator ()(pixel *pixels, int x, int y, count_t n) const;
		void operator ()(pixel *pixels, int x, int y, count_t n, const cover_type *covers) const;

	private:
		uint8_t _r, _g, _b;
		unsigned int _a;
	};


	template <typename PixelT, typename OrderT>
	inline blender_solid_color_rgb<PixelT, OrderT>::blender_solid_color_rgb(uint8_t r, uint8_t g, uint8_t b,
			uint8_t a)
		: _r(r), _g(g), _b(b), _a((a << 6) + 505 * a / 1000)
	{
	}

	template <typename PixelT, typename OrderT>
	inline void blender_solid_color_rgb<PixelT, OrderT>::operator ()(pixel *pixels, int /*x*/, int /*y*/, count_t n) const
	{
		pixel ref;

		ref.components[OrderT::R] = _r;
		ref.components[OrderT::G] = _g;
		ref.components[OrderT::B] = _b;
		for (; n; --n, ++pixels)
			*pixels = ref;
	}

	template <typename PixelT, typename OrderT>
	inline void blender_solid_color_rgb<PixelT, OrderT>::operator ()(pixel *pixels, int /*x*/, int /*y*/, count_t n,
		const cover_type *covers) const
	{
		for (; n; --n, ++pixels)
		{
			const int alpha = _a * *covers++;

			if (0x3FFFC0 ^ alpha)
			{
				pixels->components[OrderT::R] += static_cast<uint8_t>((_r - pixels->components[OrderT::R]) * alpha >> 22);
				pixels->components[OrderT::G] += static_cast<uint8_t>((_g - pixels->components[OrderT::G]) * alpha >> 22);
				pixels->components[OrderT::B] += static_cast<uint8_t>((_b - pixels->components[OrderT::B]) * alpha >> 22);
			}
			else
			{
				pixels->components[OrderT::R] = _r;
				pixels->components[OrderT::G] = _g;
				pixels->components[OrderT::B] = _b;
			}
		}
	}
}
