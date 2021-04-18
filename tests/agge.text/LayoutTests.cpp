#include <agge.text/layout.h>

#include "helpers.h"
#include "helpers_layout.h"
#include "mocks.h"

#include <agge.text/limit.h>
#include <iterator>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			font_metrics c_fm1 = { 10.0f, 2.0f, 2.0f };
			font_metrics c_fm2 = { 14.0f, 3.0f, 1.0f };
		}

		begin_test_suite( LayoutTests )
			struct font_factory : public agge::font_factory
			{
				virtual shared_ptr<font> create_font(const font_descriptor &/*descriptor*/)
				{	return font_;	}

				shared_ptr<font> operator *() const
				{	return font_;	}

				shared_ptr<font> font_;
			};

			typedef shared_ptr<font_factory> factory_ptr;

			template <size_t indices_n, size_t glyphs_n>
			factory_ptr create_single_font_factory(const font_metrics &metrics_,
				const mocks::font_accessor::char_to_index (&indices)[indices_n],
				mocks::font_accessor::glyph (&glyphs)[glyphs_n])
			{
				factory_ptr f(new font_factory);

				f->font_ = mocks::create_font(metrics_, indices, glyphs);
				return f;
			}

			test( EmptyLayoutHasEmptyBox )
			{
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { { { 0, 0 } }, };
				factory_ptr f = create_single_font_factory(c_fm1, indices, glyphs);

				// INIT / ACT
				layout l;

				l.process(richtext_t(font_style_annotation()), limit::none(), *f);

				// ACT
				box_r box = l.get_box();

				// ASSERT
				assert_equal(0.0f, box.w);
				assert_equal(0.0f, box.h);
			}


			test( SingleLineUnboundLayoutBoxEqualsSumOfAdvances )
			{
				// INIT
				mocks::font_accessor::char_to_index indices1[] = { { L'A', 1 }, { L'B', 0 }, { L'Q', 0 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 13, 0 } },
					{ { 11, 0 } },
				};
				factory_ptr f1 = create_single_font_factory(c_fm1, indices1, glyphs);

				// INIT / ACT
				layout l1;
				layout l2;
				layout l3;

				l1.process(R("A"), limit::none(), *f1);
				l2.process(R("AAB"), limit::none(), *f1);
				l3.process(R("BQA"), limit::none(), *f1);

				// ACT
				box_r box1 = l1.get_box();
				box_r box2 = l2.get_box();
				box_r box3 = l3.get_box();

				// ASSERT
				assert_equal(11.0f, box1.w);
				assert_equal(35.0f, box2.w);
				assert_equal(37.0f, box3.w);

				// INIT
				mocks::font_accessor::char_to_index indices2[] = { { L'A', 0 }, { L'B', 0 }, { L'Q', 0 }, };
				factory_ptr f2 = create_single_font_factory(c_fm1, indices2, glyphs);

				// INIT / ACT
				layout l4;
				layout l5;

				l4.process(R("A"), limit::none(), *f2);
				l5.process(R("ABQABQABQ"), limit::none(), *f2);

				// ACT
				box_r box4 = l4.get_box();
				box_r box5 = l5.get_box();

				// ASSERT
				assert_equal(13.0f, box4.w);
				assert_equal(117.0f, box5.w);
			}


			test( SingleLineUnboundLayoutProducesSingleGlyphRuns )
			{
				// INIT
				mocks::font_accessor::char_to_index indices1[] = { { L'A', 1 }, { L'B', 0 }, { L'Q', 0 }, };
				mocks::font_accessor::char_to_index indices2[] = { { L'A', 0 }, { L'B', 1 }, { L'Q', 2 }, { L' ', 3 } };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 13, 0 } },
					{ { 11, 0 } },
					{ { 12.7, 0 } },
					{ { 10.1, 0 } },
				};
				factory_ptr f1 = create_single_font_factory(c_fm1, indices1, glyphs);
				factory_ptr f2 = create_single_font_factory(c_fm2, indices2, glyphs);
				layout::const_iterator gr;

				// INIT / ACT
				layout l1;
				layout l2;
				layout l3;
				layout l4;
				layout l5;

				l1.process(R("A"), limit::none(), *f1);
				l2.process(R("AAB"), limit::none(), *f1);
				l3.process(R("BQA"), limit::none(), *f1);
				l4.process(R("A"), limit::none(), *f2);
				l5.process(R("ABQ A  QA"), limit::none(), *f2);

				// ASSERT
				positioned_glyph reference1[] = { { 1, { 11.0f, 0.0f } } };
				positioned_glyph reference2[] = { { 1, { 11.0f, 0.0f } }, { 1, { 11.0f, 0.0f } }, { 0, { 13.0f, 0.0f } }, };
				positioned_glyph reference3[] = { { 0, { 13.0f, 0.0f } }, { 0, { 13.0f, 0.0f } }, { 1, { 11.0f, 0.0f } }, };
				positioned_glyph reference4[] = { { 0, { 13.0f, 0.0f } }, };
				positioned_glyph reference5[] = {
					{ 0, { 13.0f, 0.0f } },
					{ 1, { 11.0f, 0.0f } },
					{ 2, { 12.7f, 0.0f } },
					{ 3, { 10.1f, 0.0f } },
					{ 0, { 13.0f, 0.0f } },
					{ 3, { 10.1f, 0.0f } },
					{ 3, { 10.1f, 0.0f } },
					{ 2, { 12.7f, 0.0f } },
					{ 0, { 13.0f, 0.0f } },
				};

				assert_equal(plural + ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(**f1, 0.0f, 0.0f, reference1)),
					mkvector(l1.begin(), l1.end()));

				assert_equal(plural + ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(**f1, 0.0f, 0.0f, reference2)),
					mkvector(l2.begin(), l2.end()));

				assert_equal(plural + ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(**f1, 0.0f, 0.0f, reference3)),
					mkvector(l3.begin(), l3.end()));

				assert_equal(plural + ref_text_line(0.0f, 14.0f, 0.0f, plural + ref_glyph_run(**f2, 0.0f, 0.0f, reference4)),
					mkvector(l4.begin(), l4.end()));

				assert_equal(plural + ref_text_line(0.0f, 14.0f, 0.0f, plural + ref_glyph_run(**f2, 0.0f, 0.0f, reference5)),
					mkvector(l5.begin(), l5.end()));
			}


			test( MultiLineUnboundLayoutBoxEqualsMaxOfSumOfAdvancesInEachRow )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L' ', 0 }, { L'A', 1 }, { L'B', 2 }, { L'C', 3 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 7.1, 0 } },
					{ { 11, 0 } },
					{ { 13, 0 } },
					{ { 17, 0 } },
				};
				factory_ptr f = create_single_font_factory(c_fm1, indices, glyphs);

				// ACT
				layout l1;
				layout l2;
				layout l3;

				l1.process(R("ABC CBA AB\nABB BBC\n"), limit::none(), *f);
				l2.process(R("AC CB\nA AB\nABB BBC\n"), limit::none(), *f);
				l3.process(R("AC CB\nA AB\nABB BBC"), limit::none(), *f); // Last row will be checked even if no newline is encountered.

				box_r box1 = l1.get_box();
				box_r box2 = l2.get_box();
				box_r box3 = l3.get_box();

				// ASSERT
				assert_equal(120.2f, box1.w);
				assert_equal(87.1f, box2.w);
				assert_equal(87.1f, box3.w);
			}


			test( MultiLineUnboundLayoutProducesGlyphRunsForEachLine )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L' ', 0 }, { L'A', 1 }, { L'B', 2 }, { L'C', 3 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 7.1, 0 } },
					{ { 11, 0 } },
					{ { 13, 0 } },
					{ { 17, 0 } },
				};
				factory_ptr f1 = create_single_font_factory(c_fm1, indices, glyphs);
				factory_ptr f2 = create_single_font_factory(c_fm2, indices, glyphs);
				layout::const_iterator gr;

				// INIT / ACT
				layout l1;
				layout l2;

				l1.process(R("ABC CBA AB\nABB BBC\n"), limit::none(), *f1);
				l2.process(R("AC CB\nA AB\nABB BBC"), limit::none(), *f2);

				// ASSERT
				positioned_glyph reference11[] = {
					{ 1, { 11.0f, 0.0f } }, { 2, { 13.0f, 0.0f } }, { 3, { 17.0f, 0.0f } }, { 0, { 7.1f, 0.0f } }, { 3, { 17.0f, 0.0f } },
						{ 2, { 13.0f, 0.0f } }, { 1, { 11.0f, 0.0f } }, { 0, { 7.1f, 0.0f } }, { 1, { 11.0f, 0.0f } }, { 2, { 13.0f, 0.0f } },
				};
				positioned_glyph reference12[] = {
					{ 1, { 11.0f, 0.0f } }, { 2, { 13.0f, 0.0f } }, { 2, { 13.0f, 0.0f } }, { 0, { 7.1f, 0.0f } }, { 2, { 13.0f, 0.0f } },
						{ 2, { 13.0f, 0.0f } }, { 3, { 17.0f, 0.0f } }, 
				};

				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 120.2f, plural + ref_glyph_run(**f1, 0.0f, 0.0f, reference11))
					+ ref_text_line(0.0f, 24.0f, 87.1f, plural + ref_glyph_run(**f1, 0.0f, 0.0f, reference12)),
					mkvector(l1.begin(), l1.end()));


				positioned_glyph reference21[] = {
					{ 1, { 11.0f, 0.0f } }, { 3, { 17.0f, 0.0f } }, { 0, { 7.1f, 0.0f } }, { 3, { 17.0f, 0.0f } }, { 2, { 13.0f, 0.0f } },
				};
				positioned_glyph reference22[] = {
					{ 1, { 11.0f, 0.0f } }, { 0, { 7.1f, 0.0f } }, { 1, { 11.0f, 0.0f } }, { 2, { 13.0f, 0.0f } },
				};
				positioned_glyph reference23[] = {
					{ 1, { 11.0f, 0.0f } }, { 2, { 13.0f, 0.0f } }, { 2, { 13.0f, 0.0f } }, { 0, { 7.1f, 0.0f } }, { 2, { 13.0f, 0.0f } },
						{ 2, { 13.0f, 0.0f } }, { 3, { 17.0f, 0.0f } },
				};

				assert_equal(plural
					+ ref_text_line(0.0f, 14.0f, 65.1f, plural + ref_glyph_run(**f2, 0.0f, 0.0f, reference21))
					+ ref_text_line(0.0f, 32.0f, 42.1f, plural + ref_glyph_run(**f2, 0.0f, 0.0f, reference22))
					+ ref_text_line(0.0f, 50.0f, 87.1f, plural + ref_glyph_run(**f2, 0.0f, 0.0f, reference23)),
					mkvector(l2.begin(), l2.end()));
			}


			test( MultiLineUnboundLayoutProducesGlyphRunsForEachLineUTF8 )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L' ', 0 }, { 0x2713, 1 }, { L'B', 2 }, { 0xE0, 3 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 7.1, 0 } },
					{ { 11, 0 } },
					{ { 13, 0 } },
					{ { 17, 0 } },
				};
				factory_ptr f1 = create_single_font_factory(c_fm1, indices, glyphs);
				factory_ptr f2 = create_single_font_factory(c_fm2, indices, glyphs);
				layout::const_iterator gr;

				// INIT / ACT
				layout l1;
				layout l2;

				l1.process(R("\xE2\x9C\x93""B\xC3\xA0 \xC3\xA0""B\xE2\x9C\x93 \xE2\x9C\x93""B\n\xE2\x9C\x93""BB BB\xC3\xA0\n"), limit::none(), *f1);
				l2.process(R("\xE2\x9C\x93""\xC3\xA0 \xC3\xA0""B\n\xE2\x9C\x93 \xE2\x9C\x93""B\n\xE2\x9C\x93""BB BB\xC3\xA0"), limit::none(), *f2);

				// ASSERT
				positioned_glyph reference11[] = {
					{ 1, { 11.0f, 0.0f } }, { 2, { 13.0f, 0.0f } }, { 3, { 17.0f, 0.0f } }, { 0, { 7.1f, 0.0f } }, { 3, { 17.0f, 0.0f } },
						{ 2, { 13.0f, 0.0f } }, { 1, { 11.0f, 0.0f } }, { 0, { 7.1f, 0.0f } }, { 1, { 11.0f, 0.0f } }, { 2, { 13.0f, 0.0f } },
				};
				positioned_glyph reference12[] = {
					{ 1, { 11.0f, 0.0f } }, { 2, { 13.0f, 0.0f } }, { 2, { 13.0f, 0.0f } }, { 0, { 7.1f, 0.0f } }, { 2, { 13.0f, 0.0f } },
						{ 2, { 13.0f, 0.0f } }, { 3, { 17.0f, 0.0f } }, 
				};

				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 120.2f, plural + ref_glyph_run(**f1, 0.0f, 0.0f, reference11))
					+ ref_text_line(0.0f, 24.0f, 87.1f, plural + ref_glyph_run(**f1, 0.0f, 0.0f, reference12)),
					mkvector(l1.begin(), l1.end()));


				positioned_glyph reference21[] = {
					{ 1, { 11.0f, 0.0f } }, { 3, { 17.0f, 0.0f } }, { 0, { 7.1f, 0.0f } }, { 3, { 17.0f, 0.0f } }, { 2, { 13.0f, 0.0f } },
				};
				positioned_glyph reference22[] = {
					{ 1, { 11.0f, 0.0f } }, { 0, { 7.1f, 0.0f } }, { 1, { 11.0f, 0.0f } }, { 2, { 13.0f, 0.0f } },
				};
				positioned_glyph reference23[] = {
					{ 1, { 11.0f, 0.0f } }, { 2, { 13.0f, 0.0f } }, { 2, { 13.0f, 0.0f } }, { 0, { 7.1f, 0.0f } }, { 2, { 13.0f, 0.0f } },
						{ 2, { 13.0f, 0.0f } }, { 3, { 17.0f, 0.0f } },
				};

				assert_equal(plural
					+ ref_text_line(0.0f, 14.0f, 65.1f, plural + ref_glyph_run(**f2, 0.0f, 0.0f, reference21))
					+ ref_text_line(0.0f, 32.0f, 42.1f, plural + ref_glyph_run(**f2, 0.0f, 0.0f, reference22))
					+ ref_text_line(0.0f, 50.0f, 87.1f, plural + ref_glyph_run(**f2, 0.0f, 0.0f, reference23)),
					mkvector(l2.begin(), l2.end()));
			}


			test( TrivialLineFeedsDoNotProduceEmptyLines )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L' ', 0 }, { L'A', 1 }, { L'B', 2 }, { L'C', 3 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 7.1, 0 } },
					{ { 11, 0 } },
					{ { 13, 0 } },
					{ { 17, 0 } },
				};
				factory_ptr f = create_single_font_factory(c_fm1, indices, glyphs);

				// ACT
				layout l;

				l.process(R("ABC CBA AB\n\n\nABB BBC\n\n"), limit::none(), *f);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 1 + 2 + 3 + 0 + 3 + 2 + 1 + 0 + 1 + 2))
					+ ref_text_line(0.0f, 52.0f, 0.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 1 + 2 + 2 + 0 + 2 + 2 + 3)),
					mkvector(l.begin(), l.end()));
			}


			test( LongSingleLineIsBrokenOnWordBounds )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = {
					{ L' ', 0 }, { L'A', 1 }, { L'B', 2 }, { L'C', 3 }, { L'\'', 4 }, { L'.', 5 },
				};
				mocks::font_accessor::glyph glyphs[] = {
					{ { 7.1, 0 } },
					{ { 11, 0 } },
					{ { 12, 0 } },
					{ { 13, 0 } },
					{ { 3, 0 } },
					{ { 3, 0 } },
				};
				factory_ptr f = create_single_font_factory(c_fm1, indices, glyphs);
				layout::const_iterator gr;

				layout l1;
				layout l2;

				// ACT
				// 44 + 7.1 + 48 + 7.1 + 26 + 7.1 + 48 + 7.1 + 44
				// AAAA BBBB CC|BBBB AAAA
				l1.process (R("AAAA BBBB CC BBBB AAAA"), limit::wrap(139.1f), *f);

				// 55 + 7.1 + 36 + 7.1 + 22 + 7.1 + 22 + 7.1 + 80 + 7.1 + 52 + 7.1 + 44 + 7.1 + 118
				// CCC'C BBB AA|AA AAAABBB|CCCC AAAA|ABABABABAB.
				l2.process(R("CCC'C BBB AA AA AAAABBB CCCC AAAA ABABABABAB."), limit::wrap(139.1f), *f);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 132.2f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 1 + 1 + 1 + 1 + 0 + 2 + 2 + 2 + 2 + 0 + 3 + 3))
					+ ref_text_line(0.0f, 24.0f, 99.1f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 2 + 2 + 2 + 2 + 0 + 1 + 1 + 1 + 1)),
					mkvector(l1.begin(), l1.end()));

				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 127.2f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 3 + 3 + 3 + 4 + 3 + 0 + 2 + 2 + 2 + 0 + 1 + 1))
					+ ref_text_line(0.0f, 24.0f, 109.1f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 1 + 1 + 0 + 1 + 1 + 1 + 1 + 2 + 2 + 2))
					+ ref_text_line(0.0f, 38.0f, 103.1f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 3 + 3 + 3 +3 + 0 + 1 + 1 + 1 + 1))
					+ ref_text_line(0.0f, 52.0f, 118.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 5)),
					mkvector(l2.begin(), l2.end()));
			}


			test( LongWordsAreEmergentlyBrokenOnWidthLimit )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, { L'B', 1 }, { L'C', 2 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 1, 0 } },
					{ { 2, 0 } },
					{ { 3, 0 } },
				};
				factory_ptr f = create_single_font_factory(c_fm1, indices, glyphs);
				layout::const_iterator gr;
				layout l;

				// ACT
				l.process(R("ABCABCABC"), limit::wrap(6.0f), *f);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 6.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 0 + 1 + 2))
					+ ref_text_line(0.0f, 24.0f, 6.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 0 + 1 + 2))
					+ ref_text_line(0.0f, 38.0f, 6.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 0 + 1 + 2)),
					mkvector(l.begin(), l.end()));

				// ACT
				l.process(R("ABCABCABC"), limit::wrap(8.0f), *f);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 7.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 0 + 1 + 2 + 0))
					+ ref_text_line(0.0f, 24.0f, 8.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 1 + 2 + 0 + 1))
					+ ref_text_line(0.0f, 38.0f, 3.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 2)),
					mkvector(l.begin(), l.end()));
			}


			test( BoundingBoxHeightDependsOnFontAscentAndDescend )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 5, 0 } },
				};
				factory_ptr f1 = create_single_font_factory(c_fm1, indices, glyphs);
				layout l1;

				l1.process(R("AAAAA"), limit::none(), *f1);

				// ACT
				box_r box1 = l1.get_box();

				// ASSERT
				assert_equal(12.0f, box1.h);

				// INIT
				factory_ptr f2 = create_single_font_factory(c_fm2, indices, glyphs);
				layout l2;

				l2.process(R("AAAAA"), limit::none(), *f2);

				// ACT
				box_r box2 = l2.get_box();

				// ASSERT
				assert_equal(17.0f, box2.h);
			}


			test( BoundingBoxHeightOfMultilineTextIsMultipleOfAscendDescendLeadingAndLineCountMinusOneLeading )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 5, 0 } },
				};
				factory_ptr f1 = create_single_font_factory(c_fm1, indices, glyphs);
				layout l1;

				l1.process(R("AAAAA\nAA"), limit::none(), *f1);

				// ACT
				box_r box1 = l1.get_box();

				// ASSERT
				assert_equal(26.0f, box1.h);

				// INIT
				factory_ptr f2 = create_single_font_factory(c_fm2, indices, glyphs);
				layout l2;

				l2.process(R("AAAAA\nA\nA"), limit::none(), *f2);

				// ACT
				box_r box2 = l2.get_box();

				// ASSERT
				assert_equal(53.0f, box2.h);
			}


			test( NextWordIsDisplayedWithoutSpacesOnWordBreakWhenNextWordIsFound )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, { L' ', 1 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 5, 0 } },
					{ { 4, 0 } },
				};
				factory_ptr f = create_single_font_factory(c_fm1, indices, glyphs);
				layout l;

				// ACT
				l.process(R("AAAAA   AAAA    AA"), limit::wrap(39.0f), *f);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 0 + 0 + 0 + 0 + 0))
					+ ref_text_line(0.0f, 24.0f, 0.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 0 + 0 + 0 + 0))
					+ ref_text_line(0.0f, 38.0f, 0.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 0 + 0)),
					mkvector(l.begin(), l.end()));
			}


			test( NextWordIsDisplayedWithoutSpacesOnWordBreakWhenNextWordIsNotFound )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 1 }, { L' ', 0 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 4, 0 } },
					{ { 5, 0 } },
				};
				factory_ptr f = create_single_font_factory(c_fm1, indices, glyphs);
				layout l;

				// ACT
				l.process(R("AAAAA   A   A"), limit::wrap(26.0f), *f);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 1 + 1 + 1 + 1 + 1))
					+ ref_text_line(0.0f, 24.0f, 0.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 1 + 0 + 0 + 0 + 1)),
					mkvector(l.begin(), l.end()));
			}


			test( ConsequentWordBreakWorkWhenNextWordIsFound )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L' ', 0 }, { L'A', 1 }, { L'B', 2 }, { L'C', 3 }, { L'D', 4 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 3, 0 } },
					{ { 5, 0 } },
					{ { 7, 0 } },
					{ { 11, 0 } },
					{ { 13, 0 } },
				};
				factory_ptr f = create_single_font_factory(c_fm1, indices, glyphs);
				layout l;

				// ACT
				// 36 + 3 + 61
				// ABCD|ABCDCD[]C
				l.process(R("ABCD ABCDCDC"), limit::wrap(60.0f), *f);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 1 + 2 + 3 + 4))
					+ ref_text_line(0.0f, 24.0f, 0.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 1 + 2 + 3 + 4 + 3 + 4))
					+ ref_text_line(0.0f, 38.0f, 0.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 3)),
					mkvector(l.begin(), l.end()));
			}


			test( ConsequentWordBreakWorkWhenNextWordIsNotFound )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L' ', 0 }, { L'A', 1 }, { L'B', 2 }, { L'C', 3 }, { L'D', 4 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 3, 0 } },
					{ { 5, 0 } },
					{ { 7, 0 } },
					{ { 11, 0 } },
					{ { 13, 0 } },
				};
				factory_ptr f = create_single_font_factory(c_fm1, indices, glyphs);
				layout l;

				// ACT
				// 36 + 3 + 61
				// ABCD|ABCD[]CDC
				l.process(R("ABCD ABCDCDC"), limit::wrap(38.0f), *f);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 1 + 2 + 3 + 4))
					+ ref_text_line(0.0f, 24.0f, 0.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 1 + 2 + 3 + 4))
					+ ref_text_line(0.0f, 38.0f, 0.0f, plural + ref_glyph_run(**f, 0.0f, 0.0f, plural + 3 + 4 + 3)),
					mkvector(l.begin(), l.end()));
			}


			test( TooNarrowLimitProducesNoLayout )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = {	{ L'Z', 0 }, { L'q', 1 }	};
				mocks::font_accessor::glyph glyphs[] = {
					{ { 10, 0 } },
					{ { 8, 0 } },
				};
				factory_ptr f = create_single_font_factory(c_fm1, indices, glyphs);
				layout l;

				// ACT / ASSERT
				l.process(R("Z"), limit::wrap(9.0f), *f);
				assert_equal(l.end(), l.begin());
				l.process(R("\n"), limit::wrap(9.0f), *f);
				assert_equal(l.end(), l.begin());
				l.process(R("Z\nZ"), limit::wrap(9.0f), *f);
				assert_equal(l.end(), l.begin());
				l.process(R("qZ\n"), limit::wrap(9.0f), *f);
				assert_equal(l.end(), l.begin());
			}


			struct LineFeedDetector
			{
				LineFeedDetector()
					: history(new vector<int>)
				{	}

				void begin_style(const layout_builder &/*builder*/)
				{	}

				void new_line()
				{	history->push_back(-1);	}
				
				template <typename CharIteratorT>
				bool add_glyph(layout_builder &/*builder*/, glyph_index_t glyph_index, real_t /*advance*/,
					CharIteratorT &i, CharIteratorT next, CharIteratorT /*end*/)
				{
					history->push_back(glyph_index);
					i = next;
					return true;
				}

				shared_ptr< vector<int> > history;
			};

			test( LimitProcessorIsNotifiedOfNewLines )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = {	{ L'Z', 0 }, { L'q', 1 }	};
				mocks::font_accessor::glyph glyphs[] = {
					{ { 10, 0 } },
					{ { 8, 0 } },
				};
				LineFeedDetector d;
				factory_ptr f = create_single_font_factory(c_fm1, indices, glyphs);
				layout l;

				// ACT
				l.process(R("Zq\nZ"), d, *f);

				// ASSERT
				int reference1[] = {	0, 1, -1, 0,	};

				assert_equal(reference1, *d.history);

				// ACT
				l.process(R("Zq\nZ"), d, *f);

				// INIT
				d = LineFeedDetector();

				// ACT
				l.process(R("qqqZ\nZ\n\nqZ\n"), d, *f);

				// ASSERT
				int reference2[] = {	1, 1, 1, 0, -1, 0, -1, -1, 1, 0, -1,	};

				assert_equal(reference2, *d.history);
			}
		end_test_suite
	}
}
