#pragma once

#include <agge.text/text_engine.h>

struct HFONT__;
typedef struct HFONT__ *HFONT;

class font_accessor : public agge::font::accessor
{
public:
	font_accessor(int height, const char *family, bool bold, bool italic, agge::font_hinting grid_fit);

	HFONT native() const;

private:
	virtual agge::font_metrics get_metrics() const;
	virtual agge::uint16_t get_glyph_index(wchar_t character) const;
	virtual agge::glyph::outline_ptr load_glyph(agge::uint16_t index, agge::glyph::glyph_metrics &m) const;

private:
	agge::shared_ptr<void> _native;
	agge::font_hinting _grid_fit;
};
