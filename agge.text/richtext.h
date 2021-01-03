#pragma once

#include "annotated_string.h"
#include "types.h"

namespace agge
{
	struct font_style_annotation;
	struct style_modifier;

	typedef annotated_string<wchar_t, font_style_annotation> richtext_t;
	typedef annotated_string<wchar_t, style_modifier> richtext_modifier_t;


	struct font_style_annotation
	{
		font_descriptor basic;
	};

	struct style_modifier
	{
		font_descriptor basic;

		unsigned use_family : 1;
		unsigned use_height : 1;
		unsigned use_weight : 1;
		unsigned use_italic : 1;
		unsigned use_hinting : 1;

		style_modifier operator +(const style_modifier &rhs) const;
	};

	struct style
	{
		static style_modifier family(const std::string &value);
		static style_modifier height(int value);
		static style_modifier weight(font_weight value);
		static style_modifier italic(bool value);
		static style_modifier hinting(font_hinting value);
	};



	richtext_t &operator <<(richtext_t &lhs, const style_modifier &rhs);
	richtext_t &operator <<(richtext_t &lhs, const richtext_modifier_t &rhs);

	richtext_modifier_t operator +(const wchar_t *lhs, const style_modifier &rhs);
	richtext_modifier_t operator +(const richtext_modifier_t &lhs, const wchar_t *rhs);
	richtext_modifier_t operator +(const richtext_modifier_t &lhs, const style_modifier &rhs);
}
