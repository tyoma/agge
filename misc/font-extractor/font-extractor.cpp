#include "truetype.h"

#include <agge.text/font.h>
#include <fcntl.h>
#include <io.h>
#include <samples/common/platform/win32/dc.h>
#include <stdio.h>
#include <strmd/serializer.h>
#include <windows.h>

#pragma warning(disable:4996)

using namespace agge;
using namespace std;

namespace strmd
{
	template <> struct is_arithmetic<wchar_t> { static const bool value = true; };
}

namespace
{
	using agge::uint16_t;

	real_t fixed2real(FIXED value)
	{	return static_cast<real_t>(value.value + value.fract / 65536.0);	}

	int fixed2fint(FIXED value)
	{	return (static_cast<int>(value.value) << 16) | value.fract;	}

	int real2fint(real_t value)
	{	return static_cast<int>(value * (1 << 16));	}

	bool load_glyph(HDC hdc, font::key::grid_fit grid_fit, uint16_t index, truetype::glyph &g)
	{
		typedef const void *pvoid;

		const UINT format = GGO_GLYPH_INDEX | GGO_NATIVE | GGO_METRICS
			| (font::key::gf_none == grid_fit ? GGO_UNHINTED : 0);
		const int xfactor = font::key::gf_vertical == grid_fit ? 48 : 1;
		const MAT2 c_identity = { { 0, (short)xfactor }, { 0, 0 }, { 0, 0 }, { 0, -1 }, };

		GLYPHMETRICS gm;
		const int size = ::GetGlyphOutline(hdc, index, format, &gm, 0, 0, &c_identity);

		if (size == GDI_ERROR)
			return false;

		if (grid_fit == font::key::gf_strong)
		{
			ABC abc;

			::GetCharABCWidthsI(hdc, index, 1, 0, &abc);
			g.metrics.advance_x = real2fint(static_cast<real_t>(abc.abcA + abc.abcB + abc.abcC));
		}
		else
			g.metrics.advance_x = real2fint(static_cast<real_t>(gm.gmCellIncX) / xfactor);
		g.metrics.advance_y = real2fint(static_cast<real_t>(gm.gmCellIncY));
		if (size)
		{
			pod_vector<agge::uint8_t> buffer(size);
			::GetGlyphOutline(hdc, index, format, &gm, size, &buffer[0], &c_identity);
			for (pvoid p = &buffer[0], end = &buffer[0] + size; p != end; )
			{
				g.polygons.push_back(truetype::poly());

				truetype::poly &poly = g.polygons.back();

				const TTPOLYGONHEADER *header = static_cast<const TTPOLYGONHEADER *>(p);
				const pvoid next_poly = static_cast<const agge::uint8_t *>(p) + header->cb;

				p = header + 1;
				poly.start.x = real2fint(fixed2real(header->pfxStart.x) / xfactor);
				poly.start.y = real2fint(fixed2real(header->pfxStart.y) / xfactor);

				for (const TTPOLYCURVE *curve; curve = static_cast<const TTPOLYCURVE *>(p), p != next_poly;
					p = static_cast<const agge::uint8_t *>(p) + sizeof(TTPOLYCURVE) + (curve->cpfx - 1) * sizeof(POINTFX))
				{
					poly.segments.push_back(truetype::segment());

					truetype::segment &segment = poly.segments.back();

					segment.type = static_cast<truetype::segment::segment_type>(curve->wType);
					for (WORD i = 0; i != curve->cpfx; ++i)
					{
						point<int> pt = { real2fint(fixed2real(curve->apfx[i].x) / xfactor), fixed2fint(curve->apfx[i].y) };
						segment.points.push_back(pt);
					}
				}
				p = next_poly;
			}
		}
		return true;
	}

	void delete_object(HGDIOBJ hobject)
	{	::DeleteObject(hobject);	}

	class stream_writer
	{
	public:
		stream_writer(FILE *s)
			: _stream(s)
		{	setmode(fileno(_stream), O_BINARY);	}

		void write(const void *data, size_t size)
		{	fwrite(data, 1, size, _stream);	}

	private:
		FILE *_stream;
	};
}

int main(int /*argc*/, const char * /*argv*/[])
{
	dc ctx;
	shared_ptr<void> hfont(::CreateFontA(10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 0, ANTIALIASED_QUALITY, 0, 0, 0,
		"Calibri"), &delete_object);
	shared_ptr<void> selector = ctx.select(hfont.get());
	truetype::glyph g;
	truetype::font f;

	for (agge::uint16_t index = 0; load_glyph(ctx, font::key::gf_none, index, g); ++index)
	{
		f.glyphs.push_back(g);
		g = truetype::glyph();
	}

	stream_writer w(stdout);
	strmd::serializer<stream_writer, strmd::varint> ser(w);

	ser(f);
}
