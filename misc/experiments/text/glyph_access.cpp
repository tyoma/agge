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
		const short factor = 1;
		const short factor2 = 100;
		const UINT c_format = GGO_GLYPH_INDEX | GGO_NATIVE /*| GGO_UNHINTED*/ /*| GGO_METRICS*/;
		const MAT2 c_identity = { { 0, 50 * factor }, { 0, 0 }, { 0, 0 }, { 0, -factor }, };
		const MAT2 c_identity2 = { { 0, factor2 }, { 0, 0 }, { 0, 0 }, { 0, -factor2 }, };

		real_t fixed2real(FIXED value)
		{
			return (value.value + value.fract / 65536.0f) / factor;
		}

		void bezier2(glyph_outline &outline, real_t x2, real_t y2, real_t x3, real_t y3, real_t d = 0.1f)
		{
			const real_t x1 = outline.back().first.first, y1 = outline.back().first.second;
			
			for (real_t t = d; t < 1.0f; t += d)
			{
				real_t t_ = 1.0f - t;
				real_t x = t_ * t_ * x1 + 2.0f * t_ * t * x2 + t * t * x3;
				real_t y = t_ * t_ * y1 + 2.0f * t_ * t * y2 + t * t * y3;
				
				outline.push_back(make_pair(make_pair(x, y), path_command_line_to));
			}
			outline.push_back(make_pair(make_pair(x3, y3), path_command_line_to));
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

		::GetGlyphOutline(hdc, index, c_format, &gm, 0, 0, &c_identity2);
		return gm.gmCellIncX / static_cast<real_t>(factor2);
	}

	void get_glyph_outline(HDC hdc, uint16_t index, glyph_outline &outline)
	{
		typedef const void *pvoid;

		GLYPHMETRICS gm;
		const int size = ::GetGlyphOutline(hdc, index, c_format, &gm, 0, 0, &c_identity);

		outline.clear();
		if (size == GDI_ERROR)
			return;

		vector<uint8_t> buffer(size);

		if (!size)
			return;
		::GetGlyphOutline(hdc, index, c_format, &gm, size, &buffer[0], &c_identity);
		for (pvoid p = &buffer[0], end = &buffer[0] + size; p != end; )
		{
			const TTPOLYGONHEADER *header = static_cast<const TTPOLYGONHEADER *>(p);
			const pvoid next_poly = static_cast<const uint8_t *>(p) + header->cb;

			p = header + 1;
			outline.push_back(make_pair(make_pair(fixed2real(header->pfxStart.x) / 50, fixed2real(header->pfxStart.y)),
				path_command_move_to));

			for (const TTPOLYCURVE *curve; curve = static_cast<const TTPOLYCURVE *>(p), p != next_poly;
				p = static_cast<const uint8_t *>(p) + sizeof(TTPOLYCURVE) + (curve->cpfx - 1) * sizeof(POINTFX))
			{
				switch (curve->wType)
				{
				case TT_PRIM_LINE:
					for (WORD i = 0; i != curve->cpfx; ++i)
					{
						outline.push_back(make_pair(make_pair(fixed2real(curve->apfx[i].x) / 50, fixed2real(curve->apfx[i].y)),
							path_command_line_to));
					}
					break;

				case TT_PRIM_QSPLINE:
					for (WORD i = 0; i < curve->cpfx - 1; ++i)
					{
                  POINTFX pnt_b = curve->apfx[i]; // B is always the current point
                  POINTFX pnt_c = curve->apfx[i + 1];

						if (i < curve->cpfx - 2) // If not on last spline, compute C
						{
							// midpoint (x,y)
							*(int*)&pnt_c.x = (*(int*)&pnt_b.x + *(int*)&pnt_c.x) / 2;
							*(int*)&pnt_c.y = (*(int*)&pnt_b.y + *(int*)&pnt_c.y) / 2;
						}
						bezier2(outline, fixed2real(pnt_b.x) / 50, fixed2real(pnt_b.y), fixed2real(pnt_c.x) / 50, fixed2real(pnt_c.y));
					}
					break;

				case TT_PRIM_CSPLINE:
					break;
				}
			}
			outline.back().second |= path_flag_close;
			p = next_poly;
		}
	}
}
