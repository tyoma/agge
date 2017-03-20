#include <agge.text/font_engine.h>

#include "helpers.h"
#include "mocks.h"
#include "outlines.h"

#include <agge/path.h>
#include <algorithm>
#include <tests/common/helpers.h>
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
				font_engine_base e(loader);

				// ACT
				font::ptr fonts[] = {
					e.create_font(L"arial", 13, false, false, font_engine_base::gf_strong),
					e.create_font(L"helvetica", 13, false, false, font_engine_base::gf_strong),
					e.create_font(L"tahoma", 13, false, false, font_engine_base::gf_strong),
					e.create_font(L"tahoma", 14, false, false, font_engine_base::gf_strong),
					e.create_font(L"helvetica", 13, true, false, font_engine_base::gf_strong),
					e.create_font(L"helvetica", 13, false, true, font_engine_base::gf_strong),
					e.create_font(L"helvetica", 13, true, true, font_engine_base::gf_strong),
					e.create_font(L"helvetica", 13, true, true, font_engine_base::gf_vertical),
					e.create_font(L"helvetica", 13, true, true, font_engine_base::gf_none),
				};

				// ASSERT
				sort(tests::begin(fonts), tests::end(fonts));

				assert_equal(tests::end(fonts), unique(tests::begin(fonts), tests::end(fonts)));
			}


			test( CreatingTheSameFontReturnsTheSameObject )
			{
				// INIT
				mocks::fonts_loader loader;
				font_engine_base e(loader);

				// ACT
				font::ptr fonts[] = {
					e.create_font(L"arial", 13, false, false, font_engine_base::gf_strong),
					e.create_font(L"ARial", 13, false, false, font_engine_base::gf_strong),
					e.create_font(L"arial", 15, false, false, font_engine_base::gf_vertical),
					e.create_font(L"Arial", 15, false, false, font_engine_base::gf_vertical),
					e.create_font(L"arial", 13, false, true, font_engine_base::gf_none),
					e.create_font(L"arial", 13, false, true, font_engine_base::gf_none),
					e.create_font(L"tahoma", 143, true, false, font_engine_base::gf_strong),
					e.create_font(L"taHOMA", 143, true, false, font_engine_base::gf_strong),
				};

				// ASSERT
				assert_equal(fonts[0], fonts[1]);
				assert_equal(fonts[2], fonts[3]);
				assert_equal(fonts[4], fonts[5]);
				assert_equal(fonts[6], fonts[7]);

				// ACT
				font::ptr f2 = e.create_font(L"arial", 13, false, false, font_engine_base::gf_strong);

				// ASSERT
				assert_equal(fonts[0], f2);
			}


			test( CreatingTheFontCreatesFontAccessor )
			{
				// INIT
				mocks::fonts_loader loader;
				font_engine_base e(loader);

				// ACT
				font::ptr fonts1[] = {
					e.create_font(L"arial", 13, false, false, font_engine_base::gf_strong),
					e.create_font(L"tahoma", 29, true, false, font_engine_base::gf_vertical),
					e.create_font(L"helvetica", 15, false, true, font_engine_base::gf_none /* freely scalable */),
				};
				fonts1;

				// ASSERT
				mocks::font_descriptor fd1[] = {
					mocks::font_descriptor(L"arial", 13, false, false, font_engine_base::gf_strong),
					mocks::font_descriptor(L"tahoma", 29, true, false, font_engine_base::gf_vertical),
					mocks::font_descriptor(L"helvetica", 1000, false, true, font_engine_base::gf_none),
				};

				assert_equal(fd1, loader.created_log);

				// INIT
				loader.created_log.clear();

				// ACT
				font::ptr fonts2[] = {
					e.create_font(L"arial", 15, true, true, font_engine_base::gf_none /* freely scalable */),
				};
				fonts2;

				// ASSERT
				mocks::font_descriptor fd2[] = {
					mocks::font_descriptor(L"arial", 1000, true, true, font_engine_base::gf_none),
				};

				assert_equal(fd2, loader.created_log);
			}


			test( ScalableFontAccessorIsCreatedOnceForDifferentFontSizes )
			{
				// INIT
				mocks::fonts_loader loader;
				font_engine_base e(loader);

				// ACT
				font::ptr fonts1[] = {
					e.create_font(L"arial", 13, false, true, font_engine_base::gf_none),
					e.create_font(L"arial", 29, false, true, font_engine_base::gf_none),
					e.create_font(L"arial", 140, false, true, font_engine_base::gf_none),
				};
				fonts1;

				// ASSERT
				mocks::font_descriptor fd1[] = {
					mocks::font_descriptor(L"arial", 1000, false, true, font_engine_base::gf_none),
				};

				assert_equal(fd1, loader.created_log);

				// ACT
				font::ptr fonts2[] = {
					e.create_font(L"times", 31, true, false, font_engine_base::gf_none),
					e.create_font(L"times", 91, true, false, font_engine_base::gf_none),
				};
				fonts2;

				// ASSERT
				mocks::font_descriptor fd2[] = {
					mocks::font_descriptor(L"arial", 1000, false, true, font_engine_base::gf_none),
					mocks::font_descriptor(L"times", 1000, true, false, font_engine_base::gf_none),
				};

				assert_equal(fd2, loader.created_log);
			}


			test( ScalableFontsReuseOutlinesReturnedByFontAccessor )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, { L'B', 1 }, { L'C', 2 }, };
				mocks::font_accessor::glyph glyphs[] = {
					mocks::glyph(1.1, 1.2, c_outline_1),
					mocks::glyph(1.3, 1.4, c_outline_2),
					mocks::glyph(2.3, 2.4, c_outline_diamond),
				};
				pair<mocks::font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(mocks::font_descriptor(L"Arial", 1000, false, false, font_engine_base::gf_none),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				mocks::font_accessor &arial_accessor = loader.fonts[mocks::font_descriptor(L"Arial", 1000, false, false,
					font_engine_base::gf_none)];
				font_engine_base e(loader);
				font::ptr f1 = e.create_font(L"Arial", 17, false, false, font_engine_base::gf_none);
				font::ptr f2 = e.create_font(L"Arial", 210, false, false, font_engine_base::gf_none);

				// ACT
				f1->get_glyph(0);
				f2->get_glyph(1);

				// ASSERT
				assert_equal(2u, *arial_accessor.glyphs_loaded);

				// ACT
				f2->get_glyph(0);
				f1->get_glyph(1);

				// ASSERT
				assert_equal(2u, *arial_accessor.glyphs_loaded);

				// ACT
				f1->get_glyph(2);

				// ASSERT
				assert_equal(3u, *arial_accessor.glyphs_loaded);

				// ACT
				f2->get_glyph(2);

				// ASSERT
				assert_equal(3u, *arial_accessor.glyphs_loaded);
			}


			test( FontCreatedReceivesTheAccessorAndTheScalingNecessary )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font_accessor::glyph glyphs1[] = {
					mocks::glyph(1.1, 1.2, c_outline_1), mocks::glyph(1.3, 1.4, c_outline_2),
				};
				mocks::font_accessor::glyph glyphs2[] = {
					mocks::glyph(2.1, 2.2, c_outline_2), mocks::glyph(3.3, 3.4, c_outline_1),
				};
				pair<mocks::font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(mocks::font_descriptor(L"Tahoma", 1000, false, false, font_engine_base::gf_none),
						mocks::font_accessor(c_fm1, indices, glyphs1)),
					make_pair(mocks::font_descriptor(L"Verdana", 1000, true, false, font_engine_base::gf_none),
						mocks::font_accessor(c_fm2, indices, glyphs2)),
				};
				mocks::fonts_loader loader(fonts);
				font_engine_base e(loader);

				// ACT
				font::ptr f = e.create_font(L"Tahoma", 17, false, false, font_engine_base::gf_none);
				const glyph *g1 = f->get_glyph(0);
				pod_vector<glyph::path_point> o1 = convert_copy(g1->get_outline());
				const glyph *g2 = f->get_glyph(1);
				pod_vector<glyph::path_point> o2 = convert_copy(g2->get_outline());

				// ASSERT
				assert_equal(0.017 * c_fm1, f->get_metrics());
				assert_is_true(equal(0.017f * 1.1f, g1->metrics.advance_x));
				assert_is_true(equal(0.017f * 1.2f, g1->metrics.advance_y));
				assert_equal(c_outline_1, (1 / 0.017) * o1);
				assert_is_true(equal(0.017f * 1.3f, g2->metrics.advance_x));
				assert_is_true(equal(0.017f * 1.4f, g2->metrics.advance_y));
				assert_equal(c_outline_2, (1 / 0.017) * o2);

				// ACT
				f = e.create_font(L"Verdana", 710, true, false, font_engine_base::gf_none);
				g1 = f->get_glyph(0);
				pod_vector<glyph::path_point> o3 = convert_copy(g1->get_outline());
				g2 = f->get_glyph(1);
				pod_vector<glyph::path_point> o4 = convert_copy(g2->get_outline());

				// ASSERT
				assert_equal(0.71 * c_fm2, f->get_metrics());
				assert_equal(c_outline_2, (1 / 0.71) * o3);
				assert_equal(c_outline_1, (1 / 0.71) * o4);
			}


			test( GlyphsRequestedAreRenderedIntoRasterizerProvidedAtAnExpectedIntegerPosition )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, { L'b', 1 }, { L'c', 2 }, };
				mocks::font_accessor::glyph glyphs[] = {
					mocks::glyph(0, 0, c_outline_1), mocks::glyph(0, 0, c_outline_2), mocks::glyph(0, 0, c_outline_diamond),
				};
				pair<mocks::font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(mocks::font_descriptor(L"Arial", 10, false, false, font_engine_base::gf_strong),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				font_engine<mocks::rasterizer> e(loader);
				mocks::rasterizer target;
				font::ptr f = e.create_font(L"Arial", 10, false, false, font_engine_base::gf_strong);

				// ACT
				e.render_glyph(target, *f, 1, 19.0f, 3.0f);

				// ASSERT
				assert_equal(1u, target.append_log.size());
				assert_equal(c_outline_2, target.append_log[0].first->path);
				assert_equal(mkpoint(19, 3), target.append_log[0].second);

				// ACT
				e.render_glyph(target, *f, 1, 119.0f, -30.0f);

				// ASSERT
				assert_equal(2u, target.append_log.size());
				assert_equal(c_outline_2, target.append_log[1].first->path);
				assert_equal(mkpoint(119, -30), target.append_log[1].second);

				// ACT
				e.render_glyph(target, *f, 2, -1719.0f, 29.0f);

				// ASSERT
				assert_equal(3u, target.append_log.size());
				assert_equal(c_outline_diamond, target.append_log[2].first->path);
				assert_equal(mkpoint(-1719, 29), target.append_log[2].second);
			}


			test( GlyphRastersAreCachedAcrossCalls )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, { L'b', 1 }, { L'c', 2 }, };
				mocks::font_accessor::glyph glyphs[] = {
					mocks::glyph(0, 0, c_outline_1), mocks::glyph(0, 0, c_outline_2), mocks::glyph(0, 0, c_outline_diamond),
				};
				pair<mocks::font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(mocks::font_descriptor(L"Arial", 10, false, false, font_engine_base::gf_strong),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				font_engine<mocks::rasterizer> e(loader);
				mocks::rasterizer target;
				font::ptr f = e.create_font(L"Arial", 10, false, false, font_engine_base::gf_strong);

				e.render_glyph(target, *f, 1, 19.0f, 3.0f);
				e.render_glyph(target, *f, 0, 1.0f, 2.0f);
				e.render_glyph(target, *f, 2, 0.0f, 0.0f);

				// ACT
				e.render_glyph(target, *f, 1, 29.0f, -113.0f);
				e.render_glyph(target, *f, 0, 19.0f, -13.0f);

				// ASSERT
				assert_equal(5u, target.append_log.size());
				assert_equal(target.append_log[0].first, target.append_log[3].first);
				assert_equal(target.append_log[1].first, target.append_log[4].first);

				// ACT
				e.render_glyph(target, *f, 1, 29.0f, -113.0f);
				e.render_glyph(target, *f, 2, 19.0f, -13.0f);

				// ASSERT
				assert_equal(7u, target.append_log.size());
				assert_equal(target.append_log[0].first, target.append_log[5].first);
				assert_equal(target.append_log[2].first, target.append_log[6].first);
			}

		end_test_suite
	}
}
