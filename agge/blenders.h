#pragma once

#include "types.h"

namespace agge
{
	template <typename BaseBlenderT, typename OrderT>
	struct blender_solid_color : BaseBlenderT
	{
		blender_solid_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF);

		static typename BaseBlenderT::pixel make_pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	};



	template <typename BaseBlenderT, typename OrderT>
	inline blender_solid_color<BaseBlenderT, OrderT>::blender_solid_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		: BaseBlenderT(make_pixel(r, g, b, a), a)
	{	}

	template <typename BaseBlenderT, typename OrderT>
	inline typename BaseBlenderT::pixel blender_solid_color<BaseBlenderT, OrderT>::make_pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		typename BaseBlenderT::pixel p;

		p.components[OrderT::R] = r;
		p.components[OrderT::G] = g;
		p.components[OrderT::B] = b;
		p.components[OrderT::A] = a;
		return p;
	}
}
