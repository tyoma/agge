#pragma once

#include <agge/tools.h>
#include <agge/types.h>
#include <agge.text/layout_primitives.h>
#include <agge.text/richtext.h>
#include <agge.text/shared_ptr.h>
#include <string>
#include <vector>

namespace agge
{
	class font;
	struct glyph_run;

	namespace tests
	{
		struct modify_family
		{
			modify_family(const std::string &family_)
				: family(family_)
			{	}

			void operator ()(richtext_t &target) const
			{
				font_style_annotation a = target.current_annotation();
				a.basic.family = family;
				target.annotate(a);
			}

			std::string family;
		};

		struct modify_height
		{
			modify_height(int height_)
				: height(height_)
			{	}

			void operator ()(richtext_t &target) const
			{
				font_style_annotation a = target.current_annotation();
				a.basic.height = height;
				target.annotate(a);
			}

			int height;
		};


		class ref_glyph_run
		{
		public:
			template <typename T>
			ref_glyph_run(shared_ptr<font> font_, real_t offset_x, real_t offset_y, const std::vector<T> &indices);
			template <size_t n>
			ref_glyph_run(shared_ptr<font> font_, real_t offset_x, real_t offset_y,
					positioned_glyph (&positioned)[n]);

			bool operator ==(const glyph_run &rhs) const;

		private:
			shared_ptr<font> _font;
			vector_r _offset;
			std::vector<positioned_glyph> _glyphs;
			bool _check_glyph_advances;
		};

		class ref_text_line
		{
		public:
			ref_text_line(real_t offset_x, real_t offset_y, real_t width, const std::vector<ref_glyph_run> &glyph_runs);

			bool operator ==(const text_line &rhs) const;

		private:
			vector_r _offset;
			real_t _width;
			std::vector<ref_glyph_run> _glyph_runs;
		};

		class ref_text_line_offsets
		{
		public:
			ref_text_line_offsets(real_t offset_x, real_t offset_y);

			bool operator ==(const text_line &rhs) const;

		private:
			vector_r _offset;
		};



		template <typename T>
		inline ref_glyph_run::ref_glyph_run(shared_ptr<font> font_, real_t offset_x, real_t offset_y,
				const std::vector<T> &indices)
			: _font(font_), _offset(create_vector(offset_x, offset_y)), _check_glyph_advances(false)
		{
			for (typename std::vector<T>::const_iterator i = indices.begin(); i != indices.end(); ++i)
			{
				positioned_glyph g = {	create_vector(0.0f, 0.0f), static_cast<glyph_index_t>(*i)	};
				_glyphs.push_back(g);
			}
		}

		template <size_t n>
		inline ref_glyph_run::ref_glyph_run(shared_ptr<font> font_, real_t offset_x, real_t offset_y,
				positioned_glyph (&positioned)[n])
			: _font(font_), _offset(create_vector(offset_x, offset_y)), _glyphs(mkvector(begin(positioned),
				end(positioned))), _check_glyph_advances(true)
		{	}


		richtext_t simple_richtext(const std::wstring &text, const std::string &family, int height, bool bold = false,
			bool italic = false, font_hinting hinting = hint_strong);

		template <typename StyleModifier>
		inline richtext_t &operator <<(richtext_t &lhs, const StyleModifier &rhs)
		{	return rhs(lhs), lhs;	}

		inline richtext_t &operator <<(richtext_t &lhs, const wchar_t *rhs)
		{	return lhs += std::wstring(rhs), lhs;	}
	}
}
