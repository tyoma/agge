#include "mocks.h"

#include <agge.text/glyph.h>

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			uint16_t font::get_glyph_index(wchar_t character) const
			{
				indices_map_t::const_iterator i = _indices.find(character);

				return i != _indices.end() ? i->second : 0xffff;
			}

			const glyph *font::load_glyph(uint16_t index) const
			{
				agge::glyph *g = new agge::glyph;

				g->advance_x = static_cast<real_t>(_glyphs[index].metrics.dx);
				g->advance_y = static_cast<real_t>(_glyphs[index].metrics.dy);
				return g;
			}

			pod_vector<font::kerning_pair> font::load_kerning() const
			{
				throw 0;
			}
		}
	}
}
