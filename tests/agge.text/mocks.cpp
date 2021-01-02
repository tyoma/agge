#include "mocks.h"

#include <ut/assert.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			logging_text_engine::logging_text_engine(loader &loader_, unsigned collection_cycles)
				: text_engine_base(loader_, collection_cycles)
			{	}

			void logging_text_engine::on_before_removed(font *font_) throw()
			{	deletion_log.push_back(font_);	}


			font_accessor::font_accessor()
			{	}

			font_accessor::~font_accessor()
			{
				if (_allocated)
					--*_allocated;
			}

			void font_accessor::track(shared_ptr<size_t> allocated)
			{
				_allocated = allocated;
				if (_allocated)
					++*_allocated;
			}

			font_descriptor font_accessor::get_descriptor() const
			{	return descriptor;	}

			font_metrics font_accessor::get_metrics() const
			{	return _metrics;	}

			glyph_index_t font_accessor::get_glyph_index(wchar_t character) const
			{
				indices_map_t::const_iterator i = _indices.find(character);

				++glyph_mapping_calls;
				return i != _indices.end() ? i->second : 0xffff;
			}

			agge::glyph::outline_ptr font_accessor::load_glyph(glyph_index_t index, agge::glyph::glyph_metrics &m) const
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
				: allocated(new size_t())
			{	}

			font::accessor_ptr fonts_loader::load(const font_descriptor &descriptor)
			{
				shared_ptr<font_accessor> a(new font_accessor(fonts[descriptor]));

				if (a->descriptor.family.empty())
					a->descriptor = descriptor;
				created_log.push_back(make_pair(descriptor, a));
				a->track(allocated);
				return a;
			}


			rasterizer::rasterizer()
				: _sorted(false)
			{	}

			void rasterizer::move_to(real_t x, real_t y)
			{	path.push_back(mkppoint(path_command_move_to, x, y));	}

			void rasterizer::line_to(real_t x, real_t y)
			{	path.push_back(mkppoint(path_command_line_to, x, y));	}

			void rasterizer::close_polygon()
			{	path.back().command |= path_flag_close;	}

			void rasterizer::compact()
			{	_sorted = true;	}

			void rasterizer::append(const rasterizer &source, int dx, int dy)
			{
				assert_is_true(source._sorted);
				append_log.push_back(make_pair(&source, mkpoint(dx, dy)));
			}
		}
	}
}
