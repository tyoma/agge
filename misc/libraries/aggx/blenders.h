#pragma once

#include "basics.h"

#include <agge/types.h>
#include <algorithm>

namespace aggx
{
	class blender_solid_color
	{
	public:
		typedef agge::uint8_t cover_type;
		typedef agge::pixel32 pixel;

	public:
		explicit blender_solid_color(const pixel &components, int alpha);

		void operator ()(pixel *pixels, unsigned int x, unsigned int y, unsigned int n) const;
		void operator ()(pixel *pixels, unsigned int x, unsigned int y, unsigned int n, const cover_type *covers) const;

	private:
		const pixel _components;
		const int _alpha;
	};



	inline blender_solid_color::blender_solid_color(const pixel &components, int alpha)
		: _components(components), _alpha((alpha << 6) + 505 * alpha / 1000)
	{	}

	inline void blender_solid_color::operator ()(pixel *pixels, unsigned int /*x*/, unsigned int /*y*/, unsigned int n) const
	{	std::fill_n(pixels, n, _components);	}

	inline void blender_solid_color::operator ()(pixel *pixels, unsigned int /*x*/, unsigned int /*y*/, unsigned int n, const cover_type *covers) const
	{
		for (; n; --n, ++pixels, ++covers)
		{
			const int alpha = _alpha * *covers;

			pixels->c0 += (_components.c0 - pixels->c0) * alpha >> 22;
			pixels->c1 += (_components.c1 - pixels->c1) * alpha >> 22;
			pixels->c2 += (_components.c2 - pixels->c2) * alpha >> 22;
		}
	}
}
