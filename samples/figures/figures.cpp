#include <agge/clipper.h>
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

	class Figures : public application
	{
	public:
		Figures()
		{
			_line_style.width(4.0f);
			_line_style.set_cap(caps::butt());
			_line_style.set_join(joins::bevel());
			_dash.add_dash(1.0f, 1.0f);
			_dash.dash_start(0.5f);
		}

	private:
		virtual void draw(platform_bitmap &surface, timings &/*timings*/)
		{
			ras.reset();

			_line_style.width(15.0f);
			add_path(ras, assist(qbezier(10.0f, 150.0f, 440.0f, 300.0f, 200.0f, 150.0f, 0.02f), _line_style));
			add_path(ras, assist(arc(105.0f, 140.0f, 95.0f, 1.05f * pi, 1.95f * pi), _line_style));
			_line_style.width(1.0f);
			add_path(ras, assist(assist(rounded_rectangle(10.5f, 200.5f, 110.5f, 250.5f, 7.0f), _dash), _line_style));
			add_path(ras, rounded_rectangle(10.0f, 260.0f, 111.0f, 311.0f, 7.0f));

			ras.sort();

			fill(surface, mkrect<int>(0, 0, surface.width(), surface.height()),
				platform_blender_solid_color(color::make(0, 50, 100)));
			ren(surface, zero(), 0 /*no windowing*/, ras /*mask*/,
				platform_blender_solid_color(color::make(255, 255, 255)), winding<>());
		}

	private:
		rasterizer< clipper<int> > ras;
		renderer ren;
		stroke _line_style;
		dash _dash;
	};
}

application *agge_create_application(services &/*s*/)
{
	return new Figures;
}
