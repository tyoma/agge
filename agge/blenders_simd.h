#pragma once

namespace agge
{
	typedef unsigned char uint8_t;

	struct rgba
	{
		rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF);

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
#pragma warning(disable: 4244)
		inline void blender_solid_color::operator ()(pixel *pixels, unsigned int /*x*/, unsigned int /*y*/, unsigned int n,
			const cover_type *covers)
		{
			for (; n; ++pixels, --n, ++covers)
			{
				int alpha = (*covers) * (_color.a);
				(*pixels)[0] += ((_color.r - (*pixels)[0]) * alpha) / (0xff * 0xff);
				(*pixels)[1] += ((_color.g - (*pixels)[1]) * alpha) / (0xff * 0xff);
				(*pixels)[2] += ((_color.b - (*pixels)[2]) * alpha) / (0xff * 0xff);
			}
		}
#pragma warning(default: 4244)
	}

	inline rgba::rgba(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_)
		: r(r_), g(g_), b(b_), a(a_)
	{
	}
}
