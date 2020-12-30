#include <agge.text/layout.h>

#include <agge/math.h>
#include <agge/tools.h>
#include <agge.text/font.h>

using namespace std;

namespace agge
{
	namespace
	{
		real_t height(const font_metrics &m)
		{	return m.ascent + m.descent + m.leading; }

		bool is_space(wchar_t c)
		{	return c == L' ';	}

		template <typename IteratorT>
		bool eat_lf(IteratorT &i)
		{	return *i == L'\n' ? ++i, true : false;	}

		class detector_iterator
		{
		public:
			detector_iterator(richtext_t::string_type::const_iterator from)
				: _underlying(from), _previous(0)
			{	}

			void operator ++()
			{	_previous = *_underlying++;	}

			richtext_t::string_type::value_type operator *() const
			{	return *_underlying;	}

			bool operator ==(const detector_iterator &rhs) const
			{	return _underlying == rhs._underlying;	}

			bool operator !=(const detector_iterator &rhs) const
			{	return _underlying != rhs._underlying;	}

			bool at_end_of_word() const
			{	return is_space(*_underlying) & !is_space(_previous);	}

			bool at_start_of_word() const
			{	return !is_space(*_underlying) & is_space(_previous);	}

		private:
			richtext_t::string_type::const_iterator _underlying;
			richtext_t::string_type::value_type _previous;
		};

		template <typename ContainerT, typename CharIteratorT>
		bool /*end-of-line*/ populate_glyph_run(ContainerT &glyphs, glyph_run &accumulator, glyph_run &next,
			const real_t limit, CharIteratorT &i, CharIteratorT text_end)
		{
			const font &font_ = *accumulator.glyph_run_font;
			size_t eow_position = 0, sow_position = 0;
			real_t eow_width = 0.0f, sow_width = 0.0f;

			next.width = 0.0f;
			for (real_t advance; i != text_end; ++i, accumulator.width += advance)
			{
				if (eat_lf(i))
				{
					// Next line: line-feed
					next.set_end();
					return true;
				}

				const glyph_index_t index = font_.map_single(*i);
				const glyph *g = font_.get_glyph(index);

				advance = g->metrics.advance_x;

				if (i.at_end_of_word())
					eow_position = accumulator.end_index, eow_width = accumulator.width;
				if (i.at_start_of_word())
					sow_position = accumulator.end_index, sow_width = accumulator.width;

				if (accumulator.width + advance > limit)
				{
					if (eow_position)
					{
						// Next line: normal word-boundary break
						next = accumulator;
						accumulator.end_index = eow_position;
						sow_width = accumulator.width - sow_width;
						accumulator.width = eow_width;
						if (sow_position > eow_position)
						{
							// New word was actually found after the last matched end-of-word.
							next.begin_index = sow_position;
							next.width = sow_width;
						}
						else
						{
							// No new word found before - let's scan ourselves.
							while (i != text_end && is_space(*i))
								++i;
							next.set_end();
							next.width = 0.0f;
						}
					}
					else
					{
						// Next line: emergency mid-word break
						next.set_end();
					}
					return true;
				}

				const positioned_glyph pg = {	create_vector(advance, 0.0f), index	};

				glyphs.push_back(pg);
				accumulator.extend_end();
			}
			return false;
		}
	}

	layout::layout(font::ptr base_font)
		: _base_font(base_font), _limit_width(1e30f)
	{	}

	void layout::process(const richtext_t &text)
	{
		_text_lines.clear();
		_glyph_runs.clear();
		_glyphs.clear();

		text_line accumulator_tl(_glyph_runs);

		for (richtext_t::const_iterator range = text.ranges_begin(); range != text.ranges_end(); ++range)
		{
			glyph_run accumulator(_glyphs);
			const font_metrics m = _base_font->get_metrics();

			accumulator.set_end();
			accumulator.glyph_run_font = _base_font;
			accumulator.offset = zero();
			accumulator.width = 0.0f;

			accumulator_tl.offset = create_vector(0.0f, m.ascent);

			glyph_run next(accumulator);

			for (detector_iterator i = range->begin(), end = range->end(), previous = i;
				populate_glyph_run(_glyphs, accumulator, next, _limit_width, i, end); previous = i)
			{
				if (!accumulator.empty())
				{
					_glyph_runs.push_back(accumulator);
					accumulator_tl.extend_end();
					accumulator_tl.width = accumulator.width;
					_text_lines.push_back(accumulator_tl);
					accumulator_tl.set_end();
				}
				else if (i == previous)
				{
					_text_lines.clear();
					return;
				}
				accumulator = next;
				accumulator_tl.offset += create_vector(0.0f, height(m));
			}
			if (!accumulator.empty())
			{
				_glyph_runs.push_back(accumulator);
				accumulator_tl.extend_end();
				accumulator_tl.width = accumulator.width;
				_text_lines.push_back(accumulator_tl);
				accumulator_tl.set_end();
			}
		}
	}

	void layout::set_width_limit(real_t width)
	{
		_limit_width = width;
		_text_lines.clear();
	}

	box_r layout::get_box() const
	{
		box_r box = {};

		if (_glyph_runs.empty())
			return box;

		font_metrics m = _base_font->get_metrics();

		for (const_iterator i = begin(); i != end(); ++i)
			box.w = agge_max(box.w, i->width);
		box.h = (end() - begin()) * height(m) - m.leading;
		return box;
	}
}
