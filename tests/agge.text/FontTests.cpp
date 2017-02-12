#include <agge.text/font.h>

#include "mocks.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		begin_test_suite( FontTests )
			test( MissingGlyphIsReportedAsNull )
			{
				// INIT
				mocks::font::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font::glyph glyphs[] = { { { 11, 0 } }, };
				mocks::font f(indices, glyphs);

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
				mocks::font f(indices, glyphs);

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
				auto_ptr<mocks::font> f(new mocks::font(indices, glyphs, &alive));

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
		end_test_suite
	}
}
