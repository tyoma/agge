#pragma once

#include <agge/types.h>
#include <string>

namespace agge
{
	typedef uint16_t glyph_index_t;

	enum text_alignment {
		align_near,	// vertical: top, horizontal: LTR -> left, RTL -> right
		align_far,	// vertical: bottom, horizontal: LTR -> right, RTL -> left
		align_center,
	};

	enum font_weight {
		extra_light,
		light,
		book,
		regular,
		medium,
		semi_bold,
		bold,
		black,
		extra_black,
	};

	enum font_hinting {
		hint_none,
		hint_vertical,
		hint_strong,
	};

	struct font_metrics
	{
		real_t ascent;
		real_t descent;
		real_t leading;
	};

	struct font_descriptor
	{
		explicit font_descriptor(const std::string &family, int height, bool bold = false, bool italic = false,
			font_hinting hinting_ = hint_none);

		std::string family;
		int height : 20;
		unsigned bold : 1;
		unsigned italic : 1;
		font_hinting hinting : 2;
	};



	bool operator ==(const font_descriptor &lhs, const font_descriptor &rhs);
}
