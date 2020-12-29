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

			class ref_glyph_run
			{
			public:
				template <size_t n>
				ref_glyph_run(shared_ptr<font> font_, real_t offset_x, real_t offset_y, glyph_index_t (&indices)[n])
					: _font(font_), _offset(create_vector(offset_x, offset_y)), _check_glyph_advances(false)
				{
					for (size_t i = 0; i != n; ++i)
					{
						positioned_glyph g = {	create_vector(0.0f, 0.0f), indices[i]	};
						_glyphs.push_back(g);
					}
				}

				template <typename T>
				ref_glyph_run(shared_ptr<font> font_, real_t offset_x, real_t offset_y, const vector<T> &indices)
					: _font(font_), _offset(create_vector(offset_x, offset_y)), _check_glyph_advances(false)
				{
					for (typename vector<T>::const_iterator i = indices.begin(); i != indices.end(); ++i)
					{
						positioned_glyph g = {	create_vector(0.0f, 0.0f), static_cast<glyph_index_t>(*i)	};
						_glyphs.push_back(g);
					}
				}

				template <size_t n>
				ref_glyph_run(shared_ptr<font> font_, real_t offset_x, real_t offset_y,
						positioned_glyph (&positioned)[n])
					: _font(font_), _offset(create_vector(offset_x, offset_y)), _glyphs(mkvector(begin(positioned),
						end(positioned))), _check_glyph_advances(true)
				{	}

				bool operator ==(const glyph_run &rhs) const
				{
					if (_font == rhs.glyph_run_font && _offset == rhs.offset)
					{
						vector<positioned_glyph>::const_iterator i = _glyphs.begin();
						positioned_glyphs_container_t::const_iterator j = rhs.begin();

						for (; i != _glyphs.end() && j != rhs.end(); ++i, ++j)
							if (i->index != j->index || (_check_glyph_advances && !(i->d == j->d)))
								return false;
						return true;
					}
					return false;
				}

			private:
				shared_ptr<font> _font;
				vector_r _offset;
				vector<positioned_glyph> _glyphs;
				bool _check_glyph_advances;
			};

			class ref_text_line
			{
			public:
				ref_text_line(real_t offset_x, real_t offset_y, real_t width, const vector<ref_glyph_run> &glyph_runs)
					: _offset(create_vector(offset_x, offset_y)), _width(width), _glyph_runs(glyph_runs)
				{	}

				bool operator ==(const text_line &rhs) const
				{
					if (_offset == rhs.offset && (!_width || tests::equal(_width, rhs.width)))
					{
						vector<ref_glyph_run>::const_iterator i = _glyph_runs.begin();
						glyph_runs_container_t::const_iterator j = rhs.begin();

						for (; i != _glyph_runs.end() && j != rhs.end(); ++i, ++j)
							if (!(*i == *j))
								return false;
						return true;
					}
					return false;
				}

			private:
				vector_r _offset;
				real_t _width;
				vector<ref_glyph_run> _glyph_runs;
			};
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
				font::ptr f2 = mocks::create_font(c_fm2, indices2, glyphs);
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

				assert_equal(plural + ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(f1, 0.0f, 0.0f, reference1)),
					mkvector(l1.begin(), l1.end()));

				assert_equal(plural + ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(f1, 0.0f, 0.0f, reference2)),
					mkvector(l2.begin(), l2.end()));

				assert_equal(plural + ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(f1, 0.0f, 0.0f, reference3)),
					mkvector(l3.begin(), l3.end()));

				assert_equal(plural + ref_text_line(0.0f, 14.0f, 0.0f, plural + ref_glyph_run(f2, 0.0f, 0.0f, reference4)),
					mkvector(l4.begin(), l4.end()));

				assert_equal(plural + ref_text_line(0.0f, 14.0f, 0.0f, plural + ref_glyph_run(f2, 0.0f, 0.0f, reference5)),
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

				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 120.2f, plural + ref_glyph_run(f1, 0.0f, 0.0f, reference11))
					+ ref_text_line(0.0f, 24.0f, 87.1f, plural + ref_glyph_run(f1, 0.0f, 0.0f, reference12)),
					mkvector(l1.begin(), l1.end()));


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

				assert_equal(plural
					+ ref_text_line(0.0f, 14.0f, 65.1f, plural + ref_glyph_run(f2, 0.0f, 0.0f, reference21))
					+ ref_text_line(0.0f, 32.0f, 42.1f, plural + ref_glyph_run(f2, 0.0f, 0.0f, reference22))
					+ ref_text_line(0.0f, 50.0f, 87.1f, plural + ref_glyph_run(f2, 0.0f, 0.0f, reference23)),
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
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);

				// ACT
				layout l(f);

				l.process(L"ABC CBA AB\n\n\nABB BBC\n\n");

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 1 + 2 + 3 + 0 + 3 + 2 + 1 + 0 + 1 + 2))
					+ ref_text_line(0.0f, 52.0f, 0.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 1 + 2 + 2 + 0 + 2 + 2 + 3)),
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
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 132.2f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 1 + 1 + 1 + 1 + 0 + 2 + 2 + 2 + 2 + 0 + 3 + 3))
					+ ref_text_line(0.0f, 24.0f, 99.1f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 2 + 2 + 2 + 2 + 0 + 1 + 1 + 1 + 1)),
					mkvector(l1.begin(), l1.end()));

				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 127.2f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 3 + 3 + 3 + 4 + 3 + 0 + 2 + 2 + 2 + 0 + 1 + 1))
					+ ref_text_line(0.0f, 24.0f, 109.1f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 1 + 1 + 0 + 1 + 1 + 1 + 1 + 2 + 2 + 2))
					+ ref_text_line(0.0f, 38.0f, 103.1f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 3 + 3 + 3 +3 + 0 + 1 + 1 + 1 + 1))
					+ ref_text_line(0.0f, 52.0f, 118.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 5)),
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
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);
				layout::const_iterator gr;
				layout l(f);

				// ACT
				l.set_width_limit(6);
				l.process(L"ABCABCABC");

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 6.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 0 + 1 + 2))
					+ ref_text_line(0.0f, 24.0f, 6.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 0 + 1 + 2))
					+ ref_text_line(0.0f, 38.0f, 6.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 0 + 1 + 2)),
					mkvector(l.begin(), l.end()));

				// ACT
				l.set_width_limit(8);
				l.process(L"ABCABCABC");

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 7.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 0 + 1 + 2 + 0))
					+ ref_text_line(0.0f, 24.0f, 8.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 1 + 2 + 0 + 1))
					+ ref_text_line(0.0f, 38.0f, 3.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 2)),
					mkvector(l.begin(), l.end()));
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
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 0 + 0 + 0 + 0 + 0))
					+ ref_text_line(0.0f, 24.0f, 0.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 0 + 0 + 0 + 0))
					+ ref_text_line(0.0f, 38.0f, 0.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 0 + 0)),
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
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);
				layout l(f);

				l.set_width_limit(26.0f);

				// ACT
				l.process(L"AAAAA   A   A");

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 1 + 1 + 1 + 1 + 1))
					+ ref_text_line(0.0f, 24.0f, 0.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 1 + 0 + 0 + 0 + 1)),
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
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);
				layout l(f);

				l.set_width_limit(60.0f);

				// ACT
				// 36 + 3 + 61
				// ABCD|ABCDCD[]C
				l.process(L"ABCD ABCDCDC");

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 1 + 2 + 3 + 4))
					+ ref_text_line(0.0f, 24.0f, 0.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 1 + 2 + 3 + 4 + 3 + 4))
					+ ref_text_line(0.0f, 38.0f, 0.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 3)),
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
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);
				layout l(f);

				l.set_width_limit(38.0f);

				// ACT
				// 36 + 3 + 61
				// ABCD|ABCD[]CDC
				l.process(L"ABCD ABCDCDC");

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 1 + 2 + 3 + 4))
					+ ref_text_line(0.0f, 24.0f, 0.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 1 + 2 + 3 + 4))
					+ ref_text_line(0.0f, 38.0f, 0.0f, plural + ref_glyph_run(f, 0.0f, 0.0f, plural + 3 + 4 + 3)),
					mkvector(l.begin(), l.end()));
			}
		end_test_suite
	}
}
