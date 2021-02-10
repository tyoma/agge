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

namespace demo
{
	typedef rasterizer< clipper<int> > my_rasterizer;

	class TextDrawer : public application
	{
	public:
		TextDrawer(services &s)
			: _renderer(3), _font_loader(s), _text_engine(_font_loader), _text(font_style_annotation()),
				_layout(_text_engine), _ddx(0.0f)
		{
			font_style_annotation a = {	font_descriptor::create("arial", 14, regular, false, hint_none),	};

			_text.set_base_annotation(a);
			_text << c_text_long.c_str();
		}

	private:
		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			long long counter;
			const rect_i area = { 0, 0, static_cast<int>(surface.width()), static_cast<int>(surface.height()) };
			rect_r dest = _dest_rect;

			stopwatch(counter);
				agge::fill(surface, area, solid_color_brush(color::make(0, 50, 100)));
				timings.clearing += stopwatch(counter);
				_ddx += 0.02f;
				dest.x1 += _ddx, dest.x2 += _ddx;
				_rasterizer.reset();
			stopwatch(counter);
				_layout.process(_text);
			double stroking = stopwatch(counter);
				_text_engine.render(_rasterizer, _layout, align_near, align_near, dest);
			double append = stopwatch(counter);
				_rasterizer.sort(true);
			double sort = stopwatch(counter);
				_renderer(surface, zero(), 0, _rasterizer, solid_color_brush(color::make(255, 255, 255)), winding<>());
			double render = stopwatch(counter);

			timings.stroking += stroking;
			timings.rasterization += append + sort;
			timings.rendition += render;
		}

		virtual void resize(int width, int height)
		{
			_layout.set_width_limit(static_cast<real_t>(width));
			_dest_rect = create_rect(0.0f, 0.0f, static_cast<real_t>(width), static_cast<real_t>(height));
		}

	private:
		typedef platform_blender_solid_color solid_color_brush;

	private:
		my_rasterizer _rasterizer;
		renderer_parallel _renderer;
		font_loader _font_loader;
		text_engine<my_rasterizer> _text_engine;
		layout _layout;
		richtext_t _text;
		rect_r _dest_rect;
		float _ddx;
	};
}

application *agge_create_application(services &s)
{
	return new demo::TextDrawer(s);
}
