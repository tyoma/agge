#pragma once

#include "annotated_string.h"
#include "types.h"

namespace agge
{
	struct font_style_annotation;
	struct style_modifier;

	typedef annotated_string<char, font_style_annotation> richtext_t;
	typedef annotated_string<char, style_modifier> richtext_modifier_t;


	struct font_style_annotation
	{
		font_descriptor basic;
	};

	struct style_modifier
	{
		enum action {
			none = 0,	// Do nothing with this attribute.
			set = 1,		// Set to the corresponding value from this modifier.
			reset = 2,	// Reset to the value of the base style.

			scale = 4,	// Scales current height by the this->basic.height / 256. Uses base style if 'reset' is set.
		};

		font_descriptor basic;

		unsigned use_family : 2;
		unsigned use_height : 3; // Only this action can accept scale_flag.
		unsigned use_weight : 2;
		unsigned use_italic : 2;
		unsigned use_hinting : 2;

		style_modifier operator +(const style_modifier &rhs) const;
	};

	struct style
	{
		static style_modifier family(const std::string &value);
		static style_modifier family_base();
		static style_modifier height(int value);
		static style_modifier height_base();
		static style_modifier height_scale(double scale);
		static style_modifier height_scale_base(double scale);
		static style_modifier weight(font_weight value);
		static style_modifier weight_base();
		static style_modifier italic(bool value);
		static style_modifier italic_base();
		static style_modifier hinting(font_hinting value);
		static style_modifier hinting_base();
	};



	richtext_t &operator <<(richtext_t &lhs, const style_modifier &rhs);
	richtext_t &operator <<(richtext_t &lhs, const richtext_modifier_t &rhs);

	richtext_modifier_t operator +(const char *lhs, const style_modifier &rhs);
	richtext_modifier_t operator +(const richtext_modifier_t &lhs, const char *rhs);
	richtext_modifier_t operator +(const richtext_modifier_t &lhs, const style_modifier &rhs);
}
