#pragma once

#include "annotated_string.h"
#include "types.h"

#include <agge/color.h>

namespace agge
{
	struct font_style_annotation;
	struct style_modifier;

	typedef annotated_string<char, font_style_annotation> richtext_t;
	typedef annotated_string<char, style_modifier> richtext_modifier_t;


	struct font_style_annotation
	{
		font_descriptor basic;
		color foreground;
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
		color foreground;

		unsigned int use_family : 2,
			use_height : 3, // Only this action can accept scale_flag.
			use_weight : 2,
			use_italic : 2,
			use_hinting : 2,
			use_foreground : 2;

		style_modifier operator +(const style_modifier &rhs) const;

		static const style_modifier empty;
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
		static style_modifier foreground(color value);
		static style_modifier foreground_base();
	};



	template <typename T1, typename T2>
	inline annotated_string<T1, T2> &operator <<(annotated_string<T1, T2> &lhs, const char *rhs)
	{
		const char *e = rhs;

		while (*e)
			e++;
		lhs.append(rhs, e);
		return lhs;
	}

	richtext_t &operator <<(richtext_t &lhs, const style_modifier &rhs);
	richtext_t &operator <<(richtext_t &lhs, const richtext_modifier_t &rhs);

	richtext_modifier_t operator +(const char *lhs, const style_modifier &rhs);
	richtext_modifier_t operator +(const richtext_modifier_t &lhs, const char *rhs);
	richtext_modifier_t operator +(const richtext_modifier_t &lhs, const style_modifier &rhs);
}
