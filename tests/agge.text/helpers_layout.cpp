#include "helpers_layout.h"

#include "helpers.h"

using namespace std;

namespace agge
{
	namespace tests
	{
		bool ref_glyph_run::operator ==(const glyph_run &rhs) const
		{
			if (_font == rhs.font_ && _offset == rhs.offset)
			{
				vector<positioned_glyph>::const_iterator i = _glyphs.begin();
				positioned_glyphs_container_t::const_iterator j = rhs.begin();

				for (; i != _glyphs.end() && j != rhs.end(); ++i, ++j)
					if (i->index != j->index || (_check_glyph_advances && !(i->d == j->d)))
						return false;
				return true;
			}
			return false;
		}


		ref_text_line::ref_text_line(real_t offset_x, real_t offset_y, real_t width, const vector<ref_glyph_run> &glyph_runs)
			: _offset(create_vector(offset_x, offset_y)), _width(width), _glyph_runs(glyph_runs)
		{	}

		bool ref_text_line::operator ==(const text_line &rhs) const
		{
			if (_offset == rhs.offset && (!_width || tests::equal(_width, rhs.width)))
			{
				vector<ref_glyph_run>::const_iterator i = _glyph_runs.begin();
				glyph_runs_container_t::const_iterator j = rhs.begin();

				for (; i != _glyph_runs.end() && j != rhs.end(); ++i, ++j)
					if (!(*i == *j))
						return false;
				return true;
			}
			return false;
		}


		ref_text_line_offsets::ref_text_line_offsets(real_t offset_x, real_t offset_y)
			: _offset(create_vector(offset_x, offset_y))
		{	}

		bool ref_text_line_offsets::operator ==(const text_line &rhs) const
		{	return _offset == rhs.offset;	}


		richtext_t simple_richtext(const wstring &text, const string &family, int height, font_weight weight, bool italic,
			font_hinting hinting)
		{
			font_style_annotation a = {	font_descriptor::create(family, height, weight, italic, hinting),	};
			return richtext_t(text.c_str(), a);
		}
	}
}
