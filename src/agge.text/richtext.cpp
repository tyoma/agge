#include <agge.text/richtext.h>

#include <agge/tools.h>

using namespace std;

namespace agge
{
	namespace
	{
		template <typename TargetT>
		void apply(TargetT &target, const style_modifier &modifier)
		{
			if (modifier.use_family)
				target.basic.family = modifier.basic.family;
			if (modifier.use_height)
				target.basic.height = modifier.basic.height;
			if (modifier.use_weight)
				target.basic.bold = modifier.basic.bold;
			if (modifier.use_italic)
				target.basic.italic = modifier.basic.italic;
			if (modifier.use_hinting)
				target.basic.hinting = modifier.basic.hinting;
		}

		void apply_usings(style_modifier &target, const style_modifier &modifier)
		{
			target.use_family |= modifier.use_family;
			target.use_height |= modifier.use_height;
			target.use_weight |= modifier.use_weight;
			target.use_italic |= modifier.use_italic;
			target.use_hinting |= modifier.use_hinting;
		}
	}

	style_modifier style_modifier::operator +(const style_modifier &rhs) const
	{
		style_modifier a = *this;

		return apply(a, rhs), apply_usings(a, rhs), a;
	}


	style_modifier style::family(const std::string &value)
	{	style_modifier m = {}; return m.basic.family = value, m.use_family = true, m;	}

	style_modifier style::height(int value)
	{	style_modifier m = {}; return m.basic.height = value, m.use_height = true, m;	}

	style_modifier style::weight(font_weight value)
	{	style_modifier m = {}; return m.basic.bold = value >= bold, m.use_weight = true, m;	}

	style_modifier style::italic(bool value)
	{	style_modifier m = {}; return m.basic.italic = !!value, m.use_italic = true, m;	}

	style_modifier style::hinting(font_hinting value)
	{	style_modifier m = {}; return m.basic.hinting = value, m.use_hinting = true, m;	}


	richtext_t &operator <<(richtext_t &lhs, const style_modifier &rhs)
	{
		font_style_annotation a = lhs.current_annotation();

		return apply(a, rhs), lhs.annotate(a), lhs;
	}

	richtext_t &operator <<(richtext_t &lhs, const richtext_modifier_t &rhs)
	{
		font_style_annotation a = lhs.current_annotation();

		for (richtext_modifier_t::const_iterator i = rhs.ranges_begin(); i != rhs.ranges_end(); ++i)
		{
			apply(a, i->get_annotation());
			lhs.annotate(a);
			lhs += wstring(i->begin(), i->end());
		}
		return lhs;
	}

	richtext_modifier_t operator +(const wchar_t *lhs, const style_modifier &rhs)
	{
		richtext_modifier_t s(L"", zero());

		return s += lhs, s.annotate(rhs), s;
	}

	richtext_modifier_t operator +(const richtext_modifier_t &lhs, const wchar_t *rhs)
	{
		richtext_modifier_t s(lhs);

		return s += rhs, s;
	}

	richtext_modifier_t operator +(const richtext_modifier_t &lhs, const style_modifier &rhs)
	{
		richtext_modifier_t s(lhs);

		return s.annotate(rhs), s;
	}
}
