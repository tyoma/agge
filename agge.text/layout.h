#pragma once

#include "font.h"
#include "layout_primitives.h"
#include "richtext.h"
#include "tools.h"
#include "utf8.h"

#include <agge/tools.h>

namespace agge
{
	struct font_factory;

	class layout : noncopyable
	{
	public:
		typedef std::vector<text_line> text_lines_container_t;
		typedef text_lines_container_t::const_iterator const_iterator;

	public:
		layout(font_factory &factory);

		void process(const richtext_t &text);

		void set_width_limit(real_t width);
		box_r get_box() const;

		const_iterator begin() const;
		const_iterator end() const;

		template <typename ContainerT, typename LimitProcessorT, typename CharIteratorT>
		static bool /*end-of-line*/ populate_glyph_run(ContainerT &glyphs, glyph_run &accumulator,
			LimitProcessorT &limit_processor, const real_t limit, CharIteratorT &i, CharIteratorT text_end);

	private:
		void commit_glyph_run(text_line &current_line, glyph_run *&current);
		static std::pair<real_t /*ascent*/, real_t /*descent + leading*/> setup_line_metrics(text_line &line);
		static std::pair<real_t /*ascent*/, real_t /*descent + leading*/> setup_line_metrics(text_line &line,
			const font_metrics &m);

	private:
		font_factory &_factory;
		positioned_glyphs_container_t _glyphs;
		glyph_runs_container_t _glyph_runs;
		text_lines_container_t _text_lines;
		real_t _limit_width;
		box_r _box;
	};



	inline void layout::set_width_limit(real_t width)
	{
		_limit_width = width;
		_text_lines.clear();
	}

	inline box_r layout::get_box() const
	{	return _box;	}

	inline layout::const_iterator layout::begin() const
	{	return _text_lines.begin();	}

	inline layout::const_iterator layout::end() const
	{	return _text_lines.end();	}

	template <typename ContainerT, typename LimitProcessorT, typename CharIteratorT>
	inline bool /*end-of-line*/ layout::populate_glyph_run(ContainerT &glyphs, glyph_run &accumulator,
		LimitProcessorT &limit_processor, const real_t limit, CharIteratorT &i, CharIteratorT text_end)
	{
		bool eol = false;
		real_t occupied = accumulator.width;
		size_t end_index = accumulator.end_index;

		while (i != text_end)
		{
			if (eat_lf(i)) // Next line - line-feed
			{
				eol = true;
				break;
			}

			CharIteratorT i_next = i;
			const glyph *const g = accumulator.font_->get_glyph_for_codepoint(utf8::next(i_next, text_end));
			const real_t advance = g->metrics.advance_x;

			limit_processor.analyze_character(*i, end_index, occupied);
			if (occupied + advance > limit)
			{
				limit_processor.on_limit_reached(i, text_end, end_index, occupied);
				eol = true;
				break;
			}

			const positioned_glyph pg = {	create_vector(advance, 0.0f), g->index	};

			glyphs.push_back(pg);
			occupied += advance;
			end_index++;
			i = i_next;
		}
		accumulator.width = occupied;
		accumulator.end_index = end_index;
		return eol;
	}
}
