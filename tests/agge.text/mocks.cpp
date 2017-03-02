#include "mocks.h"

using namespace std;

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

			const agge::glyph *font::load_glyph(uint16_t index) const
			{
				class mock_glyph : public agge::glyph
				{
				public:
					mock_glyph(size_t *glyphs_alive)
						: _glyphs_alive(glyphs_alive)
					{
						if (_glyphs_alive)
							++*_glyphs_alive;
					}

					virtual ~mock_glyph()
					{
						if (_glyphs_alive)
							--*_glyphs_alive;
					}

				private:
					size_t *_glyphs_alive;
				};

				agge::glyph *g = new mock_glyph(_glyphs_alive);
				const font::glyph &myg = _glyphs[index];

				g->advance_x = static_cast<real_t>(myg.metrics.dx);
				g->advance_y = static_cast<real_t>(myg.metrics.dy);
				g->index = index;
				g->outline.reset(new agge::glyph::outline_storage);
				for (vector<agge::glyph::path_point>::const_iterator i = myg.outline.begin(); i != myg.outline.end(); ++i)
					g->outline->push_back(*i);
				return g;
			}

			pod_vector<font::kerning_pair> font::load_kerning() const
			{
				throw 0;
			}
		}
	}
}
