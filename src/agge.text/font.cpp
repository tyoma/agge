#include <agge.text/font.h>

namespace agge
{
	font::font(const accessor_ptr &accessor_)
		: _accessor(accessor_), _metrics(accessor_->get_metrics())
	{	}
	
	font::metrics font::get_metrics() const
	{	return _metrics;	}

	uint16_t font::map_single(wchar_t character) const
	{
		char2index_cache_t::const_iterator i = _char2glyph.find(character);

		if (_char2glyph.end() == i)
		{
			char2index_cache_t::iterator inserted;

			_char2glyph.insert(character, _accessor->get_glyph_index(character), inserted);
			i = inserted;
		}
		return i->second;
	}

	const glyph *font::get_glyph(uint16_t index) const
	{
		glyphs_cache_t::iterator i = _glyphs.find(index);

		if (_glyphs.end() == i)
		{
			glyphs_cache_t::iterator inserted;
			glyph g;

			g.outline = _accessor->load_glyph(index, g.metrics);
			_glyphs.insert(index, g, i);
		}
		return i->second.outline ? &i->second : 0;
	}
}
