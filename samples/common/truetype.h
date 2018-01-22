#pragma once

#include <agge.text/font.h>
#include <memory>
#include <unordered_map>
#include <vector>

namespace truetype
{
	struct segment
	{
		enum segment_type { line = 1, qspline = 2, cspline = 3 } type;
		std::vector< agge::point<int> > points;
	};

	struct poly
	{
		agge::point<int> start;
		std::vector<segment> segments;
	};

	struct glyph
	{
		struct glyph_metrics
		{
			int advance_x, advance_y;
		};

		glyph_metrics metrics;
		std::vector<poly> polygons;
	};

	struct font
	{
		struct font_metrics
		{
			int ascent, descent, leading;
		};

		font_metrics metrics;
		std::unordered_map<wchar_t, agge::uint16_t> char_to_glyph;
		std::vector<glyph> glyphs;
	};

	agge::font::accessor_ptr create_accessor(const std::shared_ptr<font> &tt_font);
}
