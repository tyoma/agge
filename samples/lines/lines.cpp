﻿#include <agge/clipper.h>
#include <agge/curves.h>
#include <agge/dash.h>
#include <agge/figures.h>
#include <agge/filling_rules.h>
#include <agge/path.h>
#include <agge/renderer.h>
#include <agge/rasterizer.h>
#include <agge/stroke.h>
#include <agge/stroke_features.h>

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

	class Lines : public application
	{
	private:
		void knot(real_t x, real_t y)
		{
			add_path(ras, assist(cbezier(x - 25.0f, y - 15.0f, x + 80.0f, y + 200.0f,
				x - 80.0f, y + 200.0f, x + 25.0f, y - 15.0f, 0.01f), line_style));
		}

		virtual void draw(platform_bitmap &surface, timings &/*timings*/)
		{
			ras.reset();

			line_style.width(15.0f);
			line_style.set_cap(caps::triangle(3.0f));
			line_style.set_join(joins::bevel());
			knot(40.0f, 100.0f);

			line_style.width(5.0f);
			line_style.set_cap(caps::butt());
			line_style.set_join(joins::bevel());
			knot(110.0f, 100.0f);

			line_style.width(10.0f);
			line_style.set_cap(caps::triangle(-3.0f));
			line_style.set_join(joins::bevel());
			knot(180.0f, 100.0f);

			line_style.width(10.0f);
			line_style.set_cap(caps::round());
			line_style.set_join(joins::bevel());
			knot(250.0f, 100.0f);

			line_style.width(15.0f);
			line_style.set_cap(caps::round());
			line_style.set_join(joins::bevel());
			knot(320.0f, 100.0f);

			line_style.width(1.0f);
			line_style.set_cap(caps::butt());
			dash_style.remove_all_dashes();
			dash_style.add_dash(1.0f, 1.0f);
			dash_style.dash_start(0.5f);
			add_path(ras, assist(assist(line(19.5f, 300.5f, 319.5f, 300.5f), dash_style), line_style));

			ras.sort();

			fill(surface, mkrect<int>(0, 0, surface.width(), surface.height()),
				platform_blender_solid_color(color::make(0, 50, 100)));
			ren(surface, zero(), 0 /*no windowing*/, ras /*mask*/,
				platform_blender_solid_color(color::make(255, 255, 255)), winding<>());
		}

	private:
		rasterizer< clipper<int> > ras;
		renderer ren;
		stroke line_style;
		dash dash_style;
	};
}

application *agge_create_application(services &/*s*/)
{
	return new Lines;
}
