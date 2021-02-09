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

				text.append("Z");

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
				richtext_modifier_t mtext("", zero());

				a2.basic.height = 5;
				a2.basic.weight = bold;
				a3 = a2;
				a3.basic.family = "Segoe";
				a3.basic.height = 10;
				a3.basic.italic = false;

				// INIT / ACT
				mtext.append("Z");
				mtext.annotate(style::height(5) + style::weight(bold));
				mtext.append("ebra");
				mtext.annotate(style::height(10) + style::italic(false) + style::family("Segoe"));
				mtext.append("fish");

				// ACT
				text << mtext;

				// ASSERT
				assert_equal((plural
					+ pair<string, font_style_annotation>("Z", a)
					+ pair<string, font_style_annotation>("ebra", a2)
					+ pair<string, font_style_annotation>("fish", a3)),
					mkvector(text.ranges_begin(), text.ranges_end()));
			}


			test( StyleModifierStringCanBeCreatedInPlace )
			{
				// ACT / ASSERT
				assert_equal((plural
					+ pair<string, style_modifier>("That's a ", zero())
					+ pair<string, style_modifier>("bold", style::weight(bold))
					+ pair<string, style_modifier>(" statement!", style::weight(regular))),
					mkvector("That's a " + style::weight(bold) + "bold" + style::weight(regular) + " statement!"));

				assert_equal((plural
					+ pair<string, style_modifier>("different ", zero())
					+ pair<string, style_modifier>("font", style::family("Segoe"))),
					mkvector("different " + style::family("Segoe") + "font"));
			}


			test( StyleAtributesCanBeResetToBase )
			{
				// INIT
				font_style_annotation a1 = {	font_descriptor::create("Arial", 15, regular, true, hint_none),	};
				richtext_t text(a1);

				text.append("foobar");
				text << style::family("Tahoma") + style::height(20) + style::weight(bold) + style::italic(false)
					+ style::hinting(hint_strong);

				// ACT / ASSERT
				text << style::family_base();
				assert_equal("Arial", text.current_annotation().basic.family);
				text << style::height_base();
				assert_equal(15, text.current_annotation().basic.height);
				text << style::weight_base();
				assert_equal(regular, text.current_annotation().basic.weight);
				text << style::italic_base();
				assert_is_true(text.current_annotation().basic.italic);
				text << style::hinting_base();
				assert_equal(hint_none, text.current_annotation().basic.hinting);

				// INIT
				font_style_annotation a2 = {	font_descriptor::create("Segoe", 16, light, false, hint_vertical),	};

				text.set_base_annotation(a2);
				text.append("foobar");
				text << style::family("Tahoma") + style::height(20) + style::weight(bold) + style::italic(true)
					+ style::hinting(hint_strong);

				// ACT / ASSERT
				text << style::family_base();
				assert_equal("Segoe", text.current_annotation().basic.family);
				text << style::height_base();
				assert_equal(16, text.current_annotation().basic.height);
				text << style::weight_base();
				assert_equal(light, text.current_annotation().basic.weight);
				text << style::italic_base();
				assert_is_false(text.current_annotation().basic.italic);
				text << style::hinting_base();
				assert_equal(hint_vertical, text.current_annotation().basic.hinting);
			}


			test( StyleAtributesCanScaleCurrentFont )
			{
				// INIT
				const font_style_annotation a1 = {	font_descriptor::create("Arial", 15, regular, true, hint_none),	};
				richtext_t text(a1);

				// ACT / ASSERT
				text << style::height_scale(0.84);

				// ASSERT
				assert_equal(12, text.current_annotation().basic.height);

				// ACT / ASSERT
				text << style::height_scale(1.44);

				// ASSERT
				assert_equal(17, text.current_annotation().basic.height);
			}


			test( StyleAtributesCanScaleBaseFont )
			{
				// INIT
				const font_style_annotation a1 = {	font_descriptor::create("Arial", 21, regular, true, hint_none),	};
				richtext_t text(a1);

				text.append("foobar");
				text << style::height(10);

				// ACT / ASSERT
				text << style::height_scale_base(0.84);

				// ASSERT
				assert_equal(17, text.current_annotation().basic.height);

				// ACT / ASSERT
				text << style::height_scale_base(1.44);

				// ASSERT
				assert_equal(30, text.current_annotation().basic.height);
			}
		end_test_suite
	}
}
