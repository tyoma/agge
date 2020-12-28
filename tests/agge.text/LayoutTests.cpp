#include <agge.text/layout.h>

#include "helpers.h"
#include "mocks.h"

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
			font::metrics c_fm1 = { 10.0f, 2.0f, 2.0f };
			font::metrics c_fm2 = { 14.0f, 3.0f, 1.0f };
		}

		begin_test_suite( LayoutTests )
			test( EmptyLayoutHasEmptyBox )
			{
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { { { 0, 0 } }, };
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);

				// INIT / ACT
				layout l(f);

				l.process(L"");

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
				font::ptr f1 = mocks::create_font(c_fm1, indices1, glyphs);

				// INIT / ACT
				layout l1(f1);
				layout l2(f1);
				layout l3(f1);

				l1.process(L"A");
				l2.process(L"AAB");
				l3.process(L"BQA");

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
				font::ptr f2 = mocks::create_font(c_fm1, indices2, glyphs);

				// INIT / ACT
				layout l4(f2);
				layout l5(f2);

				l4.process(L"A");
				l5.process(L"ABQABQABQ");

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
				font::ptr f1 = mocks::create_font(c_fm1, indices1, glyphs);
				font::ptr f2 = mocks::create_font(c_fm1, indices2, glyphs);
				layout::const_iterator gr;

				// INIT / ACT
				layout l1(f1);
				layout l2(f1);
				layout l3(f1);
				layout l4(f2);
				layout l5(f2);

				l1.process(L"A");
				l2.process(L"AAB");
				l3.process(L"BQA");
				l4.process(L"A");
				l5.process(L"ABQ A  QA");

				// ASSERT
				positioned_glyph reference1[] = { { 11.0f, 0.0f, 1 } };
				positioned_glyph reference2[] = { { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 13.0f, 0.0f, 0 }, };
				positioned_glyph reference3[] = { { 13.0f, 0.0f, 0 }, { 13.0f, 0.0f, 0 }, { 11.0f, 0.0f, 1 }, };
				positioned_glyph reference4[] = { { 13.0f, 0.0f, 0 }, };
				positioned_glyph reference5[] = {
					{ 13.0f, 0.0f, 0 },
					{ 11.0f, 0.0f, 1 },
					{ 12.7f, 0.0f, 2 },
					{ 10.1f, 0.0f, 3 },
					{ 13.0f, 0.0f, 0 },
					{ 10.1f, 0.0f, 3 },
					{ 10.1f, 0.0f, 3 },
					{ 12.7f, 0.0f, 2 },
					{ 13.0f, 0.0f, 0 },
				};

				gr = l1.begin();
				assert_equal(1, std::distance(gr, l1.end()));
				assert_equal(f1, gr->glyph_run_font);
				assert_equal(reference1, mkvector(gr->begin(), gr->end()));

				gr = l2.begin();
				assert_equal(1, std::distance(gr, l2.end()));
				assert_equal(f1, gr->glyph_run_font);
				assert_equal(reference2, mkvector(gr->begin(), gr->end()));

				gr = l3.begin();
				assert_equal(1, std::distance(gr, l3.end()));
				assert_equal(reference3, mkvector(gr->begin(), gr->end()));

				gr = l4.begin();
				assert_equal(1, std::distance(gr, l4.end()));
				assert_equal(f2, gr->glyph_run_font);
				assert_equal(reference4, mkvector(gr->begin(), gr->end()));

				gr = l5.begin();
				assert_equal(1, std::distance(gr, l5.end()));
				assert_equal(reference5, mkvector(gr->begin(), gr->end()));
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
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);

				// ACT
				layout l1(f);
				layout l2(f);
				layout l3(f);

				l1.process(L"ABC CBA AB\nABB BBC\n");
				l2.process(L"AC CB\nA AB\nABB BBC\n");
				l3.process(L"AC CB\nA AB\nABB BBC"); // Last row will be checked even if no newline is encountered.

				box_r box1 = l1.get_box();
				box_r box2 = l2.get_box();
				box_r box3 = l3.get_box();

				// ASSERT
				assert_equal(120.2f, box1.w);
				assert_equal(87.1f, box2.w);
				assert_equal(87.1f, box3.w);
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
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);

				// ACT
				layout l(f);

				l.process(L"ABC CBA AB\n\n\nABB BBC\n\n");

				// ASSERT
				glyph_index_t reference1[] = {	1, 2, 3, 0, 3, 2, 1, 0, 1, 2,	};
				glyph_index_t reference2[] = {	1, 2, 2, 0, 2, 2, 3,	};
				layout::const_iterator gr = l.begin();

				assert_equal(2, std::distance(gr, l.end()));
				assert_equal(create_vector(0.0f, 10.0f), gr->offset);
				assert_equal(reference1, mkvector(gr->begin(), gr->end()));
				++gr;
				assert_equal(create_vector(0.0f, 52.0f), gr->offset);
				assert_equal(reference2, mkvector(gr->begin(), gr->end()));
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
				font::ptr f1 = mocks::create_font(c_fm1, indices, glyphs);
				font::ptr f2 = mocks::create_font(c_fm2, indices, glyphs);
				layout::const_iterator gr;

				// INIT / ACT
				layout l1(f1);
				layout l2(f2);

				l1.process(L"ABC CBA AB\nABB BBC\n");
				l2.process(L"AC CB\nA AB\nABB BBC");

				// ASSERT
				positioned_glyph reference11[] = {
					{ 11.0f, 0.0f, 1 }, { 13.0f, 0.0f, 2 }, { 17.0f, 0.0f, 3 }, { 7.1f, 0.0f, 0 }, { 17.0f, 0.0f, 3 },
						{ 13.0f, 0.0f, 2 }, { 11.0f, 0.0f, 1 }, { 7.1f, 0.0f, 0 }, { 11.0f, 0.0f, 1 }, { 13.0f, 0.0f, 2 },
				};
				positioned_glyph reference12[] = {
					{ 11.0f, 0.0f, 1 }, { 13.0f, 0.0f, 2 }, { 13.0f, 0.0f, 2 }, { 7.1f, 0.0f, 0 }, { 13.0f, 0.0f, 2 },
						{ 13.0f, 0.0f, 2 }, { 17.0f, 0.0f, 3 }, 
				};
				positioned_glyph reference21[] = {
					{ 11.0f, 0.0f, 1 }, { 17.0f, 0.0f, 3 }, { 7.1f, 0.0f, 0 }, { 17.0f, 0.0f, 3 }, { 13.0f, 0.0f, 2 },
				};
				positioned_glyph reference22[] = {
					{ 11.0f, 0.0f, 1 }, { 7.1f, 0.0f, 0 }, { 11.0f, 0.0f, 1 }, { 13.0f, 0.0f, 2 },
				};
				positioned_glyph reference23[] = {
					{ 11.0f, 0.0f, 1 }, { 13.0f, 0.0f, 2 }, { 13.0f, 0.0f, 2 }, { 7.1f, 0.0f, 0 }, { 13.0f, 0.0f, 2 },
						{ 13.0f, 0.0f, 2 }, { 17.0f, 0.0f, 3 },
				};

				gr = l1.begin();
				assert_equal(2, std::distance(gr, l1.end()));
				assert_equal(create_vector(0.0f, 10.0f), gr->offset);
				assert_equal(reference11, mkvector(gr->begin(), gr->end()));
				++gr;
				assert_equal(create_vector(0.0f, 24.0f), gr->offset);
				assert_equal(reference12, mkvector(gr->begin(), gr->end()));

				gr = l2.begin();
				assert_equal(3, std::distance(gr, l2.end()));
				assert_equal(create_vector(0.0f, 14.0f), gr->offset);
				assert_equal(reference21, mkvector(gr->begin(), gr->end()));
				++gr;
				assert_equal(create_vector(0.0f, 32.0f), gr->offset);
				assert_equal(reference22, mkvector(gr->begin(), gr->end()));
				++gr;
				assert_equal(create_vector(0.0f, 50.0f), gr->offset);
				assert_equal(reference23, mkvector(gr->begin(), gr->end()));
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
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);
				layout::const_iterator gr;
				
				layout l1(f);

				layout l2(f);

				// ACT
				// 44 + 7.1 + 48 + 7.1 + 26 + 7.1 + 48 + 7.1 + 44
				l1.set_width_limit(139.1f); // AAAA BBBB CC|BBBB AAAA
				l1.process (L"AAAA BBBB CC BBBB AAAA");

				// 55 + 7.1 + 36 + 7.1 + 22 + 7.1 + 22 + 7.1 + 80 + 7.1 + 52 + 7.1 + 44 + 7.1 + 118
				l2.set_width_limit(139.1f); // CCC'C BBB AA|AA AAAABBB|CCCC AAAA|ABABABABAB.
				l2.process(L"CCC'C BBB AA AA AAAABBB CCCC AAAA ABABABABAB.");

				// ASSERT
				positioned_glyph reference11[] = {
					{ 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 7.1f, 0.0f, 0 },
					{ 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 7.1f, 0.0f, 0 },
					{ 13.0f, 0.0f, 3 }, { 13.0f, 0.0f, 3 },
				};
				positioned_glyph reference12[] = {
					{ 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 7.1f, 0.0f, 0 },
					{ 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 },
				};

				positioned_glyph reference21[] = {
					{ 13.0f, 0.0f, 3 }, { 13.0f, 0.0f, 3 }, { 13.0f, 0.0f, 3 }, { 3.0f, 0.0f, 4 }, { 13.0f, 0.0f, 3 },
						{ 7.1f, 0.0f, 0 },
					{ 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 7.1f, 0.0f, 0 },
					{ 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 },
				};
				positioned_glyph reference22[] = {
					{ 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 7.1f, 0.0f, 0 },
					{ 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 12.0f, 0.0f, 2 },
						{ 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 },
				};
				positioned_glyph reference23[] = {
					{ 13.0f, 0.0f, 3 }, { 13.0f, 0.0f, 3 }, { 13.0f, 0.0f, 3 }, { 13.0f, 0.0f, 3 }, { 7.1f, 0.0f, 0 },
					{ 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, 
				};
				positioned_glyph reference24[] = {
					{ 11.0f, 0.0f, 1 }, { 12.0f, 0.0f, 2 }, { 11.0f, 0.0f, 1 }, { 12.0f, 0.0f, 2 }, { 11.0f, 0.0f, 1 },
						{ 12.0f, 0.0f, 2 }, { 11.0f, 0.0f, 1 }, { 12.0f, 0.0f, 2 }, { 11.0f, 0.0f, 1 }, { 12.0f, 0.0f, 2 },
						{ 3.0f, 0.0f, 5 },
				};

				gr = l1.begin();
				assert_equal(2, std::distance(gr, l1.end()));
				assert_approx_equal(132.2f, gr->width, 0.001f);
				assert_equal(create_vector(0.0f, 10.0f), gr->offset);
				assert_equal(reference11, mkvector(gr->begin(), gr->end()));
				++gr;
				assert_approx_equal(99.1f, gr->width, 0.001f);
				assert_equal(create_vector(0.0f, 24.0f), gr->offset);
				assert_equal(reference12, mkvector(gr->begin(), gr->end()));

				gr = l2.begin();
				assert_equal(4, std::distance(gr, l2.end()));
				assert_equal(reference21, mkvector(gr->begin(), gr->end()));
				++gr;
				assert_equal(reference22, mkvector(gr->begin(), gr->end()));
				++gr;
				assert_equal(reference23, mkvector(gr->begin(), gr->end()));
				++gr;
				assert_equal(reference24, mkvector(gr->begin(), gr->end()));
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
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);
				layout::const_iterator gr;
				layout l(f);

				// ACT
				l.set_width_limit(6);
				l.process(L"ABCABCABC");

				// ASSERT
				positioned_glyph reference1[] = { { 1.0f, 0.0f, 0 }, { 2.0f, 0.0f, 1 }, { 3.0f, 0.0f, 2 }, };

				gr = l.begin();
				assert_equal(3, std::distance(gr, l.end()));
				assert_equal(reference1, mkvector(gr->begin(), gr->end()));
				++gr;
				assert_equal(reference1, mkvector(gr->begin(), gr->end()));
				++gr;
				assert_equal(reference1, mkvector(gr->begin(), gr->end()));

				// ACT
				l.set_width_limit(8);
				l.process(L"ABCABCABC");

				// ASSERT
				positioned_glyph reference21[] = {
					{ 1.0f, 0.0f, 0 }, { 2.0f, 0.0f, 1 }, { 3.0f, 0.0f, 2 }, { 1.0f, 0.0f, 0 },
				};
				positioned_glyph reference22[] = {
					{ 2.0f, 0.0f, 1 }, { 3.0f, 0.0f, 2 }, { 1.0f, 0.0f, 0 }, { 2.0f, 0.0f, 1 },
				};
				positioned_glyph reference23[] = {
					{ 3.0f, 0.0f, 2 },
				};

				gr = l.begin();
				assert_equal(3, std::distance(gr, l.end()));
				assert_equal(reference21, mkvector(gr->begin(), gr->end()));
				++gr;
				assert_equal(reference22, mkvector(gr->begin(), gr->end()));
				++gr;
				assert_equal(reference23, mkvector(gr->begin(), gr->end()));
			}


			test( BoundingBoxHeightDependsOnFontAscentAndDescend )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 5, 0 } },
				};
				font::ptr f1 = mocks::create_font(c_fm1, indices, glyphs);
				layout l1(f1);

				l1.process(L"AAAAA");

				// ACT
				box_r box1 = l1.get_box();

				// ASSERT
				assert_equal(12.0f, box1.h);

				// INIT
				font::ptr f2 = mocks::create_font(c_fm2, indices, glyphs);
				layout l2(f2);

				l2.process(L"AAAAA");

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
				font::ptr f1 = mocks::create_font(c_fm1, indices, glyphs);
				layout l1(f1);

				l1.process(L"AAAAA\nAA");

				// ACT
				box_r box1 = l1.get_box();

				// ASSERT
				assert_equal(26.0f, box1.h);

				// INIT
				font::ptr f2 = mocks::create_font(c_fm2, indices, glyphs);
				layout l2(f2);

				l2.process(L"AAAAA\nA\nA");

				// ACT
				box_r box2 = l2.get_box();

				// ASSERT
				assert_equal(53.0f, box2.h);
			}


			test( SettingWidthResetsLayoutContent )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 5, 0 } },
				};
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);
				layout l(f);

				l.process(L"AAAAA\nAA");

				// ACT
				l.set_width_limit(15.0f);

				// ASSERT
				assert_equal(l.end(), l.begin());

				// ACT
				l.process(L"A");

				// ASSERT
				assert_equal(1, distance(l.begin(), l.end()));
				assert_equal(0u, l.begin()->begin_index);
			}


			test( NextWordIsDisplayedWithoutSpacesOnWordBreakWhenNextWordIsFound )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, { L' ', 1 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 5, 0 } },
					{ { 4, 0 } },
				};
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);
				layout l(f);

				l.set_width_limit(39.0f);

				// ACT
				l.process(L"AAAAA   AAAA    AA");

				// ASSERT
				glyph_index_t reference1[] = {	0, 0, 0, 0, 0,	};
				glyph_index_t reference2[] = {	0, 0, 0, 0,	};
				glyph_index_t reference3[] = {	0, 0,	};

				assert_equal(3, distance(l.begin(), l.end()));
				layout::const_iterator i = l.begin();
				assert_equal(reference1, mkvector(i->begin(), i->end()));
				++i;
				assert_equal(reference2, mkvector(i->begin(), i->end()));
				++i;
				assert_equal(reference3, mkvector(i->begin(), i->end()));
			}


			test( NextWordIsDisplayedWithoutSpacesOnWordBreakWhenNextWordIsNotFound )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 1 }, { L' ', 0 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 4, 0 } },
					{ { 5, 0 } },
				};
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);
				layout l(f);

				l.set_width_limit(26.0f);

				// ACT
				l.process(L"AAAAA   A   A");

				// ASSERT
				glyph_index_t reference1[] = {	1, 1, 1, 1, 1,	};
				glyph_index_t reference2[] = {	1, 0, 0, 0, 1,	};

				assert_equal(2, distance(l.begin(), l.end()));
				layout::const_iterator i = l.begin();
				assert_equal(reference1, mkvector(i->begin(), i->end()));
				++i;
				assert_equal(reference2, mkvector(i->begin(), i->end()));
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
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);
				layout l(f);

				l.set_width_limit(60.0f);

				// ACT
				// 36 + 3 + 61
				// ABCD|ABCDCD[]C
				l.process(L"ABCD ABCDCDC");

				// ASSERT
				glyph_index_t reference1[] = {	1, 2, 3, 4,	};
				glyph_index_t reference2[] = {	1, 2, 3, 4, 3, 4,	};
				glyph_index_t reference3[] = {	3,	};

				assert_equal(3, distance(l.begin(), l.end()));
				layout::const_iterator i = l.begin();
				assert_equal(reference1, mkvector(i->begin(), i->end()));
				++i;
				assert_equal(reference2, mkvector(i->begin(), i->end()));
				++i;
				assert_equal(reference3, mkvector(i->begin(), i->end()));
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
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);
				layout l(f);

				l.set_width_limit(38.0f);

				// ACT
				// 36 + 3 + 61
				// ABCD|ABCD[]CDC
				l.process(L"ABCD ABCDCDC");

				// ASSERT
				glyph_index_t reference1[] = {	1, 2, 3, 4,	};
				glyph_index_t reference2[] = {	1, 2, 3, 4,	};
				glyph_index_t reference3[] = {	3,	4, 3, };

				assert_equal(3, distance(l.begin(), l.end()));
				layout::const_iterator i = l.begin();
				assert_approx_equal(36.0f, i->width, 0.001f);
				assert_equal(reference1, mkvector(i->begin(), i->end()));
				++i;
				assert_approx_equal(36.0f, i->width, 0.001f);
				assert_equal(reference2, mkvector(i->begin(), i->end()));
				++i;
				assert_approx_equal(35.0f, i->width, 0.001f);
				assert_equal(reference3, mkvector(i->begin(), i->end()));
			}
		end_test_suite
	}
}
