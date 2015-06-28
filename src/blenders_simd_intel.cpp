#include "agge/blenders_simd.h"

namespace agge
{
	namespace simd
	{
		namespace
		{
			__m128i make_color_u16(pixel32 components)
			{
				__m128i t = _mm_cvtsi32_si128(reinterpret_cast<const unsigned int &>(components));

				t = _mm_unpacklo_epi8(t, _mm_setzero_si128());
				return _mm_unpacklo_epi64(t, t);
			}

			__m128i make_alpha_u16(unsigned int alpha)
			{
				alpha = (alpha << 6) + 505 * alpha / 1000;
				__m128i t = _mm_shufflelo_epi16(_mm_cvtsi32_si128(alpha), 0);
				return _mm_unpacklo_epi64(t, t);
			}
		}

		blender_solid_color::blender_solid_color(pixel components, uint8_t alpha)
			: _color_u16(make_color_u16(components)), _alpha_u16(make_alpha_u16(alpha))
		{	}

		void blender_solid_color::operator ()(pixel *pixels, unsigned int /*x*/, unsigned int /*y*/, unsigned int n) const
		{
			// UNTESTED !!!
			__m128i color_u8 = _mm_packus_epi16(_color_u16, _color_u16);
			__m128i *p;

			for (n = (n + 3) >> 2, p = reinterpret_cast<__m128i *>(pixels); n; --n, ++p)
				_mm_store_si128(p, color_u8);
		}

		void blender_solid_color::operator ()(pixel *pixels, unsigned int /*x*/, unsigned int /*y*/, unsigned int n,
			const cover_type *covers) const
		{
			const unsigned int correction = static_cast<unsigned int>((reinterpret_cast<uintptr_t>(pixels) & 0xF) >> 2);

			pixels = reinterpret_cast<pixel *>(reinterpret_cast<uintptr_t>(pixels) & ~static_cast<uintptr_t>(0xF));
			blend_aligned(pixels, (n + correction + 3) >> 2, covers - correction);
		}

		void blender_solid_color::blend_aligned(pixel *pixels, unsigned int n_quads, const cover_type* covers) const
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
