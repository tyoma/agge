#pragma once

#include <agge/pixel.h>
#include <algorithm>

namespace common
{
	class blender_solid_color : agge::noncopyable
	{
	public:
		typedef agge::uint8_t cover_type;
		typedef agge::pixel32 pixel;

	public:
		explicit blender_solid_color(const pixel &components, int alpha);

		void operator ()(pixel *pixels, int x, int y, unsigned int n) const;
		void operator ()(pixel *pixels, int x, int y, unsigned int n, const cover_type *covers) const;

	private:
		pixel _components;
		const int _alpha;
	};



	inline blender_solid_color::blender_solid_color(const pixel &components, int alpha)
		: _components(components), _alpha((alpha << 6) + 505 * alpha / 1000)
	{	_components.components[3] = 0xFF;	}

	inline void blender_solid_color::operator ()(pixel *pixels, int /*x*/, int /*y*/, unsigned int n) const
	{	std::fill_n(pixels, n, _components);	}

	inline void blender_solid_color::operator ()(pixel *pixels, int /*x*/, int /*y*/, unsigned int n, const cover_type *covers) const
	{
		for (; n; --n, ++pixels, ++covers)
		{
			const int alpha = _alpha * *covers;

			if (0x3FFFC0 == alpha)
				*pixels = _components;
			else
			{
				pixels->components[0] += (_components.components[0] - pixels->components[0]) * alpha >> 22;
				pixels->components[1] += (_components.components[1] - pixels->components[1]) * alpha >> 22;
				pixels->components[2] += (_components.components[2] - pixels->components[2]) * alpha >> 22;
			}
		}
	}
}
