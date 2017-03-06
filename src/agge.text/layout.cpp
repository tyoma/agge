#include <agge.text/layout.h>

#include <agge/tools.h>

using namespace std;

namespace agge
{
	namespace
	{
		real_t height(const font::metrics &m)
		{	return m.ascent + m.descent + m.leading; }

		bool eat_lf(wstring::const_iterator &i)
		{
			if (*i == L'\n')
			{
				++i;
				return true;
			}
			return false;
		}
	}

	namespace sensors
	{
		class eow : noncopyable
		{
		public:
			eow ()
				: _previous(0)
			{	}

			bool operator ()(wchar_t c)
			{
				bool result = c == L' ' && _previous != L' ';
				_previous = c;
				return result;
			}

		private:
			wchar_t _previous;
		};
	}

	layout::layout(const wchar_t *text, font::ptr font_)
		: _text(text), _font(font_), _limit_width(1e30f)
	{
		analyze();
	}

	void layout::limit_width(real_t width)
	{
		_limit_width = width;
		analyze();
	}

	box_r layout::get_box()
	{
		box_r box = {};

		for (const_iterator i = begin(); i != end(); ++i)
			box.w = agge_max(box.w, i->width);
		return box;
	}

	void layout::analyze()
	{
		if (_text.empty())
			return;

		const font::metrics m = _font->get_metrics();
		const wstring &text = _text;
		real_t y = 0;

		_glyph_runs.clear();
		_glyphs.resize(static_cast<count_t>(_text.size()));

		positioned_glyphs_container::iterator pgi = _glyphs.begin();

		for (wstring::const_iterator i = text.begin(); i != text.end(); )
		{
			real_t width = 0.0f;
			const glyph *previous = 0;
			sensors::eow eow;
			positioned_glyphs_container::iterator start_pgi = pgi, eow_pgi = pgi;

			for (wstring::const_iterator eow_i = _text.end(); i != _text.end() && !eat_lf(i); ++i, ++pgi)
			{
				const uint16_t index = _font->map_single(*i);
				const glyph *g = _font->get_glyph(index);

				if (eow(*i))
				{
					eow_i = i;
					eow_pgi = pgi;
				}

				width += g->metrics.advance_x;
				if (width > _limit_width)
				{
					if (eow_i != _text.end()) // not an emergency break
					{
						i = eow_i + 1;
						pgi = eow_pgi;
					}
					break;
				}
				pgi->dx = previous ? previous->metrics.advance_x : 0.0f;
				pgi->dy = 0.0f;
				pgi->index = index;
				previous = g;
			}

			glyph_run gr;

			gr.begin = start_pgi;
			gr.end = pgi;
			gr.reference.x = 0.0f;
			gr.reference.y = y + m.ascent;
			gr.width = width;
			gr.glyph_run_font = _font;
			_glyph_runs.push_back(gr);
			y += height(m);
		}
	}
}
