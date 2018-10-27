#pragma once

#include "color.h"

namespace agge
{
	template <typename BaseBlenderT, typename OrderT>
	struct blender_solid_color : BaseBlenderT
	{
		explicit blender_solid_color(color color_);

		static typename BaseBlenderT::pixel make_pixel(color color_);
	};



	template <typename BaseBlenderT, typename OrderT>
	inline blender_solid_color<BaseBlenderT, OrderT>::blender_solid_color(color color_)
		: BaseBlenderT(make_pixel(color_), color_.a)
	{	}

	template <typename BaseBlenderT, typename OrderT>
	inline typename BaseBlenderT::pixel blender_solid_color<BaseBlenderT, OrderT>::make_pixel(color color_)
	{
		typename BaseBlenderT::pixel p;

		p.components[OrderT::R] = color_.r;
		p.components[OrderT::G] = color_.g;
		p.components[OrderT::B] = color_.b;
		p.components[OrderT::A] = color_.a;
		return p;
	}
}
