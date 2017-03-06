#pragma once

#include <agge.text/font.h>

struct HFONT__;
typedef struct HFONT__ *HFONT;

namespace demo
{
	class win32_font_accessor : public agge::font::accessor
	{
	public:
		win32_font_accessor(int height, const wchar_t *typeface, bool bold, bool italic);

		HFONT native() const;

	private:
		virtual agge::font::metrics get_metrics() const;
		virtual agge::uint16_t get_glyph_index(wchar_t character) const;
		virtual bool load_glyph(agge::uint16_t index, agge::glyph::glyph_metrics &m,
			agge::glyph::outline_storage &o) const;

	private:
		agge::shared_ptr<void> _native;
	};		 
}
