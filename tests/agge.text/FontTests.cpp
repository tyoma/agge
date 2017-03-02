#include <agge.text/font.h>

#include "helpers.h"
#include "mocks.h"

#include <agge/path.h>
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

			template <typename IteratorT>
			pod_vector<glyph::path_point> convert(IteratorT &i)
			{
				real_t x, y;
				pod_vector<glyph::path_point> result;

				for (int command; command = i.vertex(&x, &y), path_command_stop != command; )
				{
					glyph::path_point p = { command, x, y };
					result.push_back(p);
				}
				return result;
			}

			template <typename IteratorT>
			pod_vector<glyph::path_point> convert_copy(IteratorT i)
			{	return convert(i);	}
		}

		begin_test_suite( FontTests )
			test( MissingGlyphIsReportedAsNull )
			{
				// INIT
				mocks::font::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font::glyph glyphs[] = { { { 11, 0 } }, };
				mocks::font f(c_fm1, indices, glyphs);

				// ACT / ASSERT
				assert_null(f.get_glyph(L'B'));
				assert_null(f.get_glyph(L'C'));
				assert_null(f.get_glyph(L' '));
			}


			test( SameGlyphIsReturnedOnConsequentCalls )
			{
				// INIT
				mocks::font::char_to_index indices[] = { { L'A', 0 }, { L'B', 1 }, { L'!', 2 }, };
				mocks::font::glyph glyphs[] = { { { 11, 0 } }, { { 11, 0 } }, { { 11, 0 } }, };
				mocks::font f(c_fm1, indices, glyphs);

				// ACT
				const glyph *g1 = f.get_glyph(L'A');
				const glyph *g2 = f.get_glyph(L'B');
				const glyph *g3 = f.get_glyph(L'!');
				const glyph *g4 = f.get_glyph(L'B');
				const glyph *g5 = f.get_glyph(L'A');
				const glyph *g6 = f.get_glyph(L'B');
				const glyph *g7 = f.get_glyph(L'!');

				// ASSERT
				assert_equal(g1, g5);
				assert_equal(g2, g4);
				assert_equal(g2, g6);
				assert_equal(g3, g7);
			}


			test( GlyphsAreReleasedOnFontDestruction )
			{
				// INIT
				size_t alive = 0;
				mocks::font::char_to_index indices[] = { { L'A', 0 }, { L'B', 1 }, { L'C', 2 }, };
				mocks::font::glyph glyphs[] = { { { 11, 0 } }, { { 11, 0 } }, { { 11, 0 } }, };
				auto_ptr<mocks::font> f(new mocks::font(c_fm1, indices, glyphs, &alive));

				// ACT
				f->get_glyph(L'A');
				f->get_glyph(L'B');

				// ASSERT
				assert_equal(2u, alive);

				// ACT
				f->get_glyph(L'C');

				// ASSERT
				assert_equal(3u, alive);

				// ACT
				f.reset();

				// ASSERT
				assert_equal(0u, alive);
			}


			test( GlyphsReturnIteratableOutlinePaths )
			{
				// INIT
				mocks::font::char_to_index indices[] = { { L'A', 0 }, { L'B', 1 }, };
				glyph::path_point outline0[] = {
					{ 1, 1.1f, 2.4f }, { 2, 4.1f, 7.5f }, { 3, 3.6f, 1.3f }, { 4, 3.6f, 1.3f }
				};
				glyph::path_point outline1[] = { { 2110, 4.1f, 7.5f }, { 13, 3.6f, 1.3f }, { 7, 3.6f, 1.3f } };
				mocks::font::glyph glyphs[] = { mocks::glyph(11.8, 19.1, outline0), mocks::glyph(12.3, 13.5, outline1), };
				mocks::font f(c_fm1, indices, glyphs);

				// ACT
				const glyph *g1 = f.get_glyph(L'A');
				pod_vector<glyph::path_point> points1 = convert_copy(g1->get_outline());
				const glyph *g2 = f.get_glyph(L'B');
				pod_vector<glyph::path_point> points2 = convert_copy(g2->get_outline());

				// ASSERT
				assert_equal(outline0, points1);
				assert_equal(outline1, points2);
			}


			test( GlyphsOutlineIteratorIsRewindable )
			{
				// INIT
				mocks::font::char_to_index indices[] = { { L'A', 0 }, };
				glyph::path_point outline[] = {
					{ 1, 1.1f, 2.4f }, { 2, 4.1f, 7.5f }, { 3, 3.6f, 1.3f }, { 4, 3.6f, 1.3f }
				};
				mocks::font::glyph glyphs[] = { mocks::glyph(1.1, 1.1, outline), };
				mocks::font f(c_fm1, indices, glyphs);

				// ACT
				const glyph *g = f.get_glyph(L'A');
				glyph::path_iterator i = g->get_outline();
				pod_vector<glyph::path_point> points1 = convert(i);
				i.rewind(0);
				pod_vector<glyph::path_point> points2 = convert(i);

				// ASSERT
				assert_equal(outline, points1);
				assert_equal(outline, points2);
			}

		end_test_suite
	}
}
