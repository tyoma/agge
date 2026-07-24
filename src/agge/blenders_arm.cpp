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
				const uint8x8_t c = vcreate_u8(reinterpret_cast<const unsigned int &>(components));
				const uint16x8_t c_u16 = vmovl_u8(c);

				return vcombine_u16(vget_low_u16(c_u16), vget_low_u16(c_u16));
			}

			uint16x8_t make_alpha_u16(unsigned int alpha)
			{
				alpha = (alpha << 6) + 505 * alpha / 1000;
				return vdupq_n_u16(static_cast<uint16_t>(alpha));
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
			const uint16x4_t covers_u16 = vget_low_u16(vshll_n_u8(vcreate_u8(covers_packed), 8));
			const int16x4_t alpha = vreinterpret_s16_u16(vshrn_n_u32(vmull_u16(covers_u16, vget_low_u16(alpha_u16)), 16));

			const int16x8_t alpha10 = vcombine_s16(vdup_lane_s16(alpha, 0), vdup_lane_s16(alpha, 1));
			const int16x8_t alpha32 = vcombine_s16(vdup_lane_s16(alpha, 2), vdup_lane_s16(alpha, 3));

			const uint8x16_t source_u8 = vld1q_u8(reinterpret_cast<uint8_t *>(pixels));
			int16x8_t source10 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(source_u8)));
			int16x8_t source32 = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(source_u8)));

			// source -= ((source - color) << 2 * alpha) >> 16
			source10 = vsubq_s16(source10, vqdmulhq_s16(vshlq_n_s16(vsubq_s16(source10, vreinterpretq_s16_u16(color_u16)), 1), alpha10));
			source32 = vsubq_s16(source32, vqdmulhq_s16(vshlq_n_s16(vsubq_s16(source32, vreinterpretq_s16_u16(color_u16)), 1), alpha32));

			vst1q_u8(reinterpret_cast<uint8_t *>(pixels), vcombine_u8(vqmovun_s16(source10), vqmovun_s16(source32)));
		}
	}
}
