#include <agge/renderer.h>
#include <agge.text/layout.h>
#include <samples/common/lipsum.h>
#include <samples/common/platform/win32/dc.h>
#include <samples/common/platform/win32/font_accessor.h>
#include <samples/common/shell.h>
#include <samples/common/timing.h>
#include <windows.h>

using namespace agge;
using namespace std;

namespace demo
{
	class TextDrawerGDI : public application
	{
	public:
		TextDrawerGDI()
			: _font_accessor(new font_accessor(14, L"tahoma", false, false, font_engine_base::gf_none)),
				_font(new font(_font_accessor)), _layout(c_text_long.c_str(), _font), _ddx(0.0f)
		{	}

	private:
		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			long long counter;
			const rect_i area = { 0, 0, surface.width(), surface.height() };
			size_t glyphs = 0;
			dc ctx(&surface);
			dc::handle h = ctx.select(_font_accessor->native());

			stopwatch(counter);
				agge::fill(surface, area, solid_color_brush(255, 255, 255));
			timings.clearing += stopwatch(counter);

			_layout.limit_width(surface.width());

			double layouting = stopwatch(counter);
			
			::SetTextAlign(ctx, TA_BASELINE | TA_LEFT);

			for (layout::const_iterator i = _layout.begin(); i != _layout.end(); ++i)
			{
				real_t x = i->reference.x + _ddx;

				_glyph_indices.clear();

				if (i->reference.y > surface.height())
					break;

				glyphs += distance(i->begin, i->end);
				for (layout::positioned_glyphs_container::const_iterator j = i->begin; j != i->end; ++j)
					_glyph_indices.push_back(j->index);
				::ExtTextOut(ctx, static_cast<int>(x), static_cast<int>(i->reference.y), ETO_GLYPH_INDEX /*| ETO_PDY*/, 0,
					reinterpret_cast<LPCTSTR>(&_glyph_indices[0]), static_cast<UINT>(_glyph_indices.size()), 0);
			}

			double rasterizer = stopwatch(counter);

			timings.stroking += (layouting + rasterizer) / glyphs;
			timings.rasterization += rasterizer;
		}

	private:
		typedef platform_blender_solid_color solid_color_brush;

	private:
		shared_ptr<font_accessor> _font_accessor;
		shared_ptr<font> _font;
		layout _layout;
		float _ddx;
		vector<uint16_t> _glyph_indices;
	};
}

application *agge_create_application()
{
	return new demo::TextDrawerGDI;
}
