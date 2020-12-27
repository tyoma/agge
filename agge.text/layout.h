#pragma once

#include "font.h"
#include "richtext.h"

#include <agge/types.h>
#include <vector>

namespace agge
{
	class layout : noncopyable
	{
	public:
		enum halign { near_, far_, center, };

		struct positioned_glyph;
		struct glyph_run;
		typedef std::vector<glyph_run> glyph_runs_container;
		typedef pod_vector<positioned_glyph> positioned_glyphs_container;
		typedef glyph_runs_container::const_iterator const_iterator;

	public:
		layout(font::ptr base_font);

		void process(const richtext_t &text);

		void limit_width(real_t width);
		box_r get_box();

		const_iterator begin() const;
		const_iterator end() const;

	public:
		void analyze();

	private:
		richtext_t _text;
		font::ptr _base_font;
		positioned_glyphs_container _glyphs;
		glyph_runs_container _glyph_runs;
		real_t _limit_width;
	};

	struct layout::positioned_glyph
	{
		vector_r d;
		uint16_t index;
	};

	struct layout::glyph_run : range<const layout::positioned_glyphs_container>
	{
		glyph_run(const layout::positioned_glyphs_container &container);

		font::ptr glyph_run_font;
		point_r reference;
		real_t width;
	};



	inline layout::const_iterator layout::begin() const
	{	return _glyph_runs.begin();	}

	inline layout::const_iterator layout::end() const
	{	return _glyph_runs.end();	}

	inline layout::glyph_run::glyph_run(const layout::positioned_glyphs_container &container)
		: range<const layout::positioned_glyphs_container>(container)
	{	}
}
