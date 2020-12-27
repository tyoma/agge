#pragma once

#include "layout_primitives.h"
#include "richtext.h"

namespace agge
{
	class layout : noncopyable
	{
	public:
		typedef glyph_runs_container_t::const_iterator const_iterator;

	public:
		layout(shared_ptr<font> base_font);

		void process(const richtext_t &text);

		void set_width_limit(real_t width);
		box_r get_box();

		const_iterator begin() const;
		const_iterator end() const;

	public:
		void new_line(glyph_run &range_, real_t dy);

	private:
		shared_ptr<font> _base_font;
		positioned_glyphs_container_t _glyphs;
		glyph_runs_container_t _glyph_runs;
		real_t _limit_width;
	};



	inline layout::const_iterator layout::begin() const
	{	return _glyph_runs.begin();	}

	inline layout::const_iterator layout::end() const
	{	return _glyph_runs.end();	}
}
