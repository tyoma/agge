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
			: _color_u16(make_color_u16(components)), _alpha_u16(make_alpha_u16(alpha)), _components(components)
		{	}

		void blender_solid_color::operator ()(pixel *pixels, int /*x*/, int /*y*/, count_t n) const
		{
			for (; n; --n, ++pixels)
				*pixels = _components;
		}

		void blender_solid_color::operator ()(pixel *pixels, int /*x*/, int /*y*/, count_t n, const cover_type *covers) const
		{
			n = (n + 3) >> 2;

			const __m128i zero = _mm_setzero_si128();
			const __m128i alpha_u16 = _mm_loadu_si128(&_alpha_u16);
			const __m128i color_u16 = _mm_loadu_si128(&_color_u16);

			for (__m128i *p = reinterpret_cast<__m128i *>(pixels); n; --n, ++p, covers += 4)
			{
				__m128i alpha = _mm_mulhi_epu16(_mm_unpacklo_epi8(zero, _mm_cvtsi32_si128(*reinterpret_cast<const int *>(covers))), alpha_u16);
				
				alpha = _mm_unpacklo_epi16(alpha, alpha);
				
				__m128i source10 = _mm_loadu_si128(p);
				__m128i	source32 = _mm_unpackhi_epi8(source10, zero);
							source10 = _mm_unpacklo_epi8(source10, zero);

				// source -= ((source - color) << 2) * alpha >> 16;
				source32 = _mm_sub_epi16(source32, _mm_mulhi_epi16(_mm_slli_epi16(_mm_sub_epi16(source32, color_u16), 2), _mm_unpackhi_epi32(alpha, alpha)));
				source10 = _mm_sub_epi16(source10, _mm_mulhi_epi16(_mm_slli_epi16(_mm_sub_epi16(source10, color_u16), 2), _mm_unpacklo_epi32(alpha, alpha)));

				_mm_storeu_si128(p, _mm_packus_epi16(source10, source32));
			}
		}
	}
}
