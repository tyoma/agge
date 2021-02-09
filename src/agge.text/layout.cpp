#include <agge.text/layout.h>

#include <agge/config.h>
#include <agge/math.h>
#include <agge/tools.h>
#include <agge.text/font.h>
#include <agge.text/font_factory.h>

using namespace std;

namespace agge
{
	namespace
	{
		bool is_space(char c)
		{	return c == ' ';	}

		template <typename IteratorT>
		void eat_spaces(IteratorT &i, IteratorT end)
		{
			while (i != end && is_space(*i))
				++i;
		}

		template <typename IteratorT>
		bool eat_lf(IteratorT &i)
		{	return *i == '\n' ? ++i, true : false;	}

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
			const font &font_ = *accumulator.font_;
			size_t eow_position = 0, sow_position = 0;
			real_t eow_width = 0.0f, sow_width = 0.0f;

			next.offset = zero();
			next.width = 0.0f;
			for (real_t advance; i != text_end; ++i, accumulator.width += advance)
			{
				if (eat_lf(i))
				{
					// Next line - line-feed
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
					next.set_end();
					if (eow_position) // else: next line - emergency mid-word break
					{
						// Next line - normal word-boundary break
						sow_width = accumulator.width - sow_width;
						accumulator.end_index = eow_position;
						accumulator.width = eow_width;
						if (sow_position > eow_position)
						{
							// New word was actually found after the last matched end-of-word.
							next.begin_index = sow_position;
							next.width = sow_width;
						}
						else
						{
							// No new word found before - let's scan for it ourselves.
							eat_spaces(i, text_end);
						}
					}
					return true;
				}

				const positioned_glyph pg = {	create_vector(advance, 0.0f), index	};

				glyphs.push_back(pg);
				accumulator.extend_end();
			}
			return false;
		}

		pair<real_t /*ascent*/, real_t /*descent + leading*/> setup_line_metrics(text_line &line)
		{
			real_t descent = 0.0f;
			pair<real_t, real_t> m(0.0f, 0.0f);

			for (text_line::const_iterator i = line.begin(), end = line.end(); i != end; ++i)
			{
				const font_metrics grm = i->font_->get_metrics();

				m.first = agge_max(m.first, grm.ascent);
				m.second = agge_max(m.second, grm.descent + grm.leading);
				descent = agge_max(descent, grm.descent);
			}
			line.descent = descent;
			return m;
		}

		pair<real_t /*ascent*/, real_t /*descent + leading*/> setup_line_metrics(font_metrics m, text_line &line)
		{	return line.empty() ? make_pair(m.ascent, m.descent + m.leading) : setup_line_metrics(line);	}
	}

	layout::layout(font_factory &factory)
		: _factory(factory), _limit_width(1e30f)
	{	}

	void layout::process(const richtext_t &text)
	{
		_text_lines.clear();
		_glyph_runs.clear();
		_glyphs.clear();
		_box = zero();

		text_line *current_line = &*_text_lines.insert(_text_lines.end(), text_line(_glyph_runs));
		glyph_run *current_grun = &*_glyph_runs.insert(_glyph_runs.end(), glyph_run(_glyphs));

		for (richtext_t::const_iterator range = text.ranges_begin(); range != text.ranges_end(); ++range)
		{
			current_grun->font_ = _factory.create_font(range->get_annotation().basic);
			current_grun->offset = create_vector(current_line->width, 0.0f);

			glyph_run next_line_grun(*current_grun);

			for (detector_iterator i = range->begin(), end = range->end(), previous = i;
				populate_glyph_run(_glyphs, *current_grun, next_line_grun, _limit_width - current_line->width, i, end);
				previous = i)
			{
				if (i == previous & current_line->empty())
				{
					// Emergency: width limit is too small to layout even a single character - bailing out!
					_text_lines.clear();
					return;
				}
				else if (!commit_glyph_run(*current_line, current_grun, next_line_grun))
				{
					*current_grun = next_line_grun;
				}

				const pair<real_t, real_t> m = setup_line_metrics(current_grun->font_->get_metrics(), *current_line);

				current_line->offset += create_vector(0.0f, m.first);
				if (!current_line->empty())
				{
					_box.w = agge_max(_box.w, current_line->width);
					current_line = &*_text_lines.insert(_text_lines.end(), text_line(*current_line));
					current_line->begin_index = current_line->end_index;
					current_line->width = 0.0f;
				}
				current_line->offset += create_vector(0.0f, m.second);
			}

			if (!current_grun->empty())
			{
				// Remainder of the text range was not empty, we pushed it and prepare current_grun for reuse.
				commit_glyph_run(*current_line, current_grun, glyph_run(*current_grun));
				current_grun->set_end();
				current_grun->width = 0.0f;
			}
		}
		if (current_line->empty())
			_text_lines.pop_back();
		else
			current_line->offset += create_vector(0.0f, setup_line_metrics(*current_line).first);
		if (current_grun->empty())
			_glyph_runs.pop_back();
		if (!_text_lines.empty())
		{
			const text_line &last = _text_lines.back();

			_box.w = agge_max(_box.w, last.width);
			_box.h = last.offset.dy + last.descent;
		}
	}

	bool layout::commit_glyph_run(text_line &current_line, glyph_run *&current_grun, const glyph_run &next_line_grun)
	{
		if (current_grun->empty())
			return false;	// Nothing to commit.
		current_line.width += current_grun->width;
		current_line.extend_end();
		current_grun = &*_glyph_runs.insert(_glyph_runs.end(), next_line_grun);
		return true;	// Last glyph run committed, next line's glyph run is pushed to container to be committed later.
	}
}
