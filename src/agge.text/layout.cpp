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
		sensors::eow eow;

		_glyph_runs.clear();
		_glyphs.clear();
		for (richtext_t::const_iterator range = _text.ranges_begin(); range != _text.ranges_end(); ++range)
		{
			glyph_run pgi(_glyphs);

			pgi.set_end();
			pgi.glyph_run_font = _base_font;
			pgi.reference.x = 0.0f, pgi.reference.y = m.ascent;
			pgi.width = 0.0f;

			glyph_run eow_pgi(pgi);

			for (richtext_t::string_type::const_iterator i = range->begin(), end = range->end(); i != end; )
			{
				if (eat_lf(i))
				{
					// Next line: line-feed
					_glyph_runs.push_back(pgi);
					pgi.set_end();
					pgi.reference.x = 0.0f, pgi.reference.y += height(m);
					pgi.width = 0.0f;
					eow_pgi = pgi;
					continue;
				}

				const uint16_t index = _base_font->map_single(*i);
				const glyph *g = _base_font->get_glyph(index);

				if (eow_pgi.empty() && pgi.width + g->metrics.advance_x > _limit_width)
				{
					// Next line: emergency mid-word break
					_glyph_runs.push_back(pgi);
					pgi.set_end();
					pgi.reference.x = 0.0f, pgi.reference.y += height(m);
					pgi.width = 0.0f;
					eow_pgi = pgi;
					continue;
				}

				const positioned_glyph pg = {
					create_vector(g->metrics.advance_x, 0.0f),
					index
				};

				if (eow(*i))
					eow_pgi = pgi;
				_glyphs.push_back(pg);
				pgi.extend_end();
				pgi.width += pg.d.dx;

				if (!eow_pgi.empty() && pgi.width > _limit_width)
				{
					// Next line: normal word-boundary break
					_glyph_runs.push_back(eow_pgi);
					pgi.begin_index = eow_pgi.end_index + 1; // TODO: we eat only one space after the word-break now - have to eat them all...
					pgi.reference.x = 0.0f, pgi.reference.y += height(m);
					pgi.width -= eow_pgi.width + _base_font->get_glyph(eow_pgi.end()->index)->metrics.advance_x;
					eow_pgi.set_end();
				}
				i++;
			}
			if (!pgi.empty())
				_glyph_runs.push_back(pgi);
		}
	}
}
