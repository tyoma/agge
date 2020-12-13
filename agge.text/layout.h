#pragma once

#include "font.h"

#include <agge/types.h>
#include <string>

namespace agge
{
	class layout : noncopyable
	{
	public:
		enum halign { near_, far_, center, };

		struct positioned_glyph;
		struct glyph_run;
		typedef pod_vector<glyph_run> glyph_runs_container;
		typedef pod_vector<positioned_glyph> positioned_glyphs_container;
		typedef glyph_runs_container::const_iterator const_iterator;

	public:
		layout(const wchar_t *text, font::ptr font_);

		void limit_width(real_t width);
		box_r get_box();

		const_iterator begin() const;
		const_iterator end() const;

	public:
		void analyze();

	private:
		std::wstring _text;
		font::ptr _font;
		positioned_glyphs_container _glyphs;
		glyph_runs_container _glyph_runs;
		real_t _limit_width;
	};

	struct layout::positioned_glyph
	{
		vector_r d;
		uint16_t index;
	};

	struct layout::glyph_run
	{
		font::ptr glyph_run_font;
		point_r reference;
		real_t width;
		layout::positioned_glyphs_container::const_iterator begin, end;
	};



	inline layout::const_iterator layout::begin() const
	{	return _glyph_runs.begin();	}

	inline layout::const_iterator layout::end() const
	{	return _glyph_runs.end();	}
}
