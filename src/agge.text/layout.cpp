#include <agge.text/layout.h>

#include <agge/tools.h>
#include <agge.text/font.h>

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
			glyph_run accumulator(_glyphs);

			accumulator.set_end();
			accumulator.glyph_run_font = _base_font;
			accumulator.reference.x = 0.0f, accumulator.reference.y = m.ascent;
			accumulator.width = 0.0f;

			glyph_run eow_accumulator(accumulator);

			for (richtext_t::string_type::const_iterator i = range->begin(), end = range->end(); i != end; )
			{
				if (eat_lf(i))
				{
					// Next line: line-feed
					new_line(accumulator, height(m)), eow_accumulator.set_end();
					continue;
				}

				const glyph_index_t index = _base_font->map_single(*i);
				const glyph *g = _base_font->get_glyph(index);

				if (eow_accumulator.empty() && accumulator.width + g->metrics.advance_x > _limit_width)
				{
					// Next line: emergency mid-word break
					new_line(accumulator, height(m)), eow_accumulator.set_end();
					continue;
				}

				const positioned_glyph pg = {	create_vector(g->metrics.advance_x, 0.0f), index	};

				if (eow(*i))
					eow_accumulator = accumulator;
				_glyphs.push_back(pg);
				accumulator.extend_end();
				accumulator.width += pg.d.dx;

				if (!eow_accumulator.empty() && accumulator.width > _limit_width)
				{
					// Next line: normal word-boundary break
					_glyph_runs.push_back(eow_accumulator);
					accumulator.begin_index = eow_accumulator.end_index + 1; // TODO: we eat only one space after the word-break now - have to eat them all...
					accumulator.reference.x = 0.0f, accumulator.reference.y += height(m);
					accumulator.width -= eow_accumulator.width
						+ _base_font->get_glyph(eow_accumulator.end()->index)->metrics.advance_x;
					eow_accumulator.set_end();
				}
				i++;
			}
			if (!accumulator.empty())
				_glyph_runs.push_back(accumulator);
		}
	}

	void layout::new_line(glyph_run &range_, real_t dy)
	{
		if (!range_.empty())
			_glyph_runs.push_back(range_);
		range_.set_end();
		range_.reference.x = 0.0f, range_.reference.y += dy;
		range_.width = 0.0f;
	}
}
