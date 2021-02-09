#include <agge.text/font.h>

using namespace std;

namespace agge
{
	font::font(const accessor_ptr &accessor_, real_t factor)
		: _accessor(accessor_), _metrics(accessor_->get_metrics()), _factor(factor)
	{
		_metrics.ascent *= _factor;
		_metrics.descent *= _factor;
		_metrics.leading *= _factor;
	}

	font_descriptor font::get_key() const
	{	return _accessor->get_descriptor();	}

	font_metrics font::get_metrics() const
	{	return _metrics;	}

	glyph_index_t font::load_mapping(codepoint_t character) const
	{
		char2index_cache_t::iterator inserted;

		_char2glyph.insert(character, _accessor->get_glyph_index(character), inserted);
		return inserted->second;
	}

	const glyph* font::load_glyph(glyph_index_t index) const
	{
		glyphs_cache_t::iterator inserted;
		glyph g;

		g.factor = _factor;
		g.outline = _accessor->load_glyph(index, g.metrics);
		g.metrics.advance_x *= _factor;
		g.metrics.advance_y *= _factor;
		_glyphs.insert(index, g, inserted);
		return inserted->second.outline ? &inserted->second : 0;
	}
}
