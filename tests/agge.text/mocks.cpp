#include "mocks.h"

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			font::metrics font_accessor::get_metrics() const
			{	return _metrics;	}

			uint16_t font_accessor::get_glyph_index(wchar_t character) const
			{
				indices_map_t::const_iterator i = _indices.find(character);

				++glyph_mapping_calls;
				return i != _indices.end() ? i->second : 0xffff;
			}

			bool font_accessor::load_glyph(uint16_t index, agge::glyph::glyph_metrics &m, agge::glyph::outline_storage &o) const
			{
				if (index >= _glyphs.size())
					return false;

				const font_accessor::glyph &myg = _glyphs[index];

				m.advance_x = static_cast<real_t>(myg.metrics.dx), m.advance_y = static_cast<real_t>(myg.metrics.dy);
				for (vector<agge::glyph::path_point>::const_iterator i = myg.outline.begin(); i != myg.outline.end(); ++i)
					o.push_back(*i);
				return true;
			}


			font::accessor_ptr fonts_loader::load(const wchar_t * /*typeface*/, int /*height*/, bool /*bold*/, bool /*italic*/)
			{
				throw 0;
			}
		}
	}
}
