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
			uint8x8_t covers = vdup_n_u8(0);
			covers = vset_lane_u8(static_cast<uint8_t>(covers_packed >> 0), covers, 0);
			covers = vset_lane_u8(static_cast<uint8_t>(covers_packed >> 8), covers, 1);
			covers = vset_lane_u8(static_cast<uint8_t>(covers_packed >> 16), covers, 2);
			covers = vset_lane_u8(static_cast<uint8_t>(covers_packed >> 24), covers, 3);

			const uint16x4_t covers_u16 = vget_low_u16(vshlq_n_u16(vmovl_u8(covers), 8));
			const uint16x4_t alpha = vshrn_n_u32(vmull_u16(covers_u16, vget_low_u16(alpha_u16)), 16);

			const uint16_t a0 = vget_lane_u16(alpha, 0);
			const uint16_t a1 = vget_lane_u16(alpha, 1);
			const uint16_t a2 = vget_lane_u16(alpha, 2);
			const uint16_t a3 = vget_lane_u16(alpha, 3);

			const int16x8_t alpha10 = vreinterpretq_s16_u16(vcombine_u16(vdup_n_u16(a0), vdup_n_u16(a1)));
			const int16x8_t alpha32 = vreinterpretq_s16_u16(vcombine_u16(vdup_n_u16(a2), vdup_n_u16(a3)));

			const uint8x16_t source_u8 = vld1q_u8(reinterpret_cast<uint8_t *>(pixels));
			const int16x8_t source10 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(source_u8)));
			const int16x8_t source32 = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(source_u8)));

			const int16x8_t d10 = vshlq_n_s16(vsubq_s16(source10, vreinterpretq_s16_u16(color_u16)), 2);
			const int16x8_t d32 = vshlq_n_s16(vsubq_s16(source32, vreinterpretq_s16_u16(color_u16)), 2);

			const int32x4_t m10l = vmull_s16(vget_low_s16(d10), vget_low_s16(alpha10));
			const int32x4_t m10h = vmull_s16(vget_high_s16(d10), vget_high_s16(alpha10));
			const int32x4_t m32l = vmull_s16(vget_low_s16(d32), vget_low_s16(alpha32));
			const int32x4_t m32h = vmull_s16(vget_high_s16(d32), vget_high_s16(alpha32));

			const int16x8_t c10 = vcombine_s16(vshrn_n_s32(m10l, 16), vshrn_n_s32(m10h, 16));
			const int16x8_t c32 = vcombine_s16(vshrn_n_s32(m32l, 16), vshrn_n_s32(m32h, 16));

			const int16x8_t r10 = vsubq_s16(source10, c10);
			const int16x8_t r32 = vsubq_s16(source32, c32);

			const uint8x8_t result10 = vqmovun_s16(r10);
			const uint8x8_t result32 = vqmovun_s16(r32);

			vst1q_u8(reinterpret_cast<uint8_t *>(pixels), vcombine_u8(result10, result32));
		}
	}
}
