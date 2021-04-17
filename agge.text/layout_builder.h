#pragma once

#include "font.h"
#include "layout_primitives.h"

namespace agge
{
	class layout_builder : noncopyable
	{
	public:
		struct state
		{
			real_t extent;
			count_t next;

			bool operator !() const;
			bool operator <(const state &rhs) const;
		};

	public:
		layout_builder(positioned_glyphs_container_t &glyphs, glyph_runs_container_t &glyph_runs,
			text_lines_container_t &text_lines);

		void begin_style(const font::ptr &font_);

		void append_glyph(glyph_index_t index, real_t advance);
		void trim_current_line(const state &at);
		bool break_current_line();
		const state &get_state() const;

		void break_current_line(const state &at, const state &resume_at);

	private:
		bool commit_line();

	private:
		state _state;

		glyph_run *_current_run;
		text_line *_current_line;
		real_t _implicit_height;

		positioned_glyphs_container_t &_glyphs;
		glyph_runs_container_t &_glyph_runs;
		text_lines_container_t &_text_lines;
	};



	inline void layout_builder::append_glyph(glyph_index_t index, real_t advance)
	{
		positioned_glyph &pg = _glyphs[static_cast<count_t>(_state.next++)];

		pg.index = index;
		pg.d.dx = advance, pg.d.dy = real_t();
		_state.extent += advance;
	}

	inline const layout_builder::state &layout_builder::get_state() const
	{	return _state;	}
}
