#include <agge.text/font.h>

#include <cctype>

using namespace std;

namespace agge
{
	namespace
	{
		struct nc_compare
		{
			bool operator ()(wchar_t lhs, wchar_t rhs) const
			{	return toupper(lhs) == toupper(rhs);	}
		};
	}

	font::font(const key &key_, const accessor_ptr &accessor_, real_t factor)
		: _accessor(accessor_), _key(key_), _metrics(accessor_->get_metrics()), _factor(factor)
	{
		_metrics.ascent *= _factor;
		_metrics.descent *= _factor;
		_metrics.leading *= _factor;
	}
	
	font::key font::get_key() const
	{	return _key;	}

	font::metrics font::get_metrics() const
	{	return _metrics;	}

	glyph_index_t font::map_single(wchar_t character) const
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

	const glyph *font::get_glyph(glyph_index_t index) const
	{
		glyphs_cache_t::iterator i = _glyphs.find(index);

		if (_glyphs.end() == i)
		{
			glyphs_cache_t::iterator inserted;
			glyph g;

			g.factor = _factor;
			g.outline = _accessor->load_glyph(index, g.metrics);
			g.metrics.advance_x *= _factor;
			g.metrics.advance_y *= _factor;
			_glyphs.insert(index, g, i);
		}
		return i->second.outline ? &i->second : 0;
	}


	font::key::key(const wstring &typeface_, int height_, bool bold_, bool italic_, grid_fit grid_fit__)
		: typeface(typeface_), height(height_), bold(bold_), italic(italic_), grid_fit_(grid_fit__)
	{	}

	bool operator ==(const font::key &lhs, const font::key &rhs)
	{
		return lhs.typeface.size() == rhs.typeface.size()
			&& lhs.height == rhs.height
			&& lhs.bold == rhs.bold
			&& lhs.italic == rhs.italic
			&& lhs.grid_fit_ == rhs.grid_fit_
			&& equal(lhs.typeface.begin(), lhs.typeface.end(), rhs.typeface.begin(), nc_compare());
	}
}
