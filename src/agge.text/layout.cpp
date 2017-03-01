#include <agge.text/layout.h>

#include <agge/tools.h>
#include <agge.text/glyph.h>

using namespace std;

namespace agge
{
	namespace
	{
		real_t height(const font::metrics &m)
		{	return m.ascent + m.descent + m.leading; }
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
		: _text(text), _font(font_), _limit_width(0.0f)
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
		const count_t limit = static_cast<count_t>(_text.size());
		const glyph *previous = 0;
		sensors::eow eow;

		_glyphs.resize(limit);
		_glyph_runs.resize(limit);

		glyph_runs_container::iterator gr = _glyph_runs.begin();
		positioned_glyphs_container::iterator pg = _glyphs.begin(), eow_pg = pg;
		gr->begin = gr->end = pg;
		gr->width = 0.0f;
		gr->reference.x = 0.0f;
		gr->reference.y = m.ascent;
		gr->glyph_run_font = _font;

		for (wstring::const_iterator i = _text.begin(), eow_i = _text.end(); i != _text.end(); ++i)
		{
			const wchar_t c = *i;
			const glyph *g;

			if (eow(c))
			{
				eow_i = i;
				eow_pg = pg;
			}

			switch (c)
			{
			default:
				g = _font->get_glyph(c);

				gr->width += g->advance_x;
				if (_limit_width == 0.0f || gr->width <= _limit_width)
				{
					pg->dx = previous ? previous->advance_x : 0.0f;
					pg->dy = 0.0f;
					pg->index = g->index;
					++pg;
					previous = g;
					break;
				}
				else
				{
					gr->width -= g->advance_x;
					pg = eow_pg;
					i = eow_i;
				}

			case '\n':
				gr->end = pg;
				++gr;
				gr->end = gr->begin = pg;
				gr->width = 0.0f;
				gr->reference.x = 0.0f;
				gr->reference.y = (gr - 1)->reference.y + height(m);
				gr->glyph_run_font = _font;
				previous = 0;
				break;
			}
		}
		gr->end = pg;

		_glyph_runs.resize(static_cast<count_t>(gr - _glyph_runs.begin() + (gr->end != gr->begin)));
	}
}
