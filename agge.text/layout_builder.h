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
			size_t next, runs_size;
			real_t extent;

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
		void commit_line();

	private:
		void commit_run();

	private:
		state _state;

		glyph_run *_current_run;
		text_line *_current_line;
		real_t _implicit_height;

		positioned_glyphs_container_t &_glyphs;
		glyph_runs_container_t &_glyph_runs;
		text_lines_container_t &_text_lines;
	};



	inline const layout_builder::state &layout_builder::get_state() const
	{	return _state;	}
}
