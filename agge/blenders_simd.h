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

			void blender_solid_color::operator ()(pixel *pixels, int x, int y, unsigned int n) const;
			void operator ()(pixel *pixels, unsigned int x, unsigned int y, unsigned int n, const cover_type *covers) const;

		private:
			static __m128i make_color_u16(pixel components);
			static __m128i make_alpha_u16(unsigned short alpha);
			void blend_aligned(pixel *pixels, unsigned int n_quads, const cover_type* covers) const;

		private:
			__m128i _color_u16, _alpha_u16;
		};



		inline blender_solid_color::blender_solid_color(pixel components, uint8_t alpha)
			: _color_u16(make_color_u16(components)), _alpha_u16(make_alpha_u16(alpha))
		{	}

		inline __m128i blender_solid_color::make_color_u16(pixel components)
		{
			__m128i result = _mm_set_epi16(0, 0, 0, 0, components.c3, components.c2, components.c1, components.c0);
			return _mm_unpacklo_epi64(result, result);
		}

		inline __m128i blender_solid_color::make_alpha_u16(unsigned short alpha)
		{
			alpha = (alpha << 6) + 505 * alpha / 1000;
			return _mm_set_epi16(alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha);
		}


		inline void blender_solid_color::operator ()(pixel *pixels, int /*x*/, int /*y*/, unsigned int n) const
		{
			// UNTESTED !!!
			__m128i color_u8 = _mm_packus_epi16(_color_u16, _color_u16);
			__m128i *p;

			for (n = (n + 3) >> 2, p = reinterpret_cast<__m128i *>(pixels); n; --n, ++p)
				_mm_store_si128(p, color_u8);
		}

		inline void blender_solid_color::operator ()(pixel *pixels, unsigned int /*x*/, unsigned int /*y*/, unsigned int n,
			const cover_type *covers) const
		{
			// UNTESTED !!!
			const ptrdiff_t correction = (reinterpret_cast<intptr_t>(pixels) & 0xF) >> 2;

			blend_aligned(pixels - correction, (n + correction + 3) >> 2, covers - correction);
		}

		inline void blender_solid_color::blend_aligned(pixel *pixels, unsigned int n_quads, const cover_type* covers) const
		{
			const __m128i zero = _mm_setzero_si128();
			const __m128i alpha_u16 = _mm_load_si128(&_alpha_u16);
			const __m128i color_u16 = _mm_load_si128(&_color_u16);

			for (__m128i *p = reinterpret_cast<__m128i *>(pixels); n_quads; --n_quads, ++p, covers += 4)
			{
				__m128i alpha = _mm_mulhi_epu16(_mm_unpacklo_epi8(zero, _mm_loadl_epi64(reinterpret_cast<const __m128i*>(covers))), alpha_u16);
				
				alpha = _mm_unpacklo_epi16(alpha, alpha);
				
				__m128i source10 = _mm_load_si128(p);
				__m128i	source32 = _mm_unpackhi_epi8(source10, zero);
							source10 = _mm_unpacklo_epi8(source10, zero);

				// source -= ((source - color) << 2) * alpha >> 16;
				source32 = _mm_sub_epi16(source32, _mm_mulhi_epi16(_mm_slli_epi16(_mm_sub_epi16(source32, color_u16), 2), _mm_unpackhi_epi32(alpha, alpha)));
				source10 = _mm_sub_epi16(source10, _mm_mulhi_epi16(_mm_slli_epi16(_mm_sub_epi16(source10, color_u16), 2), _mm_unpacklo_epi32(alpha, alpha)));

				_mm_store_si128(p, _mm_packus_epi16(source10, source32));
			}
		}
	}
}
