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
		TextDrawer()
			: _renderer(1), _font_engine(_font_loader),
				_font(_font_engine.create_font(L"tahoma", 14, false, false, font_engine_base::gf_strong)),
				_layout(c_text_long.c_str(), _font), _ddx(0.0f)
		{	}

	private:
		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			long long counter;
			const rect_i area = { 0, 0, static_cast<int>(surface.width()), static_cast<int>(surface.height()) };
			size_t glyphs = 0;

			stopwatch(counter);
				agge::fill(surface, area, solid_color_brush(255, 255, 255));
			timings.clearing += stopwatch(counter);

//			_ddx += 0.01f;

			_rasterizer.reset();

			_layout.limit_width(static_cast<real_t>(surface.width()));

			double layouting = stopwatch(counter);

			for (layout::const_iterator i = _layout.begin(); i != _layout.end(); ++i)
			{
				real_t x = i->reference.x + _ddx;

				if (i->reference.y > surface.height())
					break;

				glyphs += distance(i->begin, i->end);
				for (layout::positioned_glyphs_container::const_iterator j = i->begin; j != i->end; ++j)
				{
					x += j->dx;
					_font_engine.render_glyph(_rasterizer, *i->glyph_run_font, j->index, x, i->reference.y);
				}
			}

			double append = stopwatch(counter);

			_rasterizer.sort(true);

			double sort = stopwatch(counter);

			_renderer(surface, 0, _rasterizer, solid_color_brush(0, 0, 0, 255), winding<>());

			double render = stopwatch(counter);

			timings.stroking += (layouting + append + sort + render) / glyphs;
			timings.rasterization += append + sort;
			timings.rendition += render;
		}

	private:
		typedef platform_blender_solid_color solid_color_brush;

	private:
		my_rasterizer _rasterizer;
		renderer_parallel _renderer;
		font_loader _font_loader;
		font_engine<my_rasterizer> _font_engine;
		shared_ptr<font> _font;
		layout _layout;
		float _ddx;
	};
}

application *agge_create_application()
{
	return new demo::TextDrawer;
}
