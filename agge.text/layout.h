#pragma once

#include "layout_primitives.h"
#include "richtext.h"

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

	private:
		bool commit_glyph_run(text_line &current_line, glyph_run *&current_grun, const glyph_run &next_line_grun);

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
}
