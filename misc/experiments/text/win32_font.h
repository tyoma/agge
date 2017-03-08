#pragma once

#include <agge.text/font_engine.h>

struct HFONT__;
typedef struct HFONT__ *HFONT;

namespace demo
{
	class win32_font_accessor : public agge::font::accessor
	{
	public:
		win32_font_accessor(int height, const wchar_t *typeface, bool bold, bool italic,
			agge::font_engine::grid_fit grid_fit);

		HFONT native() const;

	private:
		virtual agge::font::metrics get_metrics() const;
		virtual agge::uint16_t get_glyph_index(wchar_t character) const;
		virtual agge::glyph::outline_ptr load_glyph(agge::uint16_t index, agge::glyph::glyph_metrics &m) const;

	private:
		agge::shared_ptr<void> _native;
		agge::font_engine::grid_fit _grid_fit;
	};		 

	class win32_font_loader : public agge::font_engine::loader
	{
		virtual agge::font::accessor_ptr load(const wchar_t *typeface, int height, bool bold, bool italic,
			agge::font_engine::grid_fit grid_fit)
		{
			return agge::font::accessor_ptr(new win32_font_accessor(height, typeface, bold, italic, grid_fit));
		}
	};
}
