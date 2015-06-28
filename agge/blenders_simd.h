#pragma once

#include "types.h"

#include <emmintrin.h>

namespace agge
{
	namespace simd
	{
		class blender_solid_color
		{
		public:
			typedef pixel32 pixel;
			typedef uint8_t cover_type;

		public:
			blender_solid_color(pixel components, uint8_t alpha);

			void operator ()(pixel *pixels, unsigned int x, unsigned int y, unsigned int n) const;
			void operator ()(pixel *pixels, unsigned int x, unsigned int y, unsigned int n, const cover_type *covers) const;

		private:
			void blend_aligned(pixel *pixels, unsigned int n_quads, const cover_type* covers) const;

		private:
			__m128i _color_u16, _alpha_u16;
		};
	}
}
