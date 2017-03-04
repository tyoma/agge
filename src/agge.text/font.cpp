#include <agge.text/font.h>

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

	uint16_t font::map_single(wchar_t character) const
	{
		pair<char2index_cache_t::iterator, bool> r = _char2glyph.insert(make_pair(character, 0));

		if (r.second)
			r.first->second = get_glyph_index(character);
		return r.first->second;
	}

	const glyph *font::get_glyph(uint16_t index) const
	{
		glyphs_cache_t::const_iterator i = _glyphs.find(index);

		if (_glyphs.end() == i)
		{
			const glyph *g = load_glyph(index);

			i = _glyphs.insert(make_pair(index, g)).first;
		}
		return i->second;
	}
}
