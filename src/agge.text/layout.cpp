#include <agge.text/layout.h>

#include <agge/config.h>
#include <agge/math.h>
#include <agge/tools.h>
#include <agge.text/font.h>
#include <agge.text/font_factory.h>
#include <agge.text/utf8.h>

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


		class wrap_processor
		{
		public:
			wrap_processor(glyph_run &carry)
				: _carry(carry)
			{	}

			void on_new_run()
			{
				_carry.offset = zero(), _carry.width = real_t();
				_eow_index = _sow_index = size_t();
				_eow_width = _sow_width = real_t();
				_previous_space = false;
			}

			void on_linefeed()
			{	_carry.set_end();	}

			void analyze_character(char c, const glyph_run &accumulator)
			{
				const auto space = is_space(c);

				if (_previous_space == space)
					return;
				else if (space)
					_eow_index = accumulator.end_index, _eow_width = accumulator.width;
				else
					_sow_index = accumulator.end_index, _sow_width = accumulator.width;
				_previous_space = space;
			}

			template <typename CharIteratorT>
			void on_limit_reached(CharIteratorT &i, CharIteratorT text_end, glyph_run &accumulator)
			{
				_carry.set_end();
				if (!_eow_index)
					return;	// Next line - emergency mid-word break

				// Next line - normal word-boundary break
				_sow_width = accumulator.width - _sow_width;
				accumulator.end_index = _eow_index;
				accumulator.width = _eow_width;
				if (_sow_index > _eow_index)
				{
					// New word was actually found after the last matched end-of-word.
					_carry.begin_index = _sow_index;
					_carry.width = _sow_width;
				}
				else
				{
					// No new word found before - let's scan for it ourselves.
					eat_spaces(i, text_end);
				}
			}

		private:
			const wrap_processor &operator =(const wrap_processor &rhs);

		private:
			glyph_run &_carry;
			size_t _eow_index, _sow_index;
			real_t _eow_width, _sow_width;
			bool _previous_space;
		};


		template <typename ContainerT, typename ProcessorT, typename CharIteratorT>
		bool /*end-of-line*/ populate_glyph_run(ContainerT &glyphs, glyph_run &accumulator, ProcessorT &limit_processor,
			const real_t limit, CharIteratorT &i, CharIteratorT text_end)
		{
			limit_processor.on_new_run();
			while (i != text_end)
			{
				if (eat_lf(i)) // Next line - line-feed
					return limit_processor.on_linefeed(), true;

				CharIteratorT i_next = i;
				const glyph *const g = accumulator.font_->get_glyph_for_codepoint(utf8::next(i_next, text_end));
				const real_t advance = g->metrics.advance_x;

				limit_processor.analyze_character(*i, accumulator);
				if (accumulator.width + advance > limit)
					return limit_processor.on_limit_reached(i, text_end, accumulator), true;

				const positioned_glyph pg = {	create_vector(advance, 0.0f), g->index	};

				glyphs.push_back(pg);
				accumulator.end_index++;
				accumulator.width += advance;
				i = i_next;
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

				if (m.first < grm.ascent)
					m.first = grm.ascent;
				if (m.second < grm.descent + grm.leading)
					m.second = grm.descent + grm.leading;
				if (descent < grm.descent)
					descent = grm.descent;
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
		glyph_run *current_grun = &*_glyph_runs.insert(_glyph_runs.end(), glyph_run(_glyphs)), carry(*current_grun);
		wrap_processor limit_processor(carry);

		for (richtext_t::const_iterator range = text.ranges_begin(); range != text.ranges_end(); ++range)
		{
			current_grun->font_ = _factory.create_font(range->get_annotation().basic);
			current_grun->offset = create_vector(current_line->width, 0.0f);

			carry = *current_grun;

			for (auto i = range->begin(), end = range->end(), previous = i;
				populate_glyph_run(_glyphs, *current_grun, limit_processor, _limit_width - current_line->width, i, end);
				previous = i)
			{
				if ((i == previous) & current_line->empty())
				{
					// Emergency: width limit is too small to layout even a single character - bailing out!
					_text_lines.clear();
					return;
				}
				else if (!commit_glyph_run(*current_line, current_grun, carry))
				{
					*current_grun = carry;
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

	bool layout::commit_glyph_run(text_line &current_line, glyph_run *&current_grun, const glyph_run &carry)
	{
		if (current_grun->empty())
			return false;	// Nothing to commit.
		current_line.width += current_grun->width;
		current_line.extend_end();
		current_grun = &*_glyph_runs.insert(_glyph_runs.end(), carry);
		return true;	// Last glyph run committed, next line's glyph run is pushed to container to be committed later.
	}
}
