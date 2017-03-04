#include <agge.text/font.h>

namespace agge
{
	font::font(const metrics &metrics_)
		: _metrics(metrics_)
	{	}

	font::~font()
	{
		for (glyphs_cache_t::const_iterator i = _glyphs.begin(); i != _glyphs.end(); ++i)
			delete i->second;
	}
	
	font::metrics font::get_metrics() const
	{	return _metrics;	}

	uint16_t font::map_single(wchar_t character) const
	{
		char2index_cache_t::const_iterator i = _char2glyph.find(character);

		if (_char2glyph.end() == i)
		{
			char2index_cache_t::iterator inserted;

			_char2glyph.insert(character, get_glyph_index(character), inserted);
			i = inserted;
		}
		return i->second;
	}

	const glyph *font::get_glyph(uint16_t index) const
	{
		glyphs_cache_t::const_iterator i = _glyphs.find(index);

		if (_glyphs.end() == i)
		{
			glyphs_cache_t::iterator inserted;

			_glyphs.insert(index, 0, inserted);
			inserted->second = load_glyph(index); // Avoid memory leak on exception in insert().
			i = inserted;
		}
		return i->second;
	}
}
