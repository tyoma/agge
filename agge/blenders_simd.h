#pragma once

#include "config.h"
#include "pixel.h"

#if defined(AGGE_ARCH_INTEL)
	#include <emmintrin.h>
#elif defined(AGGE_ARCH_ARM)
	#include <arm_neon.h>
#else
	#error "Unsupported architecture"
#endif

namespace agge
{
	namespace simd
	{
#if defined(AGGE_ARCH_INTEL)
		typedef __m128i uint16x8_t;
#elif defined(AGGE_ARCH_ARM)
		typedef ::uint16x8_t uint16x8_t;
#endif

		// This blender requires and assumes the following:
		// 1. Result of blending of empty covers vector (n == 0) is undefined;
		// 2. Covers vector must be accessible beyond the length (n) up to the nearest multiple of 4;
		// 3. Pixels vector must be accessible beyond the length (n) up to the nearest multiple of 4.
		class blender_solid_color
		{
		public:
			typedef pixel32 pixel;
			typedef uint8_t cover_type;

		public:
			blender_solid_color(pixel components, uint8_t alpha);

			void operator ()(pixel *pixels, int x, int y, count_t n) const;
			void operator ()(pixel *pixels, int x, int y, count_t n, const cover_type *covers) const;

		private:
			static void blend4(pixel *pixels, uint16x8_t color_u16, uint16x8_t alpha_u16, unsigned int covers_packed);

		private:
			uint16x8_t _color_u16, _alpha_u16;
			pixel _components;
			static unsigned int _tail_mask[5];
		};



		AGGE_INLINE void blender_solid_color::operator ()(pixel *pixels, int /*x*/, int /*y*/, count_t n,
			const cover_type *covers) const
		{
			const uint16x8_t alpha_u16 = _alpha_u16;
			const uint16x8_t color_u16 = _color_u16;

			for (; n > 4; pixels += 4, covers += 4, n -= 4)
				blend4(pixels, color_u16, alpha_u16, *reinterpret_cast<const unsigned int *>(covers));
			blend4(pixels, color_u16, alpha_u16, *reinterpret_cast<const unsigned int *>(covers)
				& (0xFFFFFFFF >> 8 * (4 - n)));
		}
	}
}
