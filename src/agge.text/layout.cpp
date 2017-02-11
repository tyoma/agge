#include <agge.text/layout.h>

#include <agge.text/glyph.h>

using namespace std;

namespace agge
{
	layout::layout(const wchar_t *text, font::ptr font_)
		: _text(text), _font(font_)
	{
	}

	box_r layout::get_box()
	{
		box_r box = { };

		for (wstring::const_iterator i = _text.begin(); i != _text.end(); ++i)
		{
			const glyph *g = _font->get_glyph(*i);
			box.w += g->advance_x;
		}
		return box;
	}
}
