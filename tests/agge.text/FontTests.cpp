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
			font_metrics c_fm1 = { 10.0f, 2.0f, 2.0f };
		}

		begin_test_suite( FontTests )
			test( MissingGlyphIsReportedAsNull )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { { { 11, 0 } }, };
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);

				// ACT / ASSERT
				assert_null(f->get_glyph(2));
				assert_null(f->get_glyph(10111));
				assert_null(f->get_glyph(1));
			}


			test( SameGlyphIsReturnedOnConsequentCalls )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { { { 11, 0 } }, { { 11, 0 } }, { { 11, 0 } }, };
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);

				// ACT
				const glyph *g1 = f->get_glyph(0);
				const glyph *g2 = f->get_glyph(1);
				const glyph *g3 = f->get_glyph(2);
				const glyph *g4 = f->get_glyph(1);
				const glyph *g5 = f->get_glyph(0);
				const glyph *g6 = f->get_glyph(1);
				const glyph *g7 = f->get_glyph(2);

				// ASSERT
				assert_equal(g1, g5);
				assert_equal(g2, g4);
				assert_equal(g2, g6);
				assert_equal(g3, g7);
			}


			test( GlyphsReturnIteratableOutlinePaths )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, };
				glyph::path_point outline0[] = {
					{ 1, 1.1f, 2.4f }, { 2, 4.1f, 7.5f }, { 3, 3.6f, 1.3f }, { 4, 3.6f, 1.3f }
				};
				glyph::path_point outline1[] = { { 2110, 4.1f, 7.5f }, { 13, 3.6f, 1.3f }, { 7, 3.6f, 1.3f } };
				mocks::font_accessor::glyph glyphs[] = { mocks::glyph(11.8, 19.1, outline0), mocks::glyph(12.3, 13.5, outline1), };
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);

				// ACT
				const glyph *g1 = f->get_glyph(0);
				pod_vector<glyph::path_point> points1 = convert_copy(g1->get_outline());
				const glyph *g2 = f->get_glyph(1);
				pod_vector<glyph::path_point> points2 = convert_copy(g2->get_outline());

				// ASSERT
				assert_equal(outline0, points1);
				assert_equal(outline1, points2);
			}


			test( GlyphsOutlineIteratorIsRewindable )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, };
				glyph::path_point outline[] = {
					{ 1, 1.1f, 2.4f }, { 2, 4.1f, 7.5f }, { 3, 3.6f, 1.3f }, { 4, 3.6f, 1.3f }
				};
				mocks::font_accessor::glyph glyphs[] = { mocks::glyph(1.1, 1.1, outline), };
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);

				// ACT
				const glyph *g = f->get_glyph(0);
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
				mocks::font_accessor::char_to_index indices[] = {
					{ L'A', 0 }, { L'b', 13 }, { L'h', 1130 }, { L'!', 7 }, { L'"', 19 },
				};
				mocks::font_accessor::glyph glyphs[] = { { { 0, 0 } }, };
				font::ptr f = mocks::create_font(c_fm1, indices, glyphs);

				// ACT / ASSERT
				assert_equal(0u, f->map_single(L'A'));
				assert_equal(13u, f->map_single(L'b'));
				assert_equal(1130u, f->map_single(L'h'));
				assert_equal(7u, f->map_single(L'!'));
				assert_equal(19u, f->map_single(L'"'));
			}


			test( SingleGlyphMatchingCachesUnderlyingResult )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = {
					{ L'a', 19 }, { L'B', 13111 },
				};
				mocks::font_accessor::glyph glyphs[] = { { { 0, 0 } }, };
				shared_ptr<mocks::font_accessor> a(new mocks::font_accessor(c_fm1, indices, glyphs));
				font::ptr f(new font(a));

				// INIT / ACT (caching occurs here)
				f->map_single(L'a');
				f->map_single(L'B');
				a->glyph_mapping_calls = 0;

				// ACT / ASSERT
				assert_equal(19u, f->map_single(L'a'));
				assert_equal(13111u, f->map_single(L'B'));
				assert_equal(0, a->glyph_mapping_calls);
			}


			test( FontProvidesItsAccessorDescriptor )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = {
					{ L'a', 19 }, { L'B', 13111 },
				};
				mocks::font_accessor::glyph glyphs[] = { { { 0, 0 } }, };
				shared_ptr<mocks::font_accessor> a(new mocks::font_accessor(c_fm1, indices, glyphs));

				a->descriptor = font_descriptor::create("Tahoma", 10, true, false, hint_strong);

				// INIT / ACT
				font f(a);

				// ACT / ASSERT
				assert_equal(a->descriptor, f.get_key());

				// INIT
				a->descriptor = font_descriptor::create("Verdana", 13, false, true, hint_none);

				// ACT / ASSERT
				assert_equal(a->descriptor, f.get_key());
			}

		end_test_suite
	}
}
