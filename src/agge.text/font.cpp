#include <agge.text/font.h>

#include <agge.text/glyph.h>

using namespace std;

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

	const glyph *font::get_glyph(wchar_t character) const
	{
		uint16_t index = get_glyph_index(character);

		if (index == 0xFFFF)
			return 0;

		glyphs_cache_t::const_iterator i = _glyphs.find(index);

		if (_glyphs.end() == i)
		{
			const glyph *g = load_glyph(index);

			i = _glyphs.insert(make_pair(index, g)).first;
		}
		return i->second;
	}
}
