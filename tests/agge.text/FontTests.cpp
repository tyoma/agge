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
				assert_null(f.get_glyph(2));
				assert_null(f.get_glyph(10111));
				assert_null(f.get_glyph(1));
			}


			test( SameGlyphIsReturnedOnConsequentCalls )
			{
				// INIT
				mocks::font::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font::glyph glyphs[] = { { { 11, 0 } }, { { 11, 0 } }, { { 11, 0 } }, };
				mocks::font f(c_fm1, indices, glyphs);

				// ACT
				const glyph *g1 = f.get_glyph(0);
				const glyph *g2 = f.get_glyph(1);
				const glyph *g3 = f.get_glyph(2);
				const glyph *g4 = f.get_glyph(1);
				const glyph *g5 = f.get_glyph(0);
				const glyph *g6 = f.get_glyph(1);
				const glyph *g7 = f.get_glyph(2);

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
				mocks::font::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font::glyph glyphs[] = { { { 11, 0 } }, { { 11, 0 } }, { { 11, 0 } }, };
				auto_ptr<mocks::font> f(new mocks::font(c_fm1, indices, glyphs, &alive));

				// ACT
				f->get_glyph(0);
				f->get_glyph(1);

				// ASSERT
				assert_equal(2u, alive);

				// ACT
				f->get_glyph(2);

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
				mocks::font::char_to_index indices[] = { { L'A', 0 }, };
				glyph::path_point outline0[] = {
					{ 1, 1.1f, 2.4f }, { 2, 4.1f, 7.5f }, { 3, 3.6f, 1.3f }, { 4, 3.6f, 1.3f }
				};
				glyph::path_point outline1[] = { { 2110, 4.1f, 7.5f }, { 13, 3.6f, 1.3f }, { 7, 3.6f, 1.3f } };
				mocks::font::glyph glyphs[] = { mocks::glyph(11.8, 19.1, outline0), mocks::glyph(12.3, 13.5, outline1), };
				mocks::font f(c_fm1, indices, glyphs);

				// ACT
				const glyph *g1 = f.get_glyph(0);
				pod_vector<glyph::path_point> points1 = convert_copy(g1->get_outline());
				const glyph *g2 = f.get_glyph(1);
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
				const glyph *g = f.get_glyph(0);
				glyph::path_iterator i = g->get_outline();
				pod_vector<glyph::path_point> points1 = convert(i);
				i.rewind(0);
				pod_vector<glyph::path_point> points2 = convert(i);

				// ASSERT
				assert_equal(outline, points1);
				assert_equal(outline, points2);
			}


			test( SingleGlyphMatchingIsMadeAccordinglyMappingSupplied )
			{
				// INIT
				mocks::font::char_to_index indices[] = {
					{ L'A', 0 }, { L'b', 13 }, { L'h', 1130 }, { L'!', 7 }, { L'"', 19 },
				};
				mocks::font::glyph glyphs[] = { { 0, 0 }, };
				mocks::font f(c_fm1, indices, glyphs);

				// ACT / ASSERT
				assert_equal(0u, f.map_single(L'A'));
				assert_equal(13u, f.map_single(L'b'));
				assert_equal(1130u, f.map_single(L'h'));
				assert_equal(7u, f.map_single(L'!'));
				assert_equal(19u, f.map_single(L'"'));
			}


			test( SingleGlyphMatchingCachesUnderlyingResult )
			{
				// INIT
				mocks::font::char_to_index indices[] = {
					{ L'a', 19 }, { L'B', 13111 },
				};
				mocks::font::glyph glyphs[] = { { 0, 0 }, };
				mocks::font f(c_fm1, indices, glyphs);

				// INIT / ACT (caching occurs here)
				f.map_single(L'a');
				f.map_single(L'B');
				f.glyph_mapping_calls = 0;

				// ACT / ASSERT
				assert_equal(19u, f.map_single(L'a'));
				assert_equal(13111u, f.map_single(L'B'));
				assert_equal(0, f.glyph_mapping_calls);
			}

		end_test_suite
	}
}
