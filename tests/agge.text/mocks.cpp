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

			agge::glyph::outline_ptr font_accessor::load_glyph(uint16_t index, agge::glyph::glyph_metrics &m) const
			{
				if (index >= _glyphs.size())
					return agge::glyph::outline_ptr();

				const font_accessor::glyph &myg = _glyphs[index];
				agge::glyph::outline_ptr o(new agge::glyph::outline_storage);

				m.advance_x = static_cast<real_t>(myg.metrics.dx), m.advance_y = static_cast<real_t>(myg.metrics.dy);
				for (vector<agge::glyph::path_point>::const_iterator i = myg.outline.begin(); i != myg.outline.end(); ++i)
					o->push_back(*i);
				return o;
			}


			font::accessor_ptr fonts_loader::load(const wchar_t *typeface, int height, bool bold, bool italic,
				font_engine::grid_fit grid_fit)
			{
				struct accessor : font::accessor
				{
					virtual font::metrics get_metrics() const
					{
						return font::metrics();
					}

					virtual uint16_t get_glyph_index(wchar_t /*character*/) const
					{
						return 0;
					}

					virtual agge::glyph::outline_ptr load_glyph(uint16_t /*index*/, glyph::glyph_metrics &/*m*/) const
					{
						return agge::glyph::outline_ptr();
					}
				};

				font_descriptor fd = { typeface, height, bold, italic, grid_fit };

				created_log.push_back(fd);
				return font::accessor_ptr((font::accessor*)new accessor);
			}
		}
	}
}
