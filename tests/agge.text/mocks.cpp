#include "mocks.h"

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			font_descriptor::font_descriptor(const wstring &typeface_, int height_, bool bold_, bool italic_,
					font_engine::grid_fit grid_fit_)
				: typeface(typeface_), height(height_), bold(bold_), italic(italic_), grid_fit(grid_fit_)
			{	}

			bool font_descriptor::operator <(const font_descriptor &rhs) const
			{
				return typeface < rhs.typeface ? true : rhs.typeface < typeface ? false :
					height < rhs.height ? true : rhs.height < height ? false :
					bold < rhs.bold ? true : rhs.bold < bold ? false :
					italic < rhs.italic ? true : rhs.italic < italic ? false :
					grid_fit < rhs.grid_fit;
			}

			bool font_descriptor::operator ==(const font_descriptor &rhs) const
			{	return !(*this < rhs) && !(rhs < *this);	}


			font_accessor::font_accessor()
			{	}

			font::metrics font_accessor::get_metrics() const
			{	return _metrics;	}

			uint16_t font_accessor::get_glyph_index(wchar_t character) const
			{
				indices_map_t::const_iterator i = _indices.find(character);

				++glyph_mapping_calls;
				return i != _indices.end() ? i->second : 0xffff;
			}

			agge::glyph::outline_ptr font_accessor::load_glyph(uint16_t index, agge::glyph::glyph_metrics &m) const
			{
				if (index >= _glyphs.size())
					return agge::glyph::outline_ptr();

				const font_accessor::glyph &myg = _glyphs[index];
				agge::glyph::outline_ptr o(new agge::glyph::outline_storage);

				++*glyphs_loaded;
				m.advance_x = static_cast<real_t>(myg.metrics.dx), m.advance_y = static_cast<real_t>(myg.metrics.dy);
				for (vector<agge::glyph::path_point>::const_iterator i = myg.outline.begin(); i != myg.outline.end(); ++i)
					o->push_back(*i);
				return o;
			}


			fonts_loader::fonts_loader()
			{	}

			font::accessor_ptr fonts_loader::load(const wchar_t *typeface, int height, bool bold, bool italic,
				font_engine::grid_fit grid_fit)
			{
				font_descriptor fd(typeface, height, bold, italic, grid_fit);

				created_log.push_back(fd);
				return font::accessor_ptr(new font_accessor(fonts[fd]));
			}
		}
	}
}
