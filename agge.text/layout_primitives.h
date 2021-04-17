#pragma once

#include "range.h"
#include "shared_ptr.h"
#include "types.h"

#include <agge/pod_vector.h>
#include <agge/tools.h>
#include <vector>

namespace agge
{
	class font;
	struct glyph_run;
	struct positioned_glyph;
	struct text_line;

	typedef std::vector<glyph_run> glyph_runs_container_t;
	typedef pod_vector<positioned_glyph> positioned_glyphs_container_t;
	typedef std::vector<text_line> text_lines_container_t;

	struct positioned_glyph
	{
		glyph_index_t index;
		vector_r d;
	};

	struct glyph_run : range<const positioned_glyphs_container_t>
	{
		glyph_run(const positioned_glyphs_container_t &container);

		ptr<font> font_;
		vector_r offset;
	};

	struct text_line : range<const glyph_runs_container_t>
	{
		text_line(const glyph_runs_container_t &container);

		vector_r offset;
		real_t extent;
		real_t descent;
	};



	inline glyph_run::glyph_run(const positioned_glyphs_container_t &container)
		: range<const positioned_glyphs_container_t>(container), offset(zero())
	{	}


	inline text_line::text_line(const glyph_runs_container_t &container)
		: range<const glyph_runs_container_t>(container), offset(zero()), extent(real_t())
	{	}
}
