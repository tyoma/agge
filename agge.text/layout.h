#pragma once

#include "layout_primitives.h"
#include "richtext.h"

namespace agge
{
	class layout : noncopyable
	{
	public:
		typedef std::vector<text_line> text_lines_container_t;
		typedef text_lines_container_t::const_iterator const_iterator;

	public:
		layout(shared_ptr<font> base_font);

		void process(const richtext_t &text);

		void set_width_limit(real_t width);
		box_r get_box();

		const_iterator begin() const;
		const_iterator end() const;

	private:
		shared_ptr<font> _base_font;
		positioned_glyphs_container_t _glyphs;
		glyph_runs_container_t _glyph_runs;
		text_lines_container_t _text_lines;
		real_t _limit_width;
	};



	inline layout::const_iterator layout::begin() const
	{	return _text_lines.begin();	}

	inline layout::const_iterator layout::end() const
	{	return _text_lines.end();	}
}
