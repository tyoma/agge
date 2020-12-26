#include <agge.text/layout.h>

#include <agge/tools.h>
#include <agge.text/richtext.h>

using namespace std;

namespace agge
{
	namespace
	{
		real_t height(const font::metrics &m)
		{	return m.ascent + m.descent + m.leading; }

		template <typename IteratorT>
		bool eat_lf(IteratorT &i)
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

	layout::layout(font::ptr base_font)
		: _base_font(base_font), _limit_width(1e30f)
	{	}

	void layout::process(const richtext_t &text)
	{
		_text = text;
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

		if (_glyph_runs.empty())
			return box;

		font::metrics m = _base_font->get_metrics();

		for (const_iterator i = begin(); i != end(); ++i)
			box.w = agge_max(box.w, i->width);
		box.h = (end() - begin()) * height(m) - m.leading;
		return box;
	}

	void layout::analyze()
	{
		const font::metrics m = _base_font->get_metrics();
		real_t y = 0;

		_glyph_runs.clear();
		_glyphs.resize(static_cast<count_t>(_text.size()));

		positioned_glyphs_container::iterator pgi = _glyphs.begin();

		for (auto range = _text.ranges_begin(); range != _text.ranges_end(); ++range)
		{
			for (auto i = range->begin; i != range->end; )
			{
				real_t width = 0.0f;
				const glyph *previous = 0;
				sensors::eow eow;
				positioned_glyphs_container::iterator start_pgi = pgi, eow_pgi = pgi;

				for (auto eow_i = range->end; i != range->end && !eat_lf(i); ++i, ++pgi)
				{
					const uint16_t index = _base_font->map_single(*i);
					const glyph *g = _base_font->get_glyph(index);

					if (eow(*i))
					{
						eow_i = i;
						eow_pgi = pgi;
					}

					width += g->metrics.advance_x;
					if (width > _limit_width)
					{
						if (eow_i != range->end) // not an emergency break
						{
							i = eow_i;
							++i;
							pgi = eow_pgi;
						}
						break;
					}
					pgi->d.dx = previous ? previous->metrics.advance_x : 0.0f;
					pgi->d.dy = 0.0f;
					pgi->index = index;
					previous = g;
				}

				glyph_run gr;

				gr.begin = start_pgi;
				gr.end = pgi;
				gr.reference.x = 0.0f;
				gr.reference.y = y + m.ascent;
				gr.width = width;
				gr.glyph_run_font = _base_font;
				_glyph_runs.push_back(gr);
				y += height(m);
			}
		}
	}
}
