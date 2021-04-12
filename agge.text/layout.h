#pragma once

#include "font.h"
#include "layout_primitives.h"
#include "richtext.h"
#include "tools.h"
#include "utf8.h"

#include <agge/math.h>
#include <agge/tools.h>

namespace agge
{
	class layout : noncopyable
	{
	public:
		typedef std::vector<text_line> text_lines_container_t;
		typedef text_lines_container_t::const_iterator const_iterator;

	public:
		template <typename LimitProcessorT, typename FontFactoryT>
		void process(const richtext_t &text, LimitProcessorT limit_processor, FontFactoryT &font_factory_);

		box_r get_box() const;

		const_iterator begin() const;
		const_iterator end() const;

		template <typename ContainerT, typename LimitProcessorT, typename CharIteratorT>
		static bool /*end-of-line*/ populate_glyph_run(ContainerT &glyphs, glyph_run &accumulator,
			LimitProcessorT &limit_processor, real_t &occupied, CharIteratorT &i, CharIteratorT text_end);

	private:
		template <typename VectorT>
		typename VectorT::value_type &duplicate_last(VectorT &container);
		void commit_glyph_run(text_line &current_line, glyph_run *&current);
		static std::pair<real_t /*ascent*/, real_t /*descent + leading*/> setup_line_metrics(text_line &text_line_);
		static std::pair<real_t /*ascent*/, real_t /*descent + leading*/> setup_line_metrics(text_line &text_line_,
			const font_metrics &m);

	private:
		positioned_glyphs_container_t _glyphs;
		glyph_runs_container_t _glyph_runs;
		text_lines_container_t _text_lines;
		box_r _box;
	};



	template <typename LimitProcessorT, typename FontFactoryT>
	inline void layout::process(const richtext_t &text, LimitProcessorT limit_processor, FontFactoryT &font_factory_)
	{
		_text_lines.clear();
		_glyph_runs.clear();
		_glyphs.clear();
		_box = zero();

		text_line *current_line = &*_text_lines.insert(_text_lines.end(), text_line(_glyph_runs));
		glyph_run *current = &*_glyph_runs.insert(_glyph_runs.end(), glyph_run(_glyphs));

		for (richtext_t::const_iterator range = text.ranges_begin(); range != text.ranges_end(); ++range)
		{
			current->font_ = font_factory_.create_font(range->get_annotation().basic);
			current->offset = create_vector(current_line->width, real_t());

			for (std::string::const_iterator i = range->begin(), end = range->end(), previous = i;
				populate_glyph_run(_glyphs, *current, limit_processor, current_line->width, i, end);
				previous = i)
			{
				if ((i == previous) & current_line->empty())
				{
					// Emergency: width limit is too small to layout even a single character - bailing out!
					_text_lines.clear();
					return;
				}
				commit_glyph_run(*current_line, current);
				real_t carry_occupied = limit_processor.init_newline(*current);

				const std::pair<real_t, real_t> m = setup_line_metrics(*current_line, current->font_->get_metrics());

				current_line->offset += create_vector(real_t(), m.first);
				if (!current_line->empty())
				{
					current_line = &duplicate_last(_text_lines);
					current_line->begin_index = current_line->end_index;
					_box.w = agge_max(_box.w, current_line->width);
				}
				current_line->width = carry_occupied;
				current_line->offset += create_vector(real_t(), m.second);
			}

			// Commit any non-empty content of the current glyph run and prepare the next one.
			commit_glyph_run(*current_line, current);
		}
		if (current_line->empty())
			_text_lines.pop_back();
		else
			current_line->offset += create_vector(real_t(), setup_line_metrics(*current_line).first);
		if (!_text_lines.empty())
		{
			const text_line &last = _text_lines.back();

			_box.w = agge_max(_box.w, last.width);
			_box.h = last.offset.dy + last.descent;
		}
	}

	inline box_r layout::get_box() const
	{	return _box;	}

	inline layout::const_iterator layout::begin() const
	{	return _text_lines.begin();	}

	inline layout::const_iterator layout::end() const
	{	return _text_lines.end();	}

	template <typename ContainerT, typename LimitProcessorT, typename CharIteratorT>
	inline bool /*end-of-line*/ layout::populate_glyph_run(ContainerT &glyphs, glyph_run &accumulator,
		LimitProcessorT &limit_processor, real_t &occupied, CharIteratorT &i, CharIteratorT text_end)
	{
		bool eol = false;
		size_t end_index = accumulator.end_index;
		real_t occupied_local = occupied;

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

			if (!limit_processor.accept_glyph(advance, i, text_end, end_index, occupied_local))
			{
				eol = true;
				break;
			}

			const positioned_glyph pg = {	g->index, create_vector(advance, 0.0f) 	};

			glyphs.push_back(pg);
			occupied_local += advance;
			end_index++;
			i = i_next;
		}
		accumulator.end_index = end_index;
		occupied = occupied_local;
		return eol;
	}

	template <typename VectorT>
	inline typename VectorT::value_type &layout::duplicate_last(VectorT &container)
	{	return container.reserve(container.size() + 1), *container.insert(container.end(), container.back());	}
}
