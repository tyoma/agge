#pragma once

#include "range.h"
#include "shared_ptr.h"
#include "types.h"

#include <agge/pod_vector.h>
#include <vector>

namespace agge
{
	class font;
	struct glyph_run;
	typedef std::vector<glyph_run> glyph_runs_container_t;
	struct positioned_glyph;
	typedef pod_vector<positioned_glyph> positioned_glyphs_container_t;

	struct positioned_glyph
	{
		vector_r d;
		glyph_index_t index;
	};

	struct glyph_run : range<const positioned_glyphs_container_t>
	{
		glyph_run(const positioned_glyphs_container_t &container);

		shared_ptr<font> glyph_run_font;
		vector_r offset;
		real_t width;
	};

	struct text_line : range<const glyph_runs_container_t>
	{
		text_line(const glyph_runs_container_t &container);

		vector_r offset;
		real_t width;
	};



	inline glyph_run::glyph_run(const positioned_glyphs_container_t &container)
		: range<const positioned_glyphs_container_t>(container)
	{	}


	inline text_line::text_line(const glyph_runs_container_t &container)
		: range<const glyph_runs_container_t>(container)
	{	}
}
