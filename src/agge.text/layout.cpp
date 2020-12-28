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

		bool is_space(wchar_t c)
		{	return c == L' ';	}

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
		class sensor : noncopyable
		{
		public:
			sensor()
				: _previous(0)
			{	}

		protected:
			wchar_t _previous;
		};

		struct eow : sensor
		{
			bool operator ()(wchar_t c)
			{
				bool result = is_space(c) && !is_space(_previous);

				_previous = c;
				return result;
			}
		};

		struct sow : sensor
		{
			bool operator ()(wchar_t c)
			{
				bool result = !is_space(c) && is_space(_previous);

				_previous = c;
				return result;
			}
		};
	}

	layout::layout(font::ptr base_font)
		: _base_font(base_font), _limit_width(1e30f)
	{	}

	void layout::process(const richtext_t &text)
	{
		const font::metrics m = _base_font->get_metrics();
		sensors::eow eow;
		sensors::sow sow;

		_glyph_runs.clear();
		_glyphs.clear();
		for (richtext_t::const_iterator range = text.ranges_begin(); range != text.ranges_end(); ++range)
		{
			glyph_run accumulator(_glyphs);

			accumulator.set_end();
			accumulator.glyph_run_font = _base_font;
			accumulator.reference.x = 0.0f, accumulator.reference.y = m.ascent;
			accumulator.width = 0.0f;

			size_t eow_position = 0, sow_position = 0;
			real_t eow_width = 0.0f, sow_width = 0.0f;

			for (richtext_t::string_type::const_iterator i = range->begin(), end = range->end(); i != end; )
			{
				if (eat_lf(i))
				{
					// Next line: line-feed
					new_line(accumulator, height(m)), eow_position = 0;
					continue;
				}

				const glyph_index_t index = _base_font->map_single(*i);
				const glyph *g = _base_font->get_glyph(index);
				const real_t advance = g->metrics.advance_x;
				const positioned_glyph pg = {	create_vector(advance, 0.0f), index	};

				if (eow(*i))
					eow_position = accumulator.end_index, eow_width = accumulator.width;
				if (sow(*i))
					sow_position = accumulator.end_index, sow_width = 0.0f;
				if (!eow_position && accumulator.width + advance > _limit_width)
				{
					// Next line: emergency mid-word break
					new_line(accumulator, height(m)), eow_position = 0;
					continue;
				}

				_glyphs.push_back(pg);
				accumulator.extend_end();
				accumulator.width += pg.d.dx;
				sow_width += pg.d.dx;

				if (eow_position && accumulator.width > _limit_width)
				{
					// Next line: normal word-boundary break
					glyph_run eow_accumulator(accumulator);

					eow_accumulator.end_index = eow_position;
					eow_accumulator.width = eow_width;
					_glyph_runs.push_back(eow_accumulator);
					accumulator.reference.x = 0.0f, accumulator.reference.y += height(m);
//					eow_position = 0; // TODO: check resetting of last eow index
					if (sow_position > eow_position)
					{
						// New word was actually found after the last matched end-of-word.
						accumulator.begin_index = sow_position;
						accumulator.width = sow_width;
					}
					else
					{
						// No new word found before - let's scan ourselves.
						accumulator.set_end();
						accumulator.width = 0.0f;
						while (i != end && isspace(*i))
							i++;
						continue;
					}
				}
				i++;
			}
			if (!accumulator.empty())
				_glyph_runs.push_back(accumulator);
		}
	}

	void layout::set_width_limit(real_t width)
	{
		_limit_width = width;
		_glyph_runs.clear();
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

	void layout::new_line(glyph_run &range_, real_t dy)
	{
		if (!range_.empty())
			_glyph_runs.push_back(range_);
		range_.set_end();
		range_.reference.x = 0.0f, range_.reference.y += dy;
		range_.width = 0.0f;
	}
}
