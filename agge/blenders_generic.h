#pragma once

#include "color.h"
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
		blender_solid_color_rgb(color color_);

		void operator ()(pixel *pixels, int x, int y, count_t n) const;
		void operator ()(pixel *pixels, int x, int y, count_t n, const cover_type *covers) const;

	private:
		color _color;
		unsigned int _a;
	};


	template <typename PixelT, typename OrderT>
	inline blender_solid_color_rgb<PixelT, OrderT>::blender_solid_color_rgb(color color_)
		: _color(color_), _a((color_.a << 6) + 505 * color_.a / 1000)
	{
	}

	template <typename PixelT, typename OrderT>
	inline void blender_solid_color_rgb<PixelT, OrderT>::operator ()(pixel *pixels, int /*x*/, int /*y*/, count_t n) const
	{
		pixel ref;

		ref.components[OrderT::R] = _color.r;
		ref.components[OrderT::G] = _color.g;
		ref.components[OrderT::B] = _color.b;
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
				pixels->components[OrderT::R]
					+= static_cast<uint8_t>((_color.r - pixels->components[OrderT::R]) * alpha >> 22);
				pixels->components[OrderT::G]
					+= static_cast<uint8_t>((_color.g - pixels->components[OrderT::G]) * alpha >> 22);
				pixels->components[OrderT::B]
					+= static_cast<uint8_t>((_color.b - pixels->components[OrderT::B]) * alpha >> 22);
			}
			else
			{
				pixels->components[OrderT::R] = _color.r;
				pixels->components[OrderT::G] = _color.g;
				pixels->components[OrderT::B] = _color.b;
			}
		}
	}
}
