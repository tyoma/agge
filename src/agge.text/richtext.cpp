#include <agge.text/richtext.h>

using namespace std;

#define apply_single(target, modifier, base, action)\
	if (agge::style_modifier::set == action)\
		target = modifier;\
	else if (agge::style_modifier::reset == action)\
		target = base;

namespace agge
{
	namespace
	{
		void apply(font_style_annotation &target, const style_modifier &modifier, const font_style_annotation &base)
		{
			apply_single(target.basic.family, modifier.basic.family, base.basic.family, modifier.use_family);
			switch (modifier.use_height)
			{
			case style_modifier::set:
				target.basic.height = modifier.basic.height;
				break;

			case style_modifier::reset:
				target.basic.height = base.basic.height;
				break;

			case style_modifier::scale:
				target.basic.height = modifier.basic.height * target.basic.height / 256;
				break;

			case style_modifier::scale | style_modifier::reset:
				target.basic.height = modifier.basic.height * base.basic.height / 256;
				break;
			}
			apply_single(target.basic.weight, modifier.basic.weight, base.basic.weight, modifier.use_weight);
			apply_single(target.basic.italic, modifier.basic.italic, base.basic.italic, modifier.use_italic);
			apply_single(target.basic.hinting, modifier.basic.hinting, base.basic.hinting, modifier.use_hinting);
			apply_single(target.foreground, modifier.foreground, base.foreground, modifier.use_foreground);
		}

		void apply(style_modifier &target, const style_modifier &modifier)
		{
			if (modifier.use_family)
				target.basic.family = modifier.basic.family, target.use_family = modifier.use_family;
			if (modifier.use_height)
				target.basic.height = modifier.basic.height, target.use_height = modifier.use_height;
			if (modifier.use_weight)
				target.basic.weight = modifier.basic.weight, target.use_weight = modifier.use_weight;
			if (modifier.use_italic)
				target.basic.italic = modifier.basic.italic, target.use_italic = modifier.use_italic;
			if (modifier.use_hinting)
				target.basic.hinting = modifier.basic.hinting, target.use_hinting = modifier.use_hinting;
			if (modifier.use_foreground)
				target.foreground = modifier.foreground, target.use_foreground = modifier.use_foreground;
		}
	}

	const style_modifier style_modifier::empty = {};

	style_modifier style_modifier::operator +(const style_modifier &rhs) const
	{
		style_modifier a = *this;

		return apply(a, rhs), a;
	}


	style_modifier style::family(const std::string &value)
	{	style_modifier m = {}; return m.basic.family = value, m.use_family = style_modifier::set, m;	}

	style_modifier style::family_base()
	{	style_modifier m = {}; return m.use_family = style_modifier::reset, m;	}

	style_modifier style::height(int value)
	{	style_modifier m = {}; return m.basic.height = value, m.use_height = style_modifier::set, m;	}

	style_modifier style::height_base()
	{	style_modifier m = {}; return m.use_height = style_modifier::reset, m;	}

	style_modifier style::height_scale(double scale)
	{	style_modifier m = {}; return m.basic.height = static_cast<int>(256 * scale), m.use_height = style_modifier::scale, m;	}

	style_modifier style::height_scale_base(double scale)
	{	style_modifier m = {}; return m.basic.height = static_cast<int>(256 * scale), m.use_height = style_modifier::scale | style_modifier::reset, m;	}

	style_modifier style::weight(font_weight value)
	{	style_modifier m = {}; return m.basic.weight = value, m.use_weight = style_modifier::set, m;	}

	style_modifier style::weight_base()
	{	style_modifier m = {}; return m.use_weight = style_modifier::reset, m;	}

	style_modifier style::italic(bool value)
	{	style_modifier m = {}; return m.basic.italic = !!value, m.use_italic = style_modifier::set, m;	}

	style_modifier style::italic_base()
	{	style_modifier m = {}; return m.use_italic = style_modifier::reset, m;	}

	style_modifier style::hinting(font_hinting value)
	{	style_modifier m = {}; return m.basic.hinting = value, m.use_hinting = style_modifier::set, m;	}

	style_modifier style::hinting_base()
	{	style_modifier m = {}; return m.use_hinting = style_modifier::reset, m;	}

	style_modifier style::foreground(color value)
	{	style_modifier m = {}; return m.foreground = value, m.use_foreground = style_modifier::set, m;	}

	style_modifier style::foreground_base()
	{	style_modifier m = {}; return m.use_foreground = style_modifier::reset, m;	}


	richtext_t &operator <<(richtext_t &lhs, const style_modifier &rhs)
	{
		font_style_annotation a = lhs.current_annotation();

		apply(a, rhs, lhs.base_annotation());
		lhs.annotate(a);
		return lhs;
	}

	richtext_t &operator <<(richtext_t &lhs, const richtext_modifier_t &rhs)
	{
		font_style_annotation a = lhs.current_annotation();

		for (richtext_modifier_t::const_iterator i = rhs.ranges_begin(); i != rhs.ranges_end(); ++i)
		{
			apply(a, i->get_annotation(), lhs.base_annotation());
			lhs.annotate(a);
			lhs.append(i->begin(), i->end());
		}
		return lhs;
	}

	richtext_modifier_t operator +(const char *lhs, const style_modifier &rhs)
	{
		richtext_modifier_t s = style_modifier::empty;

		return s << lhs, s.annotate(rhs), s;
	}

	richtext_modifier_t operator +(const richtext_modifier_t &lhs, const char *rhs)
	{
		richtext_modifier_t s(lhs);

		return s << rhs, s;
	}

	richtext_modifier_t operator +(const richtext_modifier_t &lhs, const style_modifier &rhs)
	{
		richtext_modifier_t s(lhs);

		return s.annotate(rhs), s;
	}
}
