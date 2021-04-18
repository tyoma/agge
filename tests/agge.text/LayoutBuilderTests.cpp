#include <agge.text/layout_builder.h>

#include "helpers.h"
#include "helpers_layout.h"
#include "mocks.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			font_metrics c_fm1 = { 10.1f, 5.0f, 2.7f };
			font_metrics c_fm2 = { 11.0f, 1.3f, 7.0f };

			positioned_glyph pg(glyph_index_t index, real_t dx, real_t dy)
			{
				positioned_glyph result = {	index, create_vector(dx, dy)	};
				return result;
			}

			template <typename T>
			vector<typename T::value_type> sub(const T &container, unsigned count)
			{
				assert_is_true(count <= container.size());
				return vector<typename T::value_type>(container.begin(), container.begin() + count);
			}
		}

		begin_test_suite( LayoutBuilderTests )
			positioned_glyphs_container_t glyphs;
			glyph_runs_container_t glyph_runs;
			text_lines_container_t text_lines;
			font::ptr font1, font2;

			init( TrashContainers )
			{
				glyphs.resize(100);
				glyph_runs.resize(3, glyph_run(glyphs));
				text_lines.resize(2, text_line(glyph_runs));
				glyph_runs[0].begin_index = 10, glyph_runs[0].end_index = 30;
				glyph_runs[1].begin_index = 30, glyph_runs[1].end_index = 33;
				glyph_runs[2].begin_index = 35, glyph_runs[2].end_index = 30;
				text_lines[0].begin_index = 1, text_lines[0].end_index = 11;
				text_lines[1].begin_index = 3, text_lines[1].end_index = 5;

				mocks::font_accessor::char_to_index indices1[] = {
					{ L'A', 0 }, { L'B', 3 }, { L'C', 1 }, { 0x1219, 2 },
				}, indices2[] = {
					{ L'A', 3 }, { L'B', 4 }, { 0x9191, 2 }, { L'D', 0 },
				};
				mocks::font_accessor::glyph glyphs_[] = {
					{ { 3.4f, 0 } }, { { 5.6f, 0 } }, { { 91.9f, 0 } }, { { 14.1f, 0 } }, { { 0.9f, 0 } },
				};
				font1 = mocks::create_font(c_fm1, indices1, glyphs_);
				font2 = mocks::create_font(c_fm2, indices2, glyphs_);
			}


			test( ConstructingBuilderInitializesUnderlyingContainers )
			{
				// INIT / ACT
				layout_builder m(glyphs, glyph_runs, text_lines);

				// ASSERT
				assert_equal(0u, m.get_state().next);
				assert_equal(0.0f, m.get_state().extent);
				assert_equal(100u, glyphs.size()); // left intact
				assert_equal(plural + ref_glyph_run(font::ptr(), 0.0f, 0.0f, vector<glyph_index_t>()), glyph_runs);
				assert_equal(plural + ref_text_line(0.0f, 0.0f, 0.0f, vector<ref_glyph_run>()), text_lines);
			}


			test( AppendingGlyphsPutsThemToStorageAndIncreasesTheRunningExtentButLeavesUncommittedDataIntact )
			{
				// INIT
				layout_builder m(glyphs, glyph_runs, text_lines);

				// ACT
				m.append_glyph(1291, 12.1f);

				// ASSERT
				positioned_glyph reference1[] = {	pg(1291, 12.1f, 0.0f),	};

				assert_equal(reference1, sub(glyphs, 1));
				assert_equal(1u, m.get_state().next);
				assert_approx_equal(12.1f, m.get_state().extent, 0.001);

				// ACT
				m.append_glyph(13, 7.91f);
				m.append_glyph(191, 3.11f);

				// ASSERT
				positioned_glyph reference2[] = {	pg(1291, 12.1f, 0.0f), pg(13, 7.91f, 0.0f), pg(191, 3.11f, 0.0f),	};

				assert_equal(reference2, sub(glyphs, 3));
				assert_equal(3u, m.get_state().next);
				assert_approx_equal(23.12f, m.get_state().extent, 0.001);
				assert_equal(plural + ref_glyph_run(font::ptr(), 0.0f, 0.0f, vector<glyph_index_t>()), glyph_runs);
				assert_equal(plural + ref_text_line(0.0f, 0.0f, 0.0f, vector<ref_glyph_run>()), text_lines);
			}


			test( BeginingNewStyleWhenCurrentRunIsEmptySetsCurrentRunAttributes )
			{
				// INIT
				layout_builder m(glyphs, glyph_runs, text_lines);				

				// ACT
				m.begin_style(font1);

				// ASSERT
				assert_equal(plural + ref_glyph_run(font1, 0.0f, 0.0f, vector<glyph_index_t>()), glyph_runs);

				// ACT
				m.begin_style(font2);

				// ASSERT
				assert_equal(plural + ref_glyph_run(font2, 0.0f, 0.0f, vector<glyph_index_t>()), glyph_runs);
				assert_equal(plural + ref_text_line(0.0f, 0.0f, 0.0f, vector<ref_glyph_run>()), text_lines);
				assert_equal(0u, m.get_state().next);
				assert_equal(0.0f, m.get_state().extent);
			}


			test( BeginingNewStyleForNonEmptyRunStoresItCreatesNewRunAndSetsItsAttributes )
			{
				// INIT
				layout_builder m(glyphs, glyph_runs, text_lines);

				m.begin_style(font1);

				m.append_glyph(1, 7.91f);
				m.append_glyph(191, 3.11f);

				// ACT
				m.begin_style(font2);

				// ASSERT
				positioned_glyph reference1[] = {	pg(1, 7.91f, 0.0f), pg(191, 3.11f, 0.0f),	};

				assert_equal(plural
					+ ref_glyph_run(font1, 0.0f, 0.0f, reference1)
					+ ref_glyph_run(font2, 11.02f, 0.0f, vector<glyph_index_t>()),
					glyph_runs);
				assert_equal(2u, glyph_runs.back().begin_index);
				assert_equal(2u, m.get_state().next);
				assert_approx_equal(11.02f, m.get_state().extent, 0.001);

				// INIT
				m.append_glyph(2, 20.9f);
				m.append_glyph(3, 21.76f);
				m.append_glyph(4, 21.9f);

				// ACT
				m.begin_style(font1);

				// ASSERT
				positioned_glyph reference2[] = {	pg(2, 20.9f, 0.0f), pg(3, 21.76f, 0.0f), pg(4, 21.9f, 0.0f),	};

				assert_equal(plural
					+ ref_glyph_run(font1, 0.0f, 0.0f, reference1)
					+ ref_glyph_run(font2, 11.02f, 0.0f, reference2)
					+ ref_glyph_run(font1, 75.58f, 0.0f, vector<glyph_index_t>()),
					glyph_runs);
				assert_equal(5u, glyph_runs.back().begin_index);
				assert_equal(5u, m.get_state().next);
				assert_approx_equal(75.58f, m.get_state().extent, 0.001);

				// ACT (only sets font)
				m.begin_style(font2);

				// ASSERT
				assert_equal(plural
					+ ref_glyph_run(font1, 0.0f, 0.0f, reference1)
					+ ref_glyph_run(font2, 11.02f, 0.0f, reference2)
					+ ref_glyph_run(font2, 75.58f, 0.0f, vector<glyph_index_t>()),
					glyph_runs);
				assert_equal(plural + ref_text_line(0.0f, 0.0f, 0.0f, vector<ref_glyph_run>()), text_lines);
				assert_equal(5u, m.get_state().next);
				assert_approx_equal(75.58f, m.get_state().extent, 0.001);
			}


			test( BreakingEmptyLineOffsetsCurrentOneByLastSetFontCumulativeHeight )
			{
				// INIT
				layout_builder m(glyphs, glyph_runs, text_lines);

				m.begin_style(font1);

				// ACT / ASSERT
				assert_is_false(m.break_current_line());

				// ASSERT
				assert_equal(plural + ref_glyph_run(font1, 0.0f, 0.0f, vector<glyph_index_t>()), glyph_runs);
				assert_equal(plural + ref_text_line(0.0f, 17.8f, 0.0f, vector<ref_glyph_run>()), text_lines);
				assert_equal(0u, m.get_state().next);
				assert_equal(0.0f, m.get_state().extent);

				// INIT
				m.begin_style(font2);

				// ACT
				assert_is_false(m.break_current_line());
				assert_is_false(m.break_current_line());

				// ASSERT
				assert_equal(plural + ref_glyph_run(font2, 0.0f, 0.0f, vector<glyph_index_t>()), glyph_runs);
				assert_equal(plural + ref_text_line(0.0f, 56.4f, 0.0f, vector<ref_glyph_run>()), text_lines);
				assert_equal(0u, m.get_state().next);
				assert_equal(0.0f, m.get_state().extent);
			}


			test( BreakingNonEmptyLineBreaksCurrentRangeCommitsLineDataAndCreatesNewOne )
			{
				// INIT
				layout_builder m(glyphs, glyph_runs, text_lines);

				m.begin_style(font1);
				m.append_glyph(11, 3.0f);
				m.append_glyph(12, 5.0f);
				m.append_glyph(7, 3.1f);

				// ACT
				m.break_current_line();

				// ASSERT
				assert_equal(plural
					+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 11u + 12u + 7u)
					+ ref_glyph_run(font1, 0.0f, 0.0f, vector<glyph_index_t>()),
					glyph_runs);
				assert_equal(plural
					+ ref_text_line(0.0f, 10.1f, 11.1f, plural
						+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 11u + 12u + 7u))
					+ ref_text_line(0.0f, 17.8f, 0.0f, vector<ref_glyph_run>()),
					text_lines);
				assert_equal(5.0f, text_lines[0].descent);

				// INIT
				m.append_glyph(1, 30.7f);
				m.append_glyph(2, 10.5f);
				m.begin_style(font2);
				m.append_glyph(4, 5.5f);
				m.append_glyph(5, 1.5f);

				// ACT
				m.break_current_line();

				// ASSERT
				assert_equal(plural
					+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 11u + 12u + 7u)
					+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 1u + 2u)
					+ ref_glyph_run(font2, 41.2f, 0.0f, plural + 4u + 5u)
					+ ref_glyph_run(font2, 0.0f, 0.0f, vector<glyph_index_t>()),
					glyph_runs);
				assert_equal(plural
					+ ref_text_line(0.0f, 10.1f, 11.1f, plural
						+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 11u + 12u + 7u))
					+ ref_text_line(0.0f, 28.8f, 48.2f, plural
						+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 1u + 2u)
						+ ref_glyph_run(font2, 41.2f, 0.0f, plural + 4u + 5u))
					+ ref_text_line(0.0f, 37.1f, 0.0f, vector<ref_glyph_run>()),
					text_lines);
			}


			test( TrimmingEmptyRangeDoesNothing )
			{
				// INIT
				layout_builder m(glyphs, glyph_runs, text_lines);

				// ACT
				m.trim_current_line(m.get_state());

				// ASSERT
				assert_equal(plural + ref_glyph_run(font::ptr(), 0.0f, 0.0f, vector<glyph_index_t>()), glyph_runs);
				assert_equal(plural + ref_text_line(0.0f, 0.0f, 0.0f, vector<ref_glyph_run>()), text_lines);
			}


			test( TrimmingSingleRangeRestoresExtentAndGlyphs )
			{
				// INIT
				layout_builder m(glyphs, glyph_runs, text_lines);

				m.begin_style(font2);
				m.append_glyph(11, 3.0f);
				m.append_glyph(12, 5.3f);
				layout_builder::state s1 = m.get_state();
				m.append_glyph(13, 3.1f);

				// ACT
				m.trim_current_line(s1);

				// ASSERT
				assert_equal(plural + ref_glyph_run(font2, 0.0f, 0.0f, plural + 11u + 12u), glyph_runs);
				assert_equal(plural + ref_text_line(0.0f, 0.0f, 0.0f, vector<ref_glyph_run>()), text_lines);
				assert_approx_equal(8.3f, m.get_state().extent, 0.001);

				// ACT
				m.break_current_line();

				// ASSERT
				assert_equal(plural
					+ ref_glyph_run(font2, 0.0f, 0.0f, plural + 11u + 12u)
					+ ref_glyph_run(font2, 0.0f, 0.0f, vector<glyph_index_t>()),
					glyph_runs);
				assert_equal(plural
					+ ref_text_line(0.0f, 11.0f, 8.3f, plural
						+ ref_glyph_run(font2, 0.0f, 0.0f, plural + 11u + 12u))
					+ ref_text_line(0.0f, 19.3f, 0.0f, vector<ref_glyph_run>()),
					text_lines);

				// INIT
				m.append_glyph(14, 5.7f);
				m.begin_style(font1);
				m.append_glyph(15, 3.2f);
				m.append_glyph(16, 2.9f);
				layout_builder::state s2 = m.get_state();
				m.append_glyph(17, 3.6f);

				// ACT
				m.trim_current_line(s2);

				//ASSERT
				assert_approx_equal(11.8f, m.get_state().extent, 0.001);

				// ACT
				m.break_current_line();

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 11.0f, 8.3f, plural
						+ ref_glyph_run(font2, 0.0f, 0.0f, plural + 11u + 12u))
					+ ref_text_line(0.0f, 30.3f, 11.8f, plural
						+ ref_glyph_run(font2, 0.0f, 0.0f, plural + 14u)
						+ ref_glyph_run(font1, 5.7f, 0.0f, plural + 15u + 16u))
					+ ref_text_line(0.0f, 38.6f, 0.0f, vector<ref_glyph_run>()),
					text_lines);
			}


			test( TrimmingToAPreviousStateThrowsUnnecessaryRunsAway )
			{
				// INIT
				layout_builder m(glyphs, glyph_runs, text_lines);

				m.begin_style(font1);
				m.append_glyph(11, 3.0f);
				layout_builder::state s2 = m.get_state();
				m.append_glyph(12, 5.0f);
				m.append_glyph(13, 3.1f);
				m.begin_style(font2);
				m.append_glyph(14, 5.7f);
				m.append_glyph(15, 3.2f);
				m.append_glyph(16, 2.9f);
				layout_builder::state s1 = m.get_state();
				m.append_glyph(17, 3.6f);
				m.begin_style(font1);

				// ACT
				m.trim_current_line(s1);
				m.append_glyph(19, 10.13f);

				// ASSERT
				assert_equal(plural
					+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 11u + 12u + 13u)
					+ ref_glyph_run(font2, 11.1f, 0.0f, plural + 14u + 15u + 16u),
					glyph_runs);
				assert_approx_equal(33.03f, m.get_state().extent, 0.0001);

				// ACT
				m.begin_style(font1);

				// ASSERT
				assert_equal(plural
					+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 11u + 12u + 13u)
					+ ref_glyph_run(font2, 11.1f, 0.0f, plural + 14u + 15u + 16u + 19u)
					+ ref_glyph_run(font1, 33.03f, 0.0f, vector<glyph_index_t>()),
					glyph_runs);
				assert_approx_equal(33.03f, m.get_state().extent, 0.0001);

				// ACT
				m.append_glyph(20, 1.0f);
				m.begin_style(font2);

				// ASSERT
				assert_equal(plural
					+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 11u + 12u + 13u)
					+ ref_glyph_run(font2, 11.1f, 0.0f, plural + 14u + 15u + 16u + 19u)
					+ ref_glyph_run(font1, 33.03f, 0.0f, plural + 20u)
					+ ref_glyph_run(font2, 34.03f, 0.0f, vector<glyph_index_t>()),
					glyph_runs);
				assert_approx_equal(34.03f, m.get_state().extent, 0.0001);

				// ACT
				m.trim_current_line(s2);

				// ASSERT
				assert_equal(plural
					+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 11u),
					glyph_runs);
				assert_approx_equal(3.0f, m.get_state().extent, 0.0001);

				// ACT
				m.append_glyph(20, 1.0f);
				m.begin_style(font2);

				// ASSERT
				assert_equal(plural
					+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 11u + 20u)
					+ ref_glyph_run(font2, 4.0f, 0.0f, vector<glyph_index_t>()),
					glyph_runs);
				assert_approx_equal(4.0f, m.get_state().extent, 0.0001);
			}


			test( OnlyGlyphRunsRemainedAreCountedWhenBreakingALine )
			{
				// INIT
				layout_builder m(glyphs, glyph_runs, text_lines);

				m.begin_style(font1);
				m.append_glyph(11, 3.0f);
				m.append_glyph(13, 3.1f);
				m.begin_style(font1);
				m.append_glyph(14, 5.7f);
				layout_builder::state s1 = m.get_state();
				m.append_glyph(15, 3.2f);
				m.begin_style(font2);
				m.append_glyph(15, 3.2f);
				m.begin_style(font1);
				m.append_glyph(105, 3.2f);
				m.begin_style(font2);
				m.append_glyph(1050, 3.2f);

				// ACT
				m.trim_current_line(s1);
				m.break_current_line();

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.1f, 11.8f, plural
						+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 11u + 13u)
						+ ref_glyph_run(font1, 6.1f, 0.0f, plural + 14u))
					+ ref_text_line(0.0f, 17.8f, 0.0f, vector<ref_glyph_run>()),
					text_lines);

				// INIT
				m.append_glyph(1, 1.2f);
				m.begin_style(font1);
				m.append_glyph(2, 1.3f);
				m.begin_style(font1);
				m.append_glyph(3, 1.4f);
				layout_builder::state s2 = m.get_state();
				m.begin_style(font2);
				m.append_glyph(4, 1.5f);
				m.begin_style(font1);
				m.append_glyph(105, 3.2f);
				m.begin_style(font1);
				m.append_glyph(105, 3.2f);
				m.begin_style(font1);
				m.append_glyph(105, 3.2f);

				// ACT
				m.trim_current_line(s2);
				m.break_current_line();

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.1f, 11.8f, plural
						+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 11u + 13u)
						+ ref_glyph_run(font1, 6.1f, 0.0f, plural + 14u))
					+ ref_text_line(0.0f, 27.9f, 3.9f, plural
						+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 1u)
						+ ref_glyph_run(font1, 1.2f, 0.0f, plural + 2u)
						+ ref_glyph_run(font1, 2.5f, 0.0f, plural + 3u))
					+ ref_text_line(0.0f, 35.6f, 0.0f, vector<ref_glyph_run>()),
					text_lines);
			}


			test( OnlyNonEmptyGlyphRunsRemainedAreCountedWhenBreakingALine )
			{
				// INIT
				layout_builder m(glyphs, glyph_runs, text_lines);

				m.begin_style(font1);
				m.append_glyph(11, 3.0f);
				m.append_glyph(13, 3.1f);
				m.begin_style(font1);
				m.append_glyph(14, 5.7f);
				m.append_glyph(15, 3.2f);
				m.begin_style(font2);
				layout_builder::state s = m.get_state();
				m.append_glyph(100, 5.7f);
				m.append_glyph(100, 3.2f);

				// ACT
				m.trim_current_line(s);
				m.break_current_line();
				m.append_glyph(110, 3.0f);
				m.break_current_line();

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.1f, 15.0f, plural
						+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 11u + 13u)
						+ ref_glyph_run(font1, 6.1f, 0.0f, plural + 14u + 15u))
					+ ref_text_line(0.0f, 28.8f, 3.0f, plural
						+ ref_glyph_run(font2, 0.0f, 0.0f, plural + 110u))
					+ ref_text_line(0.0f, 37.1f, 0.0f, vector<ref_glyph_run>()),
					text_lines);

				// ACT (implicit height is set to the last font)
				m.break_current_line();

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.1f, 15.0f, plural
						+ ref_glyph_run(font1, 0.0f, 0.0f, plural + 11u + 13u)
						+ ref_glyph_run(font1, 6.1f, 0.0f, plural + 14u + 15u))
					+ ref_text_line(0.0f, 28.8f, 3.0f, plural
						+ ref_glyph_run(font2, 0.0f, 0.0f, plural + 110u))
					+ ref_text_line(0.0f, 56.4f, 0.0f, vector<ref_glyph_run>()),
					text_lines);
			}


			test( LayoutBuilderReturnsExtentsAccordinglyToCodepoints )
			{
				// INIT
				layout_builder b(glyphs, glyph_runs, text_lines);

				b.begin_style(font1);

				// ACT / ASSERT
				assert_equal(make_pair(0u, 3.4f), b.current_glyph(L'A'));
				assert_equal(make_pair(3u, 14.1f), b.current_glyph(L'B'));
				assert_equal(make_pair(1u, 5.6f), b.current_glyph(L'C'));
				assert_equal(make_pair(2u, 91.9f), b.current_glyph(0x1219));

				// INIT
				b.begin_style(font2);

				// ACT / ASSERT
				assert_equal(make_pair(3u, 14.1f), b.current_glyph(L'A'));
				assert_equal(make_pair(4u, 0.9f), b.current_glyph(L'B'));
				assert_equal(make_pair(2u, 91.9f), b.current_glyph(0x9191));
				assert_equal(make_pair(0u, 3.4f), b.current_glyph(L'D'));
			}

		end_test_suite
	}
}
