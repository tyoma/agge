#include <agge/filling_rules.h>
#include <agge/clipper.h>
#include <agge/renderer.h>
#include <agge/rasterizer.h>
#include <agge/blenders_simd.h>

#include <samples/common/shell.h>

using namespace agge;

namespace
{
	class blender_solid_color : public simd::blender_solid_color
	{
	public:
		blender_solid_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
			: simd::blender_solid_color(make_pixel(r, g, b, a), a)
		{	}

	private:
		pixel make_pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		{
			pixel p = { b, g, r, a };
			return p;
		}
	};

	template <typename LinesSinkT, typename PathT>
	void add_path(LinesSinkT &sink, PathT &path)
	{
		using namespace agge;

		real_t x, y;

		path.rewind(0);
		for (int command; command = path.vertex(&x, &y), path_command_stop != command; )
			add_polyline_vertex(sink, x, y, command);
	}

	template <typename T>
	rect<T> mkrect(T x1, T y1, T x2, T y2)
	{
		rect<T> r = { x1, y1, x2, y2 };
		return r;
	}

	class Figures : public Drawer
	{
	private:
		virtual void draw(::bitmap &surface, Timings &/*timings*/)
		{
			ras.reset();

			ras.move_to(10.0f, 10.0f);
			ras.line_to(190.0f, 20.0f);
			ras.line_to(20.0f, 90.0f);
			ras.close_polygon();

			ras.sort();

			fill(surface, mkrect<int>(0, 0, surface.width(), surface.height()), blender_solid_color(0, 50, 100));
			ren(surface, 0 /*no windowing*/, ras /*mask*/, blender_solid_color(255, 255, 255), winding<>());
		}

	private:
		rasterizer< clipper<int> > ras;
		renderer ren;
	};
}

int main()
{
	Figures d;
	MainDialog dialog(d);

	MainDialog::PumpMessages();
}
