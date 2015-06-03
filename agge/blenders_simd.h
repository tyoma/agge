#pragma once

namespace agge
{
	typedef unsigned char uint8_t;

	struct rgba
	{
		rgba(uint8_t r, uint8_t g, uint8_t b);

		uint8_t r, g, b, a;
	};

	namespace simd
	{
		class blender_solid_color
		{
		public:
			typedef uint8_t pixel[4];
			typedef uint8_t cover_type;

		public:
			blender_solid_color(rgba color);

			void operator ()(pixel *pixels, unsigned int x, unsigned int y, unsigned int n, const cover_type *covers);

		private:
			rgba _color;
		};



		inline blender_solid_color::blender_solid_color(rgba color)
			: _color(color)
		{	}

		inline void blender_solid_color::operator ()(pixel *pixels, unsigned int /*x*/, unsigned int /*y*/, unsigned int n,
			const cover_type *covers)
		{
			for (; n; ++pixels, --n, ++covers)
			{
				(*pixels)[0] += (_color.r - (*pixels)[0]) * (1 + *covers) >> 8;
				(*pixels)[1] += (_color.g - (*pixels)[1]) * (1 + *covers) >> 8;
				(*pixels)[2] += (_color.b - (*pixels)[2]) * (1 + *covers) >> 8;
			}
		}
	}

	inline rgba::rgba(uint8_t r_, uint8_t g_, uint8_t b_)
		: r(r_), g(g_), b(b_)
	{
	}
}
