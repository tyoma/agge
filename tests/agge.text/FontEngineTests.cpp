#include <agge.text/font_engine.h>

#include "helpers.h"
#include "mocks.h"

#include <algorithm>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			font::metrics c_fm1 = { 1.1f, 2.2f, 3.3f };
			font::metrics c_fm2 = { 4.4f, 5.6f, 7.1f };
		}

		begin_test_suite( FontEngineTests )
			
			test( CreatingDifferentFontsReturnsDifferentObjects )
			{
				// INIT
				mocks::fonts_loader loader;
				font_engine e(loader);

				// ACT
				font::ptr fonts[] = {
					e.create_font(L"arial", 13, false, false, font_engine::gf_strong),
					e.create_font(L"helvetica", 13, false, false, font_engine::gf_strong),
					e.create_font(L"tahoma", 13, false, false, font_engine::gf_strong),
					e.create_font(L"tahoma", 14, false, false, font_engine::gf_strong),
					e.create_font(L"helvetica", 13, true, false, font_engine::gf_strong),
					e.create_font(L"helvetica", 13, false, true, font_engine::gf_strong),
					e.create_font(L"helvetica", 13, true, true, font_engine::gf_strong),
					e.create_font(L"helvetica", 13, true, true, font_engine::gf_vertical),
					e.create_font(L"helvetica", 13, true, true, font_engine::gf_none),
				};

				// ASSERT
				sort(tests::begin(fonts), tests::end(fonts));

				assert_equal(tests::end(fonts), unique(tests::begin(fonts), tests::end(fonts)));
			}


			test( CreatingTheSameFontReturnsTheSameObject )
			{
				// INIT
				mocks::fonts_loader loader;
				font_engine e(loader);

				// ACT
				font::ptr fonts[] = {
					e.create_font(L"arial", 13, false, false, font_engine::gf_strong),
					e.create_font(L"ARial", 13, false, false, font_engine::gf_strong),
					e.create_font(L"arial", 15, false, false, font_engine::gf_vertical),
					e.create_font(L"Arial", 15, false, false, font_engine::gf_vertical),
					e.create_font(L"arial", 13, false, true, font_engine::gf_none),
					e.create_font(L"arial", 13, false, true, font_engine::gf_none),
					e.create_font(L"tahoma", 143, true, false, font_engine::gf_strong),
					e.create_font(L"taHOMA", 143, true, false, font_engine::gf_strong),
				};

				// ASSERT
				assert_equal(fonts[0], fonts[1]);
				assert_equal(fonts[2], fonts[3]);
				assert_equal(fonts[4], fonts[5]);
				assert_equal(fonts[6], fonts[7]);

				// ACT
				font::ptr f2 = e.create_font(L"arial", 13, false, false, font_engine::gf_strong);

				// ASSERT
				assert_equal(fonts[0], f2);
			}


			test( CreatingTheFontCreatesFontAccessor )
			{
				// INIT
				mocks::fonts_loader loader;
				font_engine e(loader);

				// ACT
				font::ptr fonts1[] = {
					e.create_font(L"arial", 13, false, false, font_engine::gf_strong),
					e.create_font(L"tahoma", 29, true, false, font_engine::gf_vertical),
					e.create_font(L"helvetica", 15, false, true, font_engine::gf_none /* freely scalable */),
				};
				fonts1;

				// ASSERT
				mocks::font_descriptor fd1[] = {
					mocks::font_descriptor(L"arial", 13, false, false, font_engine::gf_strong),
					mocks::font_descriptor(L"tahoma", 29, true, false, font_engine::gf_vertical),
					mocks::font_descriptor(L"helvetica", 1000, false, true, font_engine::gf_none),
				};

				assert_equal(fd1, loader.created_log);

				// INIT
				loader.created_log.clear();

				// ACT
				font::ptr fonts2[] = {
					e.create_font(L"arial", 15, true, true, font_engine::gf_none /* freely scalable */),
				};
				fonts2;

				// ASSERT
				mocks::font_descriptor fd2[] = {
					mocks::font_descriptor(L"arial", 1000, true, true, font_engine::gf_none),
				};

				assert_equal(fd2, loader.created_log);
			}


			test( ScalableFontAccessorIsCreatedOnceForDifferentFontSizes )
			{
				// INIT
				mocks::fonts_loader loader;
				font_engine e(loader);

				// ACT
				font::ptr fonts1[] = {
					e.create_font(L"arial", 13, false, true, font_engine::gf_none),
					e.create_font(L"arial", 29, false, true, font_engine::gf_none),
					e.create_font(L"arial", 140, false, true, font_engine::gf_none),
				};
				fonts1;

				// ASSERT
				mocks::font_descriptor fd1[] = {
					mocks::font_descriptor(L"arial", 1000, false, true, font_engine::gf_none),
				};

				assert_equal(fd1, loader.created_log);

				// ACT
				font::ptr fonts2[] = {
					e.create_font(L"times", 31, true, false, font_engine::gf_none),
					e.create_font(L"times", 91, true, false, font_engine::gf_none),
				};
				fonts2;

				// ASSERT
				mocks::font_descriptor fd2[] = {
					mocks::font_descriptor(L"arial", 1000, false, true, font_engine::gf_none),
					mocks::font_descriptor(L"times", 1000, true, false, font_engine::gf_none),
				};

				assert_equal(fd2, loader.created_log);
			}


			test( FontCreatedReceivesTheAccessorAndTheScalingNecessary )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, };
				glyph::path_point outline1[] = { { 1, 1.1f, 2.4f }, { 2, 4.1f, 7.5f }, };
				glyph::path_point outline2[] = { { 3, 1.1f, 4.2f }, { 7, 1.4f, 5.7f }, };
				mocks::font_accessor::glyph glyphs1[] = {
					mocks::glyph(1.1, 1.2, outline1), mocks::glyph(1.3, 1.4, outline2),
				};
				mocks::font_accessor::glyph glyphs2[] = {
					mocks::glyph(2.1, 2.2, outline2), mocks::glyph(3.3, 3.4, outline1),
				};
				pair<mocks::font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(mocks::font_descriptor(L"Tahoma", 1000, false, false, font_engine::gf_none),
						mocks::font_accessor(c_fm1, indices, glyphs1)),
					make_pair(mocks::font_descriptor(L"Verdana", 1000, true, false, font_engine::gf_none),
						mocks::font_accessor(c_fm2, indices, glyphs2)),
				};
				mocks::fonts_loader loader(fonts);
				font_engine e(loader);

				// ACT
				font::ptr f = e.create_font(L"Tahoma", 17, false, false, font_engine::gf_none);
				const glyph *g1 = f->get_glyph(0);
				pod_vector<glyph::path_point> o1 = convert_copy(g1->get_outline());
				const glyph *g2 = f->get_glyph(1);
				pod_vector<glyph::path_point> o2 = convert_copy(g2->get_outline());

				// ASSERT
				assert_equal(0.017 * c_fm1, f->get_metrics());
				assert_is_true(equal(0.017f * 1.1f, g1->metrics.advance_x));
				assert_is_true(equal(0.017f * 1.2f, g1->metrics.advance_y));
				assert_equal(outline1, (1 / 0.017) * o1);
				assert_is_true(equal(0.017f * 1.3f, g2->metrics.advance_x));
				assert_is_true(equal(0.017f * 1.4f, g2->metrics.advance_y));
				assert_equal(outline2, (1 / 0.017) * o2);

				// ACT
				f = e.create_font(L"Verdana", 710, true, false, font_engine::gf_none);
				g1 = f->get_glyph(0);
				pod_vector<glyph::path_point> o3 = convert_copy(g1->get_outline());
				g2 = f->get_glyph(1);
				pod_vector<glyph::path_point> o4 = convert_copy(g2->get_outline());

				// ASSERT
				assert_equal(0.71 * c_fm2, f->get_metrics());
				assert_equal(outline2, (1 / 0.71) * o3);
				assert_equal(outline1, (1 / 0.71) * o4);
			}

		end_test_suite
	}
}
