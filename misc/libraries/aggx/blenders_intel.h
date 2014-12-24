#pragma once

#include "basics.h"
#include "pixel_formats.h"

#include <emmintrin.h>

namespace aggx
{
	namespace intel
	{
		class blender_solid_color
		{
		public:
			typedef pixel_format::bgra32 pixel;

		public:
			explicit blender_solid_color(const rgba8 &color);

			void operator ()(pixel *pixels, int x, int y, unsigned int n) const;
			void operator ()(pixel *pixels, int x, int y, unsigned int n, cover_type cover) const;
			void operator ()(pixel *pixels, int x, int y, unsigned int n, const cover_type* covers) const;

		private:
			void blend_aligned(pixel *pixels, unsigned int n_quads, const cover_type* covers) const;

		private:
			const unsigned int _color;
			const unsigned int _alpha;
			__m128i _color_u16, _alpha_u16;
		};



		inline blender_solid_color::blender_solid_color(const rgba8 &color)
			: _color((color.a << 24) + (color.r << 16) + (color.g << 8) + color.b),
				_alpha(((1 + color.a) << 16) + (1 + color.a))
		{
			const __m128i zero = _mm_cvtsi32_si128(0);
			_alpha_u16 = _mm_cvtsi32_si128(_alpha);
			_alpha_u16 = _mm_unpacklo_epi32(_alpha_u16, _alpha_u16);
			_alpha_u16 = _mm_unpacklo_epi64(_alpha_u16, _alpha_u16);
			_color_u16 = _mm_cvtsi32_si128(_color);
			_color_u16 = _mm_unpacklo_epi8(_color_u16, zero);
			_color_u16 = _mm_unpacklo_epi64(_color_u16, _color_u16);
		}

		inline void blender_solid_color::operator ()(pixel *pixels, int /*x*/, int /*y*/, unsigned int n) const
		{
			__m128i color_u16 = _mm_cvtsi32_si128(_color);
			color_u16 = _mm_unpacklo_epi32(color_u16, color_u16);
			color_u16 = _mm_unpacklo_epi64(color_u16, color_u16);
			__m128i *p;

			for (n = (n + 3) >> 2, p = reinterpret_cast<__m128i *>(pixels); n; --n, ++p)
				_mm_store_si128(p, color_u16);
		}

		inline void blender_solid_color::operator ()(pixel *pixels, int /*x*/, int /*y*/, unsigned int n, const cover_type* covers) const
		{
			const ptrdiff_t correction = (reinterpret_cast<intptr_t>(pixels) & 0xF) >> 2;

			blend_aligned(pixels - correction, (n + correction + 3) >> 2, covers - correction);
		}

		inline void blender_solid_color::blend_aligned(pixel *pixels, unsigned int n_quads, const cover_type* covers) const
		{
			const __m128i zero = _mm_cvtsi32_si128(0);
			const __m128i alpha_u16 = _mm_load_si128(&_alpha_u16);
			const __m128i color_u16 = _mm_load_si128(&_color_u16);

			for (__m128i *p = reinterpret_cast<__m128i *>(pixels); n_quads; --n_quads, ++p, covers += 4)
			{
				__m128i x32 = _mm_unpacklo_epi8(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(covers)), zero);

				x32 = _mm_srli_epi16(_mm_mullo_epi16(x32, alpha_u16), 8);
				x32 = _mm_unpacklo_epi16(x32, x32);

				const __m128i source = _mm_load_si128(p);

				// X = (Source - Color) * alpha >> 8;
				const __m128i x10 = _mm_srai_epi16(_mm_mullo_epi16(_mm_sub_epi16(_mm_unpacklo_epi8(source, zero), color_u16), _mm_unpacklo_epi32(x32, x32)), 8);
				x32 = _mm_srai_epi16(_mm_mullo_epi16(_mm_sub_epi16(_mm_unpackhi_epi8(source, zero), color_u16), _mm_unpackhi_epi32(x32, x32)), 8);

				// Destination = Source - X;
				_mm_store_si128(p, _mm_sub_epi8(source, _mm_packs_epi16(x10, x32)));
			}
		}
	}
}
