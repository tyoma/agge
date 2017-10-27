#pragma once

#include "helpers.h"

#include <agge.text/font.h>
#include <agge.text/text_engine.h>

#include <map>
#include <string>
#include <tests/common/helpers.h>
#include <vector>

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			class font_accessor : public font::accessor
			{
			public:
				struct char_to_index;
				struct glyph;

			public:
				font_accessor();
				template <size_t indices_n, size_t glyphs_n>
				font_accessor(const font::metrics &metrics_, const char_to_index (&indices)[indices_n],
					glyph (&glyphs)[glyphs_n]);
				~font_accessor();

				void track(shared_ptr<size_t> allocated);

			public:
				mutable int glyph_mapping_calls;
				shared_ptr<size_t> glyphs_loaded;

			private:
				typedef std::map<wchar_t, uint16_t> indices_map_t;

			private:
				virtual font::metrics get_metrics() const;
				virtual uint16_t get_glyph_index(wchar_t character) const;
				virtual agge::glyph::outline_ptr load_glyph(uint16_t index, agge::glyph::glyph_metrics &m) const;

			private:
				font::metrics _metrics;
				indices_map_t _indices;
				std::vector<glyph> _glyphs;
				shared_ptr<size_t> _allocated;
			};

			class fonts_loader : public text_engine_base::loader
			{
			public:
				template <typename T, size_t n>
				explicit fonts_loader(T (&fonts)[n]);
				fonts_loader();

				size_t allocated_accessors() const;

			public:
				std::vector< std::pair<font::key, weak_ptr<font::accessor> > > created_log;
				std::map<font::key, font_accessor> fonts;
				shared_ptr<size_t> allocated;

			private:
				virtual font::accessor_ptr load(const wchar_t *typeface, int height, bool bold, bool italic,
					font::key::grid_fit grid_fit);
			};

			struct font_accessor::char_to_index
			{
				wchar_t symbol;
				uint16_t index;

				template <typename T1, typename T2>
				operator std::pair<T1, T2>() const
				{	return std::make_pair(symbol, index);	}
			};

			struct font_accessor::glyph
			{
				struct
				{
					double dx;
					double dy;
				} metrics;
				std::vector<agge::glyph::path_point> outline;
			};

			class rasterizer
			{
			public:
				typedef std::pair<const rasterizer * /*source*/, point<int> /*d*/> appended;

			public:
				rasterizer();

				void move_to(real_t x, real_t y);
				void line_to(real_t x, real_t y);
				void close_polygon();

				void sort();

				void append(const rasterizer &source, int dx, int dy);

			public:
				bool _sorted;
				std::vector<appended> append_log;
				std::vector<glyph::path_point> path;
			};



			template <size_t indices_n, size_t glyphs_n>
			inline font_accessor::font_accessor(const font::metrics &metrics_, const char_to_index (&indices)[indices_n],
					glyph (&glyphs)[glyphs_n])
				: glyph_mapping_calls(0), glyphs_loaded(new size_t(0)), _metrics(metrics_),
					_indices(indices, indices + indices_n), _glyphs(glyphs, glyphs + glyphs_n)/*,
					_allocated(allocated)*/
			{
				if (_allocated)
					++*_allocated;
			}


			template <typename T, size_t n>
			inline fonts_loader::fonts_loader(T (&fonts)[n])
				: fonts(fonts, fonts + n), allocated(new size_t())
			{	}


			template <typename T, size_t n>
			inline font_accessor::glyph glyph(double dx, double dy, T (&outline)[n])
			{
				font_accessor::glyph g = { dx, dy };

				for (size_t i = 0; i != n; ++i)
					g.outline.push_back(outline[i]);
				return g;
			}

			template <size_t indices_n, size_t glyphs_n>
			inline font::ptr create_font(const font::metrics &metrics_,
				const font_accessor::char_to_index (&indices)[indices_n], font_accessor::glyph (&glyphs)[glyphs_n])
			{	return font::ptr(new font(font::accessor_ptr(new font_accessor(metrics_, indices,glyphs))));	}
		}
	}

	bool operator <(const font::key &lhs, const font::key &rhs);
}
