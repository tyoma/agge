#include <agge/clipper.h>
#include <agge/rasterizer.h>
#include <agge/renderer.h>
#include <agge.text/layout.h>
#include <map>
#include <samples/common/font_loader.h>
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
	typedef rasterizer< clipper<int> > my_rasterizer;

	class TextDrawerGDI : public application
	{
	public:
		TextDrawerGDI()
			: _text_engine(_font_loader), _text(font_style_annotation()), _layout(_text_engine)
		{
			font_style_annotation a = {	font_descriptor::create("Tahoma", 14),	};

			_text.set_base_annotation(a);
			_text << c_text_long.c_str();
		}

	private:
		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			long long counter;
			const rect_i area = { 0, 0, static_cast<int>(surface.width()), static_cast<int>(surface.height()) };
			dc ctx(&surface);

			stopwatch(counter);
				agge::fill(surface, area, platform_blender_solid_color(color::make(255, 255, 255)));
			timings.clearing += stopwatch(counter);
				_layout.process(_text);
			double layouting = stopwatch(counter);
				::SetTextAlign(ctx, TA_BASELINE | TA_LEFT);
				for (layout::const_iterator i = _layout.begin(); i != _layout.end(); ++i)
				{
					vector_r d = i->offset;

					_glyph_indices.clear();

					if (i->offset.dy > surface.height())
						break;

					for (glyph_runs_container_t::const_iterator j = i->begin(); j != i->end(); ++j)
					{
						shared_ptr<void> hfont = create_font(j->font_->get_key());
						dc::handle h = ctx.select(static_cast<HGDIOBJ>(hfont.get()));
						vector_r dg = d;

						dg += j->offset;
						for (positioned_glyphs_container_t::const_iterator k = j->begin(); k != j->end(); ++k)
							_glyph_indices.push_back(static_cast<unsigned short>(k->index));
						::ExtTextOut(ctx, static_cast<int>(dg.dx), static_cast<int>(dg.dy), ETO_GLYPH_INDEX /*| ETO_PDY*/, 0,
							reinterpret_cast<LPCTSTR>(&_glyph_indices[0]), static_cast<UINT>(_glyph_indices.size()), 0);
					}
				}

			double rasterization = stopwatch(counter);

			timings.stroking += layouting;
			timings.rasterization += rasterization;
		}

		virtual void resize(int width, int /*height*/)
		{	_layout.set_width_limit(static_cast<real_t>(width));	}

		shared_ptr<void> create_font(const font_descriptor &fd)
		{
			return shared_ptr<void>(::CreateFontA(-fd.height, 0, 0, 0, fd.weight >= bold ? FW_BOLD : FW_NORMAL,
				!!fd.italic, FALSE, FALSE, 0, 0, 0, ANTIALIASED_QUALITY, 0, fd.family.c_str()), &::DeleteObject);
		}

	private:
		native_font_loader _font_loader;
		text_engine<my_rasterizer> _text_engine;
		richtext_t _text;
		layout _layout;
		vector<agge::uint16_t> _glyph_indices;
	};
}

application *agge_create_application(services &/*s*/)
{
	return new demo::TextDrawerGDI;
}
