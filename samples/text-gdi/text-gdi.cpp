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
			: _font_accessor(new font_accessor(14, L"tahoma", false, false, font::key::gf_none)),
				_font(new font(font::key(L"tahoma", 14), _font_accessor)), _layout(_font), _text(c_text_long.c_str()),
				_ddx(0.0f)
		{	_layout.process(c_text_long.c_str());	}

	private:
		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			long long counter;
			const rect_i area = { 0, 0, static_cast<int>(surface.width()), static_cast<int>(surface.height()) };
			size_t glyphs = 0;
			dc ctx(&surface);
			dc::handle h = ctx.select(_font_accessor->native());

			stopwatch(counter);
				agge::fill(surface, area, platform_blender_solid_color(color::make(255, 255, 255)));
			timings.clearing += stopwatch(counter);
				_layout.process(_text);
			double layouting = stopwatch(counter);
				::SetTextAlign(ctx, TA_BASELINE | TA_LEFT);
				for (layout::const_iterator i = _layout.begin(); i != _layout.end(); ++i)
				{
					real_t x = i->offset.dx + _ddx;

					_glyph_indices.clear();

					if (i->offset.dy > surface.height())
						break;

					glyphs += distance(i->begin(), i->end());
					for (positioned_glyphs_container_t::const_iterator j = i->begin(); j != i->end(); ++j)
						_glyph_indices.push_back(j->index);
					::ExtTextOut(ctx, static_cast<int>(x), static_cast<int>(i->offset.dy), ETO_GLYPH_INDEX /*| ETO_PDY*/, 0,
						reinterpret_cast<LPCTSTR>(&_glyph_indices[0]), static_cast<UINT>(_glyph_indices.size()), 0);
				}

			double rasterization = stopwatch(counter);

			timings.stroking += layouting;
			timings.rasterization += rasterization;
		}

		virtual void resize(int width, int /*height*/)
		{	_layout.set_width_limit(static_cast<real_t>(width));	}

	private:
		shared_ptr<font_accessor> _font_accessor;
		shared_ptr<font> _font;
		richtext_t _text;
		layout _layout;
		float _ddx;
		vector<agge::uint16_t> _glyph_indices;
	};
}

application *agge_create_application(services &/*s*/)
{
	return new demo::TextDrawerGDI;
}
