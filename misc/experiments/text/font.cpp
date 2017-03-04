#include "font.h"

#include "../common/dc.h"

#include <windows.h>

using namespace agge;
using namespace std;

namespace demo
{
	namespace
	{
		const short factor = 1;
		const short xfactor = 1;
		const UINT c_format = GGO_GLYPH_INDEX | GGO_NATIVE | /*GGO_UNHINTED |*/ GGO_METRICS;
		const MAT2 c_identity = { { 0, xfactor * factor }, { 0, 0 }, { 0, 0 }, { 0, -factor }, };

		real_t fixed2real(FIXED value)
		{
			return (value.value + value.fract / 65536.0f) / factor;
		}

		glyph::path_point path_point(int command, real_t x, real_t y)
		{
			glyph::path_point p = { command, x, y };
			return p;
		}

		void bezier2(glyph::outline_storage &outline, real_t x2, real_t y2, real_t x3, real_t y3, real_t d = 0.03f)
		{
			const real_t x1 = (outline.end() - 1)->x, y1 = (outline.end() - 1)->y;
			
			for (real_t t = d; t < 1.0f; t += d)
			{
				real_t t_ = 1.0f - t;
				
				outline.push_back(path_point(path_command_line_to,
					t_ * t_ * x1 + 2.0f * t_ * t * x2 + t * t * x3,
					t_ * t_ * y1 + 2.0f * t_ * t * y2 + t * t * y3));
			}
			outline.push_back(path_point(path_command_line_to, x3, y3));
		}
	}

	std::shared_ptr<font> font::create(int height, const wchar_t *typeface, bool bold, bool italic)
	{
		shared_ptr<void> native(::CreateFontW(height, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, !!italic, FALSE, FALSE, 0,
			CLEARTYPE_NATURAL_QUALITY, 0, 0, 0, typeface), &::DeleteObject);
		dc ctx;
		dc::handle h(ctx.select(static_cast<HFONT>(native.get())));
		metrics m;
		TEXTMETRIC tm;


		::GetTextMetrics(ctx, &tm);
		m.ascent = static_cast<real_t>(tm.tmAscent);
		m.descent = static_cast<real_t>(tm.tmDescent);
		m.leading = static_cast<real_t>(tm.tmExternalLeading);
		return std::shared_ptr<font>(new font(m, native));
	}

	font::font(const metrics &m, std::shared_ptr<void> native)
		: agge::font(m), _native(native)
	{
		dc ctx;
		dc::handle h(ctx.select(this->native()));
		wchar_t chars[0x10000];
		uint16_t indices[_countof(chars)] = {};

		for (int i = 0; i != _countof(chars); ++i)
			chars[i] = (wchar_t)i;

		int converted = ::GetGlyphIndicesW(ctx, chars, _countof(chars), indices, 0);
		for (int i = 0; i != converted; ++i)
			if (indices[i] != 0xffff)
				_char2index[(wchar_t)i] = indices[i];
	}

	HFONT font::native() const
	{	return static_cast<HFONT>(_native.get());	}

	uint16_t font::get_glyph_index(wchar_t character) const
	{
		char2index::const_iterator i = _char2index.find(character);
		return i != _char2index.end() ? i->second : 0xffff;
	}

	const glyph *font::load_glyph(uint16_t index) const
	{
		typedef const void *pvoid;

		GLYPHMETRICS gm;
		auto_ptr<glyph> g(new glyph);
		dc ctx;
		dc::handle h(ctx.select(native()));
		const int size = ::GetGlyphOutline(ctx, index, c_format, &gm, 0, 0, &c_identity);

		if (size == GDI_ERROR)
			return 0;

		g->index = index;
		if (1 == xfactor * factor)
		{
			ABC abc;

			::GetCharABCWidthsI(ctx, index, 1, 0, &abc);
			g->advance_x = static_cast<real_t>(abc.abcA + abc.abcB + abc.abcC);
		}
		else
			g->advance_x = static_cast<real_t>(gm.gmCellIncX) / xfactor / factor;
		g->advance_y = static_cast<real_t>(gm.gmCellIncY);
		g->outline.reset(new glyph::outline_storage);

		if (size)
		{
			vector<uint8_t> buffer(size);
			::GetGlyphOutline(ctx, index, c_format, &gm, size, &buffer[0], &c_identity);
			for (pvoid p = &buffer[0], end = &buffer[0] + size; p != end; )
			{
				const TTPOLYGONHEADER *header = static_cast<const TTPOLYGONHEADER *>(p);
				const pvoid next_poly = static_cast<const uint8_t *>(p) + header->cb;

				p = header + 1;
				g->outline->push_back(path_point(path_command_move_to, fixed2real(header->pfxStart.x) / xfactor,
					fixed2real(header->pfxStart.y)));

				for (const TTPOLYCURVE *curve; curve = static_cast<const TTPOLYCURVE *>(p), p != next_poly;
					p = static_cast<const uint8_t *>(p) + sizeof(TTPOLYCURVE) + (curve->cpfx - 1) * sizeof(POINTFX))
				{
					switch (curve->wType)
					{
					case TT_PRIM_LINE:
						for (WORD i = 0; i != curve->cpfx; ++i)
						{
							g->outline->push_back(path_point(path_command_line_to,
								fixed2real(curve->apfx[i].x) / xfactor, fixed2real(curve->apfx[i].y)));
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
							bezier2(*g->outline, fixed2real(pnt_b.x) / xfactor, fixed2real(pnt_b.y),
								fixed2real(pnt_c.x) / xfactor, fixed2real(pnt_c.y));
						}
						break;

					case TT_PRIM_CSPLINE:
						break;
					}
				}
				(g->outline->end() - 1)->command |= path_flag_close;
				p = next_poly;
			}
		}
		return g.release();
	}

	pod_vector<font::kerning_pair> font::load_kerning() const
	{
		throw 0;
	}
}
