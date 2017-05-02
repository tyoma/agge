#include <agge/filling_rules.h>
#include <agge/clipper.h>
#include <agge/renderer.h>
#include <agge/rasterizer.h>

#include <samples/common/shell.h>

using namespace agge;

namespace
{
	template <typename T>
	rect<T> mkrect(T x1, T y1, T x2, T y2)
	{
		rect<T> r = { x1, y1, x2, y2 };
		return r;
	}

	class Figures : public application
	{
	private:
		virtual void draw(platform_bitmap &surface, timings &/*timings*/)
		{
			ras.reset();

			ras.move_to(10.0f, 50.0f);
			ras.line_to(190.0f, 60.0f);
			ras.line_to(20.0f, 130.0f);
			ras.close_polygon();

			ras.sort();

			fill(surface, mkrect<int>(0, 0, surface.width(), surface.height()), platform_blender_solid_color(0, 50, 100));
			ren(surface, 0 /*no windowing*/, ras /*mask*/, platform_blender_solid_color(255, 255, 255), winding<>());
		}

	private:
		rasterizer< clipper<int> > ras;
		renderer ren;
	};
}

application *agge_create_application()
{
	return new Figures;
}
