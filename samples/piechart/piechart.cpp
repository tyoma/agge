#include <agge/clipper.h>
#include <agge/curves.h>
#include <agge/filling_rules.h>
#include <agge/path.h>
#include <agge/renderer.h>
#include <agge/rasterizer.h>

#include <samples/common/shell.h>
#include <samples/common/timing.h>
#include <utility>

using namespace agge;
using namespace std;

namespace
{
	template <typename T>
	rect<T> mkrect(T x1, T y1, T x2, T y2)
	{
		rect<T> r = { x1, y1, x2, y2 };
		return r;
	}

	const color c_palette[] = {
		color::make(230, 85, 13),
		color::make(253, 141, 60),
		color::make(253, 174, 107),
		color::make(253, 208, 162),

		color::make(49, 163, 84),
		color::make(116, 196, 118),
		color::make(161, 217, 155),
		color::make(199, 233, 192),

		color::make(107, 174, 214),
		color::make(158, 202, 225),
		color::make(198, 219, 239),

		color::make(117, 107, 177),
		color::make(158, 154, 200),
		color::make(188, 189, 220),
		color::make(218, 218, 235),
	};

	const pair<real_t, color> c_segments[] = {
		make_pair(0.4f, c_palette[0]),
		make_pair(0.25f, c_palette[4]),
		make_pair(0.2f, c_palette[8]),
		make_pair(0.11f, c_palette[11]),
		make_pair(0.02f, c_palette[12]),
		make_pair(0.01f, c_palette[2]),
	};

	joined_path<arc, arc> pie_segment(real_t cx, real_t cy, real_t outer_r, real_t inner_r, real_t start, real_t end)
	{	return join(arc(cx, cy, outer_r, start, end), arc(cx, cy, inner_r, end, start));	}

	class Figures : public application
	{
	private:
		template <typename IteratorT>
		void drawPie(platform_bitmap &surface, real_t cx, real_t cy, real_t r, IteratorT b, IteratorT e)
		{
			for (real_t a = -pi / 2; b != e; ++b)
			{
				real_t d = 2 * pi * b->first;

				add_path(ras, pie_segment(cx, cy, 1.5f * r, 0.5f * r, a, a + d));
				ras.close_polygon();
				a += d;
				ras.sort();
				ren(surface, zero(), 0 /*no windowing*/, ras /*mask*/, platform_blender_solid_color(b->second), winding<>());
				ras.reset();
			}
		}

		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			long long counter;

			ras.reset();

			stopwatch(counter);
				fill(surface, mkrect<int>(0, 0, surface.width(), surface.height()),
					platform_blender_solid_color(color::make(0, 50, 100)));
			timings.clearing += stopwatch(counter);

			stopwatch(counter);
				drawPie(surface, surface.width() * 0.5f, surface.height() * 0.5f, 220.0f, c_segments, c_segments + 6);
			timings.rendition += stopwatch(counter);
		}

	private:
		rasterizer< clipper<int> > ras;
		renderer ren;
	};
}

application *agge_create_application(services &/*s*/)
{
	return new Figures;
}
