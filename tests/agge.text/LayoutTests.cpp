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
				layout l(L"", f);

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
				layout l1(L"A", f1);
				layout l2(L"AAB", f1);
				layout l3(L"BQA", f1);

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
				layout l4(L"A", f2);
				layout l5(L"ABQABQABQ", f2);

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
				layout l1(L"A", f1);
				layout l2(L"AAB", f1);
				layout l3(L"BQA", f1);
				layout l4(L"A", f2);
				layout l5(L"ABQ A  QA", f2);

				// ASSERT
				layout::positioned_glyph reference1[] = { { 0.0f, 0.0f, 1 } };
				layout::positioned_glyph reference2[] = { { 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 0 }, };
				layout::positioned_glyph reference3[] = { { 0.0f, 0.0f, 0 }, { 13.0f, 0.0f, 0 }, { 13.0f, 0.0f, 1 }, };
				layout::positioned_glyph reference4[] = { { 0.0f, 0.0f, 0 }, };
				layout::positioned_glyph reference5[] = {
					{ 0.0f, 0.0f, 0 },
					{ 13.0f, 0.0f, 1 },
					{ 11.0f, 0.0f, 2 },
					{ 12.7f, 0.0f, 3 },
					{ 10.1f, 0.0f, 0 },
					{ 13.0f, 0.0f, 3 },
					{ 10.1f, 0.0f, 3 },
					{ 10.1f, 0.0f, 2 },
					{ 12.7f, 0.0f, 0 },
				};

				gr = l1.begin();
				assert_equal(1, distance(gr, l1.end()));
				assert_equal(f1, gr->glyph_run_font);
				assert_equal(reference1, mkvector(gr->begin, gr->end));

				gr = l2.begin();
				assert_equal(1, distance(gr, l2.end()));
				assert_equal(f1, gr->glyph_run_font);
				assert_equal(reference2, mkvector(gr->begin, gr->end));

				gr = l3.begin();
				assert_equal(1, distance(gr, l3.end()));
				assert_equal(reference3, mkvector(gr->begin, gr->end));

				gr = l4.begin();
				assert_equal(1, distance(gr, l4.end()));
				assert_equal(f2, gr->glyph_run_font);
				assert_equal(reference4, mkvector(gr->begin, gr->end));

				gr = l5.begin();
				assert_equal(1, distance(gr, l5.end()));
				assert_equal(reference5, mkvector(gr->begin, gr->end));
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
				layout l1(L"ABC CBA AB\nABB BBC\n", f);
				layout l2(L"AC CB\nA AB\nABB BBC\n", f);
				layout l3(L"AC CB\nA AB\nABB BBC", f); // Last row will be checked even if no newline is encountered.
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
				font::ptr f1 = mocks::create_font(c_fm1, indices, glyphs);
				font::ptr f2 = mocks::create_font(c_fm2, indices, glyphs);
				layout::const_iterator gr;

				// INIT / ACT
				layout l1(L"ABC CBA AB\nABB BBC\n", f1);
				layout l2(L"AC CB\nA AB\nABB BBC", f2);

				// ASSERT
				layout::positioned_glyph reference11[] = {
					{ 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 2 }, { 13.0f, 0.0f, 3 }, { 17.0f, 0.0f, 0 }, { 7.1f, 0.0f, 3 },
						{ 17.0f, 0.0f, 2 }, { 13.0f, 0.0f, 1 }, { 11.0f, 0.0f, 0 }, { 7.1f, 0.0f, 1 }, { 11.0f, 0.0f, 2 },
				};
				layout::positioned_glyph reference12[] = {
					{ 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 2 }, { 13.0f, 0.0f, 2 }, { 13.0f, 0.0f, 0 }, { 7.1f, 0.0f, 2 },
						{ 13.0f, 0.0f, 2 }, { 13.0f, 0.0f, 3 }, 
				};
				layout::positioned_glyph reference21[] = {
					{ 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 3 }, { 17.0f, 0.0f, 0 }, { 7.1f, 0.0f, 3 }, { 17.0f, 0.0f, 2 },
				};
				layout::positioned_glyph reference22[] = {
					{ 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 0 }, { 7.1f, 0.0f, 1 }, { 11.0f, 0.0f, 2 },
				};
				layout::positioned_glyph reference23[] = {
					{ 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 2 }, { 13.0f, 0.0f, 2 }, { 13.0f, 0.0f, 0 }, { 7.1f, 0.0f, 2 },
						{ 13.0f, 0.0f, 2 }, { 13.0f, 0.0f, 3 },
				};

				gr = l1.begin();
				assert_equal(2, distance(gr, l1.end()));
				assert_equal(mkpoint(0.0f, 10.0f), gr->reference);
				assert_equal(reference11, mkvector(gr->begin, gr->end));
				++gr;
				assert_equal(mkpoint(0.0f, 24.0f), gr->reference);
				assert_equal(reference12, mkvector(gr->begin, gr->end));

				gr = l2.begin();
				assert_equal(3, distance(gr, l2.end()));
				assert_equal(mkpoint(0.0f, 14.0f), gr->reference);
				assert_equal(reference21, mkvector(gr->begin, gr->end));
				++gr;
				assert_equal(mkpoint(0.0f, 32.0f), gr->reference);
				assert_equal(reference22, mkvector(gr->begin, gr->end));
				++gr;
				assert_equal(mkpoint(0.0f, 50.0f), gr->reference);
				assert_equal(reference23, mkvector(gr->begin, gr->end));
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
				
				// 44 + 7.1 + 48 + 7.1 + 26 + 7.1 + 48 + 7.1 + 44
				layout l1(L"AAAA BBBB CC BBBB AAAA", f);

				// 55 + 7.1 + 36 + 7.1 + 22 + 7.1 + 22 + 7.1 + 80 + 7.1 + 52 + 7.1 + 44 + 7.1 + 118
				layout l2(L"CCC'C BBB AA AA AAAABBB CCCC AAAA ABABABABAB.", f);

				// ACT
				l1.limit_width(139.1f); // AAAA BBBB CC|BBBB AAAA
				l2.limit_width(139.1f); // CCC'C BBB AA|AA AAAABBB|CCCC AAAA|ABABABABAB.

				// ASSERT
				layout::positioned_glyph reference11[] = {
					{ 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 0 },
					{ 7.1f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 0 },
					{ 7.1f, 0.0f, 3 }, { 13.0f, 0.0f, 3 },
				};
				layout::positioned_glyph reference12[] = {
					{ 0.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 0 },
					{ 7.1f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 },
				};

				layout::positioned_glyph reference21[] = {
					{ 0.0f, 0.0f, 3 }, { 13.0f, 0.0f, 3 }, { 13.0f, 0.0f, 3 }, { 13.0f, 0.0f, 4 }, { 3.0f, 0.0f, 3 },
						{ 13.0f, 0.0f, 0 },
					{ 7.1f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 0 },
					{ 7.1f, 0.0f, 1 }, { 11.0f, 0.0f, 1 },
				};
				layout::positioned_glyph reference22[] = {
					{ 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 0 },
					{ 7.1f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 2 },
						{ 12.0f, 0.0f, 2 }, { 12.0f, 0.0f, 2 },
				};
				layout::positioned_glyph reference23[] = {
					{ 0.0f, 0.0f, 3 }, { 13.0f, 0.0f, 3 }, { 13.0f, 0.0f, 3 }, { 13.0f, 0.0f, 3 }, { 13.0f, 0.0f, 0 },
					{ 7.1f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, 
				};
				layout::positioned_glyph reference24[] = {
					{ 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 2 }, { 12.0f, 0.0f, 1 }, { 11.0f, 0.0f, 2 }, { 12.0f, 0.0f, 1 },
						{ 11.0f, 0.0f, 2 }, { 12.0f, 0.0f, 1 }, { 11.0f, 0.0f, 2 }, { 12.0f, 0.0f, 1 }, { 11.0f, 0.0f, 2 },
						{ 12.0f, 0.0f, 5 },
				};

				gr = l1.begin();
				assert_equal(2, distance(gr, l1.end()));
				assert_equal(reference11, mkvector(gr->begin, gr->end));
				++gr;
				assert_equal(reference12, mkvector(gr->begin, gr->end));

				gr = l2.begin();
				assert_equal(4, distance(gr, l2.end()));
				assert_equal(reference21, mkvector(gr->begin, gr->end));
				++gr;
				assert_equal(reference22, mkvector(gr->begin, gr->end));
				++gr;
				assert_equal(reference23, mkvector(gr->begin, gr->end));
				++gr;
				assert_equal(reference24, mkvector(gr->begin, gr->end));
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
				layout l(L"ABCABCABC", f);

				// ACT
				l.limit_width(6);

				// ASSERT
				layout::positioned_glyph reference1[] = { { 0.0f, 0.0f, 0 }, { 1.0f, 0.0f, 1 }, { 2.0f, 0.0f, 2 }, };

				gr = l.begin();
				assert_equal(3, distance(gr, l.end()));
				assert_equal(reference1, mkvector(gr->begin, gr->end));
				++gr;
				assert_equal(reference1, mkvector(gr->begin, gr->end));
				++gr;
				assert_equal(reference1, mkvector(gr->begin, gr->end));

				// ACT
				l.limit_width(8);

				// ASSERT
				layout::positioned_glyph reference21[] = {
					{ 0.0f, 0.0f, 0 }, { 1.0f, 0.0f, 1 }, { 2.0f, 0.0f, 2 }, { 3.0f, 0.0f, 0 },
				};
				layout::positioned_glyph reference22[] = {
					{ 0.0f, 0.0f, 1 }, { 2.0f, 0.0f, 2 }, { 3.0f, 0.0f, 0 }, { 1.0f, 0.0f, 1 },
				};
				layout::positioned_glyph reference23[] = {
					{ 0.0f, 0.0f, 2 },
				};

				gr = l.begin();
				assert_equal(3, distance(gr, l.end()));
				assert_equal(reference21, mkvector(gr->begin, gr->end));
				++gr;
				assert_equal(reference22, mkvector(gr->begin, gr->end));
				++gr;
				assert_equal(reference23, mkvector(gr->begin, gr->end));
			}


			test( BoundingBoxHeightDependsOnFontAscentAndDescend )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 5, 0 } },
				};
				font::ptr f1 = mocks::create_font(c_fm1, indices, glyphs);
				layout l1(L"AAAAA", f1);

				// ACT
				box_r box1 = l1.get_box();

				// ASSERT
				assert_equal(12.0f, box1.h);

				// INIT
				font::ptr f2 = mocks::create_font(c_fm2, indices, glyphs);
				layout l2(L"AAAAA", f2);

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
				layout l1(L"AAAAA\nAA", f1);

				// ACT
				box_r box1 = l1.get_box();

				// ASSERT
				assert_equal(26.0f, box1.h);

				// INIT
				font::ptr f2 = mocks::create_font(c_fm2, indices, glyphs);
				layout l2(L"AAAAA\nA\nA", f2);

				// ACT
				box_r box2 = l2.get_box();

				// ASSERT
				assert_equal(53.0f, box2.h);
			}

		end_test_suite
	}
}
