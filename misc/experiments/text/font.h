#pragma once

#include <agge.text/font.h>

struct HFONT__;
typedef struct HFONT__ *HFONT;

namespace demo
{
	class font : public agge::font
	{
	public:
		static agge::shared_ptr<font> create(int height, const wchar_t *typeface, bool bold, bool italic);

		HFONT native() const;

	private:
		font(const metrics &m, std::shared_ptr<void> native);

		virtual agge::uint16_t get_glyph_index(wchar_t character) const;
		virtual const agge::glyph *load_glyph(agge::uint16_t index) const;
		virtual agge::pod_vector<kerning_pair> load_kerning() const;

	private:
		agge::shared_ptr<void> _native;
	};		 
}
