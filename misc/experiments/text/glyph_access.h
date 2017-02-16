#pragma once

#include "../common/paths.h"

#include <tchar.h>
#include <vector>

namespace std { namespace tr1 { } using namespace tr1; }

struct HDC__;
typedef struct HDC__ *HDC;

namespace common
{
	typedef AggPath glyph_outline;

	void get_glyph_indices(HDC hdc, const TCHAR *text, std::vector<agge::uint16_t> &indices);
	agge::real_t get_glyph_dx(HDC hdc, agge::uint16_t index);
	void get_glyph_outline(HDC hdc, agge::uint16_t index, glyph_outline &outline);
}
