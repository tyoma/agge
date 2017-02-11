#pragma once

#include <agge.text/font.h>
#include <map>
#include <vector>

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			class font : public agge::font
			{
			public:
				typedef std::shared_ptr<font> ptr;
				struct char_to_index;
				struct glyph;

			public:
				template <size_t indices_n, size_t glyphs_n>
				font(char_to_index (&indices)[indices_n], glyph (&glyphs)[glyphs_n]);

			private:
				typedef std::map<wchar_t, uint16_t> indices_map_t;

			private:
				virtual uint16_t get_glyph_index(wchar_t character) const;
				virtual const agge::glyph *load_glyph(uint16_t index) const;
				virtual pod_vector<kerning_pair> load_kerning() const;

			private:
				indices_map_t _indices;
				std::vector<glyph> _glyphs;
			};

			struct font::char_to_index
			{
				wchar_t symbol;
				uint16_t index;

				template <typename T1, typename T2>
				operator std::pair<T1, T2>() const
				{	return std::make_pair(symbol, index);	}
			};

			struct font::glyph
			{
				struct
				{
					double dx;
					double dy;
				} metrics;
			};



			template <size_t indices_n, size_t glyphs_n>
			inline font::font(char_to_index (&indices)[indices_n], glyph (&glyphs)[glyphs_n])
				: _indices(indices, indices + indices_n), _glyphs(glyphs, glyphs + glyphs_n)
			{	}
		}
	}
}
