#include <agge/clipper.h>
#include <agge/filling_rules.h>
#include <agge/rasterizer.h>
#include <agge/renderer_parallel.h>
#include <agge.text/layout.h>
#include <samples/common/font_loader.h>
#include <samples/common/lipsum.h>
#include <samples/common/shell.h>
#include <samples/common/timing.h>

using namespace agge;
using namespace std;

typedef rasterizer< clipper<int> > my_rasterizer;

namespace demo
{
	class TextDrawer : public application
	{
	public:
		TextDrawer(services &s)
			: _renderer(1), _font_loader(s), _text_engine(_font_loader), _ddx(0.0f)
		{	}

	private:
		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			long long counter;
			const rect_i area = { 0, 0, static_cast<int>(surface.width()), static_cast<int>(surface.height()) };

			stopwatch(counter);
				agge::fill(surface, area, solid_color_brush(0, 50, 100));
			timings.clearing += stopwatch(counter);

			_ddx += 0.02f;

			_rasterizer.reset();

			font::ptr f = _text_engine.create_font(L"arial", 16, false, false, font::key::gf_none);
			layout l(c_text_long.c_str(), f);

			l.limit_width(static_cast<real_t>(surface.width()));

			stopwatch(counter);

			_text_engine.render_layout(_rasterizer, l, _ddx, 0.0f);

			double append = stopwatch(counter);

			_rasterizer.sort(true);

			double sort = stopwatch(counter);

			_renderer(surface, 0, _rasterizer, solid_color_brush(255, 255, 255), winding<>());

			double render = stopwatch(counter);

			timings.rasterization += append + sort;
			timings.rendition += render;
		}

	private:
		typedef platform_blender_solid_color solid_color_brush;

	private:
		my_rasterizer _rasterizer;
		renderer_parallel _renderer;
		font_loader _font_loader;
		text_engine<my_rasterizer> _text_engine;
		float _ddx;
	};
}

application *agge_create_application(services &s)
{
	return new demo::TextDrawer(s);
}
