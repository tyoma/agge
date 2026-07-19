#include <agge/blenders_simd.h>

namespace agge
{
	namespace simd
	{
		unsigned int blender_solid_color::_tail_mask[5] = {
			0x00000000, 0x000000FF, 0x0000FFFF, 0x00FFFFFF, 0xFFFFFFFF,
		};

		namespace
		{
			uint16x8_t make_color_u16(pixel32 components)
			{
				uint16x8_t t = _mm_cvtsi32_si128(reinterpret_cast<const unsigned int &>(components));

				t = _mm_unpacklo_epi8(t, _mm_setzero_si128());
				return _mm_unpacklo_epi64(t, t);
			}

			uint16x8_t make_alpha_u16(unsigned int alpha)
			{
				alpha = (alpha << 6) + 505 * alpha / 1000;
				uint16x8_t t = _mm_shufflelo_epi16(_mm_cvtsi32_si128(alpha), 0);
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

		void blender_solid_color::blend4(pixel *pixels, uint16x8_t color_u16, uint16x8_t alpha_u16, unsigned int covers_packed)
		{
			uint16x8_t alpha = _mm_mulhi_epu16(_mm_unpacklo_epi8(_mm_setzero_si128(), _mm_cvtsi32_si128(covers_packed)),
				alpha_u16);

			alpha = _mm_unpacklo_epi16(alpha, alpha);

			uint16x8_t source10 = _mm_loadu_si128(reinterpret_cast<uint16x8_t *>(pixels));
			uint16x8_t	source32 = _mm_unpackhi_epi8(source10, _mm_setzero_si128());
						source10 = _mm_unpacklo_epi8(source10, _mm_setzero_si128());

			// source -= ((source - color) << 2) * alpha >> 16;
			source32 = _mm_sub_epi16(source32, _mm_mulhi_epi16(_mm_slli_epi16(_mm_sub_epi16(source32, color_u16), 2),
				_mm_unpackhi_epi32(alpha, alpha)));
			source10 = _mm_sub_epi16(source10, _mm_mulhi_epi16(_mm_slli_epi16(_mm_sub_epi16(source10, color_u16), 2),
				_mm_unpacklo_epi32(alpha, alpha)));

			_mm_storeu_si128(reinterpret_cast<uint16x8_t *>(pixels), _mm_packus_epi16(source10, source32));
		}
	}
}
