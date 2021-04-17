#include <agge.text/font.h>

#include <agge/memory.h>

namespace agge
{
	font::font(const accessor_ptr &accessor_, real_t factor)
		: _accessor(accessor_), _metrics(accessor_->get_metrics()), _factor(factor)
	{
		_metrics.ascent *= _factor;
		_metrics.descent *= _factor;
		_metrics.leading *= _factor;
		memset(_ansi_glyphs, static_cast<const glyph *>(nullptr), ansi_range);
	}

	font_descriptor font::get_key() const
	{	return _accessor->get_descriptor();	}

	font_metrics font::get_metrics() const
	{	return _metrics;	}

	glyph_index_t font::load_mapping(codepoint_t character) const
	{
		char2index_cache_t::iterator m = _char2glyph.find(character);

		return m != _char2glyph.end()
			? m->second
			: _char2glyph.insert(make_pair(character, _accessor->get_glyph_index(character))).first->second;
	}

	const glyph* font::load_glyph(glyph_index_t index) const
	{
		glyphs_cache_t::iterator inserted;
		glyph g;

		g.outline = _accessor->load_glyph(index, g.metrics);
		if (!g.outline)
			return 0;
		g.factor = _factor;
		g.metrics.advance_x *= _factor;
		g.metrics.advance_y *= _factor;
		g.index = index;
		return &_glyphs.insert(make_pair(index, g)).first->second;
	}

	const glyph* font::get_glyph_for_codepoint_slow(codepoint_t codepoint) const
	{
		const glyph* g = get_glyph(map_single(codepoint));

		if (codepoint < ansi_range)
			_ansi_glyphs[codepoint] = g;
		return g;
	}
}
