#include <agge.text/richtext.h>

#include "helpers.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	template <typename T, typename AnnotationT>
	inline bool operator ==(const pair<basic_string<T>, AnnotationT> &expected,
		const typename annotated_string<T, AnnotationT>::range &actual)
	{
		return expected.first == basic_string<T>(actual.begin(), actual.end())
			&& expected.second == actual.get_annotation();
	}

	inline bool operator ==(const font_style_annotation &lhs, const font_style_annotation &rhs)
	{	return lhs.basic == rhs.basic;	}

	inline bool operator ==(const style_modifier &lhs, const style_modifier &rhs)
	{
		return lhs.basic == rhs.basic && lhs.use_family == rhs.use_family && lhs.use_height == rhs.use_height
			&& lhs.use_weight == rhs.use_weight && lhs.use_italic == rhs.use_italic && lhs.use_hinting == rhs.use_hinting;
	}

	namespace tests
	{
		namespace
		{
			vector<richtext_modifier_t::range> mkvector(const richtext_modifier_t &from)
			{	return vector<richtext_modifier_t::range>(from.ranges_begin(), from.ranges_end());	}
		}

		begin_test_suite( RichTextTests )
			test( StyleModifierApplicationSetsExpectedProperties )
			{
				// INIT
				font_style_annotation a = {	font_descriptor::create("Arial", 15, regular, true, hint_none),	};
				richtext_t text(a);

				text += L"Z";

				// ACT
				text << style::family("Verdana");

				// ASSERT
				assert_equal(font_descriptor::create("Verdana", 15, regular, true, hint_none), text.current_annotation().basic);

				// ACT
				text << style::family("Tahoma");

				// ASSERT
				assert_equal(font_descriptor::create("Tahoma", 15, regular, true, hint_none), text.current_annotation().basic);

				// ACT
				text << style::height(10);

				// ASSERT
				assert_equal(font_descriptor::create("Tahoma", 10, regular, true, hint_none), text.current_annotation().basic);

				// ACT
				text << style::height(20);

				// ASSERT
				assert_equal(font_descriptor::create("Tahoma", 20, regular, true, hint_none), text.current_annotation().basic);

				// ACT
				text << style::weight(bold);

				// ASSERT
				assert_equal(font_descriptor::create("Tahoma", 20, bold, true, hint_none), text.current_annotation().basic);

				// ACT
				text << style::weight(regular);

				// ASSERT
				assert_equal(font_descriptor::create("Tahoma", 20, regular, true, hint_none), text.current_annotation().basic);

				// ACT
				text << style::italic(false);

				// ASSERT
				assert_equal(font_descriptor::create("Tahoma", 20, regular, false, hint_none), text.current_annotation().basic);

				// ACT
				text << style::italic(true);

				// ASSERT
				assert_equal(font_descriptor::create("Tahoma", 20, regular, true, hint_none), text.current_annotation().basic);

				// ACT
				text << style::hinting(hint_vertical);

				// ASSERT
				assert_equal(font_descriptor::create("Tahoma", 20, regular, true, hint_vertical), text.current_annotation().basic);

				// ACT
				text << style::hinting(hint_strong) << style::family("arial");

				// ASSERT
				assert_equal(font_descriptor::create("arial", 20, regular, true, hint_strong), text.current_annotation().basic);
			}


			test( StyleModifyingStringCanBeAppendedToRichText )
			{
				// INIT
				font_style_annotation a = {	font_descriptor::create("Arial", 15, regular, true, hint_none),	}, a2 = a,
					a3 = a;
				richtext_t text(a);
				richtext_modifier_t mtext(L"", zero());

				a2.basic.height = 5;
				a2.basic.weight = bold;
				a3 = a2;
				a3.basic.family = "Segoe";
				a3.basic.height = 10;
				a3.basic.italic = false;

				// INIT / ACT
				mtext += L"Z";
				mtext.annotate(style::height(5) + style::weight(bold));
				mtext += L"ebra";
				mtext.annotate(style::height(10) + style::italic(false) + style::family("Segoe"));
				mtext += L"fish";

				// ACT
				text << mtext;

				// ASSERT
				assert_equal((plural
					+ pair<wstring, font_style_annotation>(L"Z", a)
					+ pair<wstring, font_style_annotation>(L"ebra", a2)
					+ pair<wstring, font_style_annotation>(L"fish", a3)),
					mkvector(text.ranges_begin(), text.ranges_end()));
			}


			test( StyleModifierStringCanBeCreatedInPlace )
			{
				// ACT / ASSERT
				assert_equal((plural
					+ pair<wstring, style_modifier>(L"That's a ", zero())
					+ pair<wstring, style_modifier>(L"bold", style::weight(bold))
					+ pair<wstring, style_modifier>(L" statement!", style::weight(regular))),
					mkvector(L"That's a " + style::weight(bold) + L"bold" + style::weight(regular) + L" statement!"));

				assert_equal((plural
					+ pair<wstring, style_modifier>(L"different ", zero())
					+ pair<wstring, style_modifier>(L"font", style::family("Segoe"))),
					mkvector(L"different " + style::family("Segoe") + L"font"));
			}
		end_test_suite
	}
}
