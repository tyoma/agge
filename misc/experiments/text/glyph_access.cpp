#include "glyph_access.h"

#include <agge/path.h>
#include <string.h>
#include <windows.h>

using namespace agge;
using namespace std;

namespace common
{
	namespace
	{
		const MAT2 c_identity = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 }, };
		const MAT2 c_identity2 = { { 0, 256 }, { 0, 0 }, { 0, 0 }, { 0, 256 }, };
		
		real_t fixed2real(FIXED value)
		{
			return value.value + value.fract / 65536.0f;
		}
	}

	void get_glyph_indices(HDC hdc, const TCHAR *text, vector<uint16_t> &indices)
	{
		int size = static_cast<int>(_tcslen(text));

		indices.resize(size);
		if (size)
			::GetGlyphIndices(hdc, text, size, &indices[0], 0);
	}

	agge::real_t get_glyph_dx(HDC hdc, uint16_t index)
	{
		GLYPHMETRICS gm;

		::GetGlyphOutline(hdc, index, GGO_GLYPH_INDEX | GGO_UNHINTED, &gm, 0, 0, &c_identity2);
		return gm.gmCellIncX / 256.0f;
	}

	void get_glyph_outline(HDC hdc, uint16_t index, glyph_outline &outline)
	{
		typedef const void *pvoid;

		const UINT format = GGO_GLYPH_INDEX | GGO_NATIVE | GGO_UNHINTED;

		GLYPHMETRICS gm;
		const int size = ::GetGlyphOutline(hdc, index, format, &gm, 0, 0, &c_identity);

		if (size == GDI_ERROR)
			throw 0;

		vector<uint8_t> buffer(size);

		outline.clear();
		::GetGlyphOutline(hdc, index, format, &gm, size, &buffer[0], &c_identity);
		for (pvoid p = &buffer[0], end = &buffer[0] + size; p != end; )
		{
			const TTPOLYGONHEADER *header = static_cast<const TTPOLYGONHEADER *>(p);
			const pvoid next_poly = static_cast<const uint8_t *>(p) + header->cb;

			p = header + 1;
			outline.push_back(make_pair(make_pair(fixed2real(header->pfxStart.x), fixed2real(header->pfxStart.y)),
				path_command_move_to));
			for (const TTPOLYCURVE *curve; curve = static_cast<const TTPOLYCURVE *>(p), p != next_poly;
				p = static_cast<const uint8_t *>(p) + sizeof(TTPOLYCURVE) + (curve->cpfx - 1) * sizeof(POINTFX))
			{
				for (WORD i = 0; i != curve->cpfx; ++i)
				{
					outline.push_back(make_pair(make_pair(fixed2real(curve->apfx[i].x), fixed2real(curve->apfx[i].y)),
						path_command_line_to));
				}
			}
			outline.back().second |= path_flag_close;
			p = next_poly;
		}
	}
}
