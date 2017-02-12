#include <agge.text/layout.h>

#include <agge/tools.h>
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
		real_t w = 0.0f;

		for (wstring::const_iterator i = _text.begin(); i != _text.end(); ++i)
		{
			const wchar_t c = *i;

			if (L'\n' == c)
			{
				if (w > box.w)
					box.w = w;
				w = 0.0f;
			}
			else
			{
				const glyph *g = _font->get_glyph(c);
				w += g->advance_x;
			}
		}
		if (w > box.w)
			box.w = w;
		return box;
	}
}
