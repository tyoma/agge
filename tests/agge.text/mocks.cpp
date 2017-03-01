#include "mocks.h"

#include <agge.text/glyph.h>

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			class glyph : public agge::glyph
			{
			public:
				glyph(size_t *glyphs_alive);
				virtual ~glyph();

			private:
				size_t *_glyphs_alive;
			};



			glyph::glyph(size_t *glyphs_alive)
				: _glyphs_alive(glyphs_alive)
			{
				if (_glyphs_alive)
					++*_glyphs_alive;
			}

			glyph::~glyph()
			{
				if (_glyphs_alive)
					--*_glyphs_alive;
			}

			uint16_t font::get_glyph_index(wchar_t character) const
			{
				indices_map_t::const_iterator i = _indices.find(character);

				return i != _indices.end() ? i->second : 0xffff;
			}

			const agge::glyph *font::load_glyph(uint16_t index) const
			{
				mocks::glyph *g = new mocks::glyph(_glyphs_alive);

				g->advance_x = static_cast<real_t>(_glyphs[index].metrics.dx);
				g->advance_y = static_cast<real_t>(_glyphs[index].metrics.dy);
				g->index = index;
				return g;
			}

			pod_vector<font::kerning_pair> font::load_kerning() const
			{
				throw 0;
			}
		}
	}
}
