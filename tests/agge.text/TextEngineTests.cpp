#include <agge.text/text_engine.h>

#include "helpers.h"
#include "mocks.h"
#include "outlines.h"

#include <agge/path.h>
#include <algorithm>
#include <tests/common/helpers.h>
#include <tests/common/scoped_ptr.h>
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

			template <typename ContainerT>
			vector<font::key> get_descriptors(const ContainerT &created_log)
			{
				vector<font::key> result;

				for (typename ContainerT::const_iterator i = created_log.begin(); i != created_log.end(); ++i)
					result.push_back(i->first);
				return result;
			}
		}

		begin_test_suite( TextEngineTests )
			
			test( CreatingDifferentFontsReturnsDifferentObjects )
			{
				// INIT
				mocks::fonts_loader loader;
				text_engine_base e(loader);

				// ACT
				font::ptr fonts[] = {
					e.create_font(L"arial", 13, false, false, font::key::gf_strong),
					e.create_font(L"helvetica", 13, false, false, font::key::gf_strong),
					e.create_font(L"tahoma", 13, false, false, font::key::gf_strong),
					e.create_font(L"tahoma", 14, false, false, font::key::gf_strong),
					e.create_font(L"helvetica", 13, true, false, font::key::gf_strong),
					e.create_font(L"helvetica", 13, false, true, font::key::gf_strong),
					e.create_font(L"helvetica", 13, true, true, font::key::gf_strong),
					e.create_font(L"helvetica", 13, true, true, font::key::gf_vertical),
					e.create_font(L"helvetica", 13, true, true, font::key::gf_none),
				};

				// ASSERT
				sort(tests::begin(fonts), tests::end(fonts));

				assert_equal(tests::end(fonts), unique(tests::begin(fonts), tests::end(fonts)));
			}


			test( CreatingTheSameFontReturnsTheSameObject )
			{
				// INIT
				mocks::fonts_loader loader;
				text_engine_base e(loader);

				// ACT
				font::ptr fonts[] = {
					e.create_font(L"arial", 13, false, false, font::key::gf_strong),
					e.create_font(L"ARial", 13, false, false, font::key::gf_strong),
					e.create_font(L"arial", 15, false, false, font::key::gf_vertical),
					e.create_font(L"Arial", 15, false, false, font::key::gf_vertical),
					e.create_font(L"arial", 13, false, true, font::key::gf_none),
					e.create_font(L"arial", 13, false, true, font::key::gf_none),
					e.create_font(L"tahoma", 143, true, false, font::key::gf_strong),
					e.create_font(L"taHOMA", 143, true, false, font::key::gf_strong),
				};

				// ASSERT
				assert_equal(fonts[0], fonts[1]);
				assert_equal(fonts[2], fonts[3]);
				assert_equal(fonts[4], fonts[5]);
				assert_equal(fonts[6], fonts[7]);

				// ACT
				font::ptr f2 = e.create_font(L"arial", 13, false, false, font::key::gf_strong);

				// ASSERT
				assert_equal(fonts[0], f2);
			}


			test( CreatingTheFontCreatesFontAccessor )
			{
				// INIT
				mocks::fonts_loader loader;
				text_engine_base e(loader);

				// ACT
				font::ptr f11 = e.create_font(L"arial", 13, false, false, font::key::gf_strong);
				font::ptr f12 = e.create_font(L"tahoma", 29, true, false, font::key::gf_vertical);
				font::ptr f13 = e.create_font(L"helvetica", 15, false, true, font::key::gf_none /* freely scalable */);

				// ASSERT
				font::key fd1[] = {
					font::key(L"arial", 13, false, false, font::key::gf_strong),
					font::key(L"tahoma", 29, true, false, font::key::gf_vertical),
					font::key(L"helvetica", 1000, false, true, font::key::gf_none),
				};

				assert_equal(fd1, get_descriptors(loader.created_log));

				// INIT
				loader.created_log.clear();

				// ACT
				font::ptr f21 = e.create_font(L"arial", 15, true, true, font::key::gf_none /* freely scalable */);

				// ASSERT
				font::key fd2[] = {
					font::key(L"arial", 1000, true, true, font::key::gf_none),
				};

				assert_equal(fd2, get_descriptors(loader.created_log));
			}


			test( ScalableFontAccessorIsCreatedOnceForDifferentFontSizes )
			{
				// INIT
				mocks::fonts_loader loader;
				text_engine_base e(loader);

				// ACT
				font::ptr f11 = e.create_font(L"arial", 13, false, true, font::key::gf_none);
				font::ptr f12 = e.create_font(L"arial", 29, false, true, font::key::gf_none);
				font::ptr f13 = e.create_font(L"arial", 140, false, true, font::key::gf_none);

				// ASSERT
				font::key fd1[] = {
					font::key(L"arial", 1000, false, true, font::key::gf_none),
				};

				assert_equal(fd1, get_descriptors(loader.created_log));

				// ACT
				font::ptr f21 = e.create_font(L"times", 31, true, false, font::key::gf_none);
				font::ptr f22 = e.create_font(L"times", 91, true, false, font::key::gf_none);

				// ASSERT
				font::key fd2[] = {
					font::key(L"arial", 1000, false, true, font::key::gf_none),
					font::key(L"times", 1000, true, false, font::key::gf_none),
				};

				assert_equal(fd2, get_descriptors(loader.created_log));
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
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 1000, false, false, font::key::gf_none),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				mocks::font_accessor &arial_accessor = loader.fonts[font::key(L"Arial", 1000, false, false,
					font::key::gf_none)];
				text_engine_base e(loader);
				font::ptr f1 = e.create_font(L"Arial", 17, false, false, font::key::gf_none);
				font::ptr f2 = e.create_font(L"Arial", 210, false, false, font::key::gf_none);

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
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Tahoma", 1000, false, false, font::key::gf_none),
						mocks::font_accessor(c_fm1, indices, glyphs1)),
					make_pair(font::key(L"Verdana", 1000, true, false, font::key::gf_none),
						mocks::font_accessor(c_fm2, indices, glyphs2)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine_base e(loader);

				// ACT
				font::ptr f = e.create_font(L"Tahoma", 17, false, false, font::key::gf_none);
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
				f = e.create_font(L"Verdana", 710, true, false, font::key::gf_none);
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
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_strong),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader);
				mocks::rasterizer target;
				font::ptr f = e.create_font(L"Arial", 10, false, false, font::key::gf_strong);

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
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_strong),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader);
				mocks::rasterizer target;
				font::ptr f = e.create_font(L"Arial", 10, false, false, font::key::gf_strong);

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


			test( GlyphRastersAreCachedAtAllowedPresetFractionalPrecisionX2 )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, };
				mocks::font_accessor::glyph glyphs[] = {
					mocks::glyph(0, 0, c_outline_2),
				};
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_strong),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 1);
				mocks::rasterizer target;
				font::ptr f = e.create_font(L"Arial", 10, false, false, font::key::gf_strong);

				// ACT
				e.render_glyph(target, *f, 0, 1.0f, 1.0f);
				e.render_glyph(target, *f, 0, 1.49f, 1.0f);
				e.render_glyph(target, *f, 0, 1.0f, 1.49f);
				e.render_glyph(target, *f, 0, 1.49f, 1.49f);

				// ASSERT
				assert_equal(4u, target.append_log.size());
				assert_equal(1.1f + 0.0f, target.append_log[0].first->path[0].x);
				assert_equal(2.4f + 0.0f, target.append_log[0].first->path[0].y);
				assert_equal(target.append_log[0].first, target.append_log[1].first);
				assert_equal(target.append_log[0].first, target.append_log[2].first);
				assert_equal(target.append_log[0].first, target.append_log[3].first);

				// ACT
				e.render_glyph(target, *f, 0, 1.5f, 1.0f);
				e.render_glyph(target, *f, 0, 1.99f, 1.0f);
				e.render_glyph(target, *f, 0, 1.5f, 1.49f);
				e.render_glyph(target, *f, 0, 1.99f, 1.49f);

				// ASSERT
				assert_equal(8u, target.append_log.size());
				assert_not_equal(target.append_log[0].first, target.append_log[4].first);
				assert_equal(1.1f + 0.5f, target.append_log[4].first->path[0].x);
				assert_equal(2.4f + 0.0f, target.append_log[4].first->path[0].y);
				assert_equal(target.append_log[4].first, target.append_log[5].first);
				assert_equal(target.append_log[4].first, target.append_log[6].first);
				assert_equal(target.append_log[4].first, target.append_log[7].first);

				// ACT
				e.render_glyph(target, *f, 0, 1.0f, 1.5f);
				e.render_glyph(target, *f, 0, 1.49f, 1.5f);
				e.render_glyph(target, *f, 0, 1.0f, 1.99f);
				e.render_glyph(target, *f, 0, 1.49f, 1.99f);

				// ASSERT
				assert_equal(12u, target.append_log.size());
				assert_not_equal(target.append_log[0].first, target.append_log[4].first);
				assert_not_equal(target.append_log[4].first, target.append_log[8].first);
				assert_equal(1.1f + 0.0f, target.append_log[8].first->path[0].x);
				assert_equal(2.4f + 0.5f, target.append_log[8].first->path[0].y);
				assert_equal(target.append_log[8].first, target.append_log[9].first);
				assert_equal(target.append_log[8].first, target.append_log[10].first);
				assert_equal(target.append_log[8].first, target.append_log[11].first);

				// ACT
				e.render_glyph(target, *f, 0, 1.5f, 1.5f);
				e.render_glyph(target, *f, 0, 1.99f, 1.5f);
				e.render_glyph(target, *f, 0, 1.5f, 1.99f);
				e.render_glyph(target, *f, 0, 1.99f, 1.99f);

				// ASSERT
				assert_equal(16u, target.append_log.size());
				assert_not_equal(target.append_log[0].first, target.append_log[12].first);
				assert_not_equal(target.append_log[4].first, target.append_log[12].first);
				assert_not_equal(target.append_log[8].first, target.append_log[12].first);
				assert_equal(1.1f + 0.5f, target.append_log[12].first->path[0].x);
				assert_equal(2.4f + 0.5f, target.append_log[12].first->path[0].y);
				assert_equal(4.1f + 0.5f, target.append_log[12].first->path[1].x);
				assert_equal(7.5f + 0.5f, target.append_log[12].first->path[1].y);
				assert_equal(4.1f + 0.5f, target.append_log[12].first->path[2].x);
				assert_equal(4.5f + 0.5f, target.append_log[12].first->path[2].y);
				assert_equal(5.5f + 0.5f, target.append_log[12].first->path[3].x);
				assert_equal(1.1f + 0.5f, target.append_log[12].first->path[3].y);
				assert_equal(target.append_log[12].first, target.append_log[13].first);
				assert_equal(target.append_log[12].first, target.append_log[14].first);
				assert_equal(target.append_log[12].first, target.append_log[15].first);

				// ACT (different whole cell)
				e.render_glyph(target, *f, 0, 11.5f, 113.5f);
				e.render_glyph(target, *f, 0, 11.99f, 113.5f);
				e.render_glyph(target, *f, 0, 11.5f, 113.99f);
				e.render_glyph(target, *f, 0, 11.99f, 113.99f);

				// ASSERT
				assert_equal(20u, target.append_log.size());
				assert_equal(target.append_log[12].first, target.append_log[16].first);
				assert_equal(target.append_log[16].first, target.append_log[17].first);
				assert_equal(target.append_log[16].first, target.append_log[18].first);
				assert_equal(target.append_log[16].first, target.append_log[19].first);

				// ACT (negative whole cell)
				e.render_glyph(target, *f, 0, -11.52f, -113.53f);
				e.render_glyph(target, *f, 0, -11.98f, -113.54f);
				e.render_glyph(target, *f, 0, -11.55f, -113.97f);
				e.render_glyph(target, *f, 0, -11.96f, -113.95f);

				// ASSERT
				assert_equal(24u, target.append_log.size());
				assert_equal(target.append_log[0].first, target.append_log[20].first);
				assert_equal(target.append_log[20].first, target.append_log[21].first);
				assert_equal(target.append_log[20].first, target.append_log[22].first);
				assert_equal(target.append_log[20].first, target.append_log[23].first);
			}


			test( ReferenceGlyphsAreRenderedAtGivenSubpixelGrid )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, };
				mocks::font_accessor::glyph glyphs[] = {
					mocks::glyph(0, 0, c_outline_2),
				};
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_strong),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e1(loader, 2);
				text_engine<mocks::rasterizer> e2(loader, 3);
				mocks::rasterizer target;
				font::ptr f1 = e1.create_font(L"Arial", 10, false, false, font::key::gf_strong);
				font::ptr f2 = e2.create_font(L"Arial", 10, false, false, font::key::gf_strong);

				// ACT
				e1.render_glyph(target, *f1, 0, 1.15f, 1.3f);
				e1.render_glyph(target, *f1, 0, 1.55f, 1.9f);
				e2.render_glyph(target, *f2, 0, 1.15f, 1.3f);
				e2.render_glyph(target, *f2, 0, 1.55f, 1.9f);

				// ASSERT
				assert_equal(1.1f + 0.0f, target.append_log[0].first->path[0].x);
				assert_equal(2.4f + 0.25f, target.append_log[0].first->path[0].y);
				assert_equal(1.1f + 0.5f, target.append_log[1].first->path[0].x);
				assert_equal(2.4f + 0.75f, target.append_log[1].first->path[0].y);

				assert_equal(1.1f + 0.125f, target.append_log[2].first->path[0].x);
				assert_equal(2.4f + 0.25f, target.append_log[2].first->path[0].y);
				assert_equal(1.1f + 0.5f, target.append_log[3].first->path[0].x);
				assert_equal(2.4f + 0.875f, target.append_log[3].first->path[0].y);
			}


			test( SingleLineLayoutGlyphsAreRenderedInTheProperOrder1 )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, { L'w', 1 }, };
				mocks::font_accessor::glyph glyphs[] = {
					mocks::glyph(5, 0, c_outline_1),
					mocks::glyph(7, 0, c_outline_2),
				};
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 0);
				mocks::rasterizer target;
				layout l1(L"aww", e.create_font(L"Arial", 10, false, false, font::key::gf_strong));

				// ACT
				e.render_layout(target, l1, 0.0f, 0.0f);

				// ASSERT
				assert_equal(3u, target.append_log.size());
				assert_equal(0, target.append_log[0].second.x);
				assert_equal(4, target.append_log[0].second.y);
				assert_equal(c_outline_1, target.append_log[0].first->path);
				assert_equal(5, target.append_log[1].second.x);
				assert_equal(4, target.append_log[1].second.y);
				assert_equal(c_outline_2, target.append_log[1].first->path);
				assert_equal(12, target.append_log[2].second.x);
				assert_equal(4, target.append_log[2].second.y);
				assert_equal(c_outline_2, target.append_log[2].first->path);
			}


			test( SingleLineLayoutGlyphsAreRenderedInTheProperOrder2 )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, { L'w', 1 }, };
				mocks::font_accessor::glyph glyphs[] = {
					mocks::glyph(5, 0, c_outline_1),
					mocks::glyph(7, 0, c_outline_2),
				};
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_strong),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 0);
				mocks::rasterizer target;
				layout l(L"wa", e.create_font(L"Arial", 10, false, false, font::key::gf_strong));

				// ACT
				e.render_layout(target, l, 0.0f, 0.0f);

				// ASSERT
				assert_equal(2u, target.append_log.size());
				assert_equal(0, target.append_log[0].second.x);
				assert_equal(1, target.append_log[0].second.y);
				assert_equal(c_outline_2, target.append_log[0].first->path);
				assert_equal(7, target.append_log[1].second.x);
				assert_equal(1, target.append_log[1].second.y);
				assert_equal(c_outline_1, target.append_log[1].first->path);
			}


			test( MultiLineLayoutGlyphsAreRenderedInTheProperOrder )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, { L'w', 1 }, };
				mocks::font_accessor::glyph glyphs[] = {
					mocks::glyph(5, 0, c_outline_1),
					mocks::glyph(7, 0, c_outline_2),
				};
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 0);
				mocks::rasterizer target;
				layout l(L"aww\nww\na", e.create_font(L"Arial", 10, false, false, font::key::gf_strong));

				// ACT
				e.render_layout(target, l, 0.0f, 0.0f);

				// ASSERT
				assert_equal(6u, target.append_log.size());

				assert_equal(0, target.append_log[0].second.x);
				assert_equal(4, target.append_log[0].second.y);
				assert_equal(c_outline_1, target.append_log[0].first->path);
				assert_equal(5, target.append_log[1].second.x);
				assert_equal(4, target.append_log[1].second.y);
				assert_equal(c_outline_2, target.append_log[1].first->path);
				assert_equal(12, target.append_log[2].second.x);
				assert_equal(4, target.append_log[2].second.y);
				assert_equal(c_outline_2, target.append_log[2].first->path);

				assert_equal(0, target.append_log[3].second.x);
				assert_equal(21, target.append_log[3].second.y);
				assert_equal(c_outline_2, target.append_log[3].first->path);
				assert_equal(7, target.append_log[4].second.x);
				assert_equal(21, target.append_log[4].second.y);
				assert_equal(c_outline_2, target.append_log[4].first->path);

				assert_equal(0, target.append_log[5].second.x);
				assert_equal(38, target.append_log[5].second.y);
				assert_equal(c_outline_1, target.append_log[5].first->path);
			}


			test( TextLayoutIsPositionedAccordinglyToOffset )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, };
				mocks::font_accessor::glyph glyphs[] = {
					mocks::glyph(5.2, 0, c_outline_1),
				};
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 0);
				mocks::rasterizer target;
				layout l(L"aaa", e.create_font(L"Arial", 10, false, false, font::key::gf_strong));

				// ACT
				e.render_layout(target, l, 7.7f, 13.2f);

				// ASSERT
				assert_equal(3u, target.append_log.size());
				assert_equal(7, target.append_log[0].second.x);
				assert_equal(17, target.append_log[0].second.y);
				assert_equal(c_outline_1, target.append_log[0].first->path);
				assert_equal(12, target.append_log[1].second.x);
				assert_equal(17, target.append_log[1].second.y);
				assert_equal(c_outline_1, target.append_log[1].first->path);
				assert_equal(18, target.append_log[2].second.x);
				assert_equal(17, target.append_log[2].second.y);
				assert_equal(c_outline_1, target.append_log[2].first->path);
			}


			test( ReleasedFontIsNotifiedAboutImmidiatelyWhenNoCollectionIsRequired )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { mocks::glyph(5.2, 0, c_outline_1), };
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				mocks::logging_text_engine e(loader, 0);
				font::ptr f = e.create_font(L"Arial", 10, false, false, font::key::gf_strong);
				void *pvf = f.get();

				// ACT
				f.reset();

				// ASSERT
				void *reference[] = { pvf, };

				assert_equal(reference, e.deletion_log);
			}


			test( DestroyedFontReleasesUnderlyingAccessor )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { mocks::glyph(5.2, 0, c_outline_1), };
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 1000, false, false, font::key::gf_none),
						mocks::font_accessor(c_fm2, indices, glyphs)),
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				mocks::logging_text_engine e(loader, 0);
				font::ptr f[] = {
					e.create_font(L"Arial", 10, false, false, font::key::gf_none),
					e.create_font(L"Arial", 11, false, false, font::key::gf_none),
					e.create_font(L"Arial", 11, false, false, font::key::gf_strong),
				};

				// ACT
				f[0].reset();

				// ASSERT
				assert_equal(2u, *loader.allocated);

				// ACT
				f[1].reset();

				// ASSERT
				assert_equal(1u, *loader.allocated);

				// ACT
				f[2].reset();

				// ASSERT
				assert_equal(0u, *loader.allocated);
			}


			test( DestroyedAccessorGetsReacquiredOnFontReCreation )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { mocks::glyph(5.2, 0, c_outline_1), };
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 1000, false, false, font::key::gf_none),
						mocks::font_accessor(c_fm2, indices, glyphs)),
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				mocks::logging_text_engine e(loader, 0);
				font::ptr f = e.create_font(L"Arial", 10, false, false, font::key::gf_none);

				f.reset();

				// ACT
				f = e.create_font(L"Arial", 10, false, false, font::key::gf_none);

				// ASSERT
				font::key fd[] = {
					font::key(L"Arial", 1000, false, false, font::key::gf_none),
					font::key(L"Arial", 1000, false, false, font::key::gf_none),
				};

				assert_equal(fd, get_descriptors(loader.created_log));
			}


			test( ReleasedFontIsNotifiedUponOnPredefinedCollectionCycle )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { mocks::glyph(5.2, 0, c_outline_1), };
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
					make_pair(font::key(L"Tahoma", 10, true, false, font::key::gf_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				mocks::logging_text_engine e1(loader, 2);
				font::ptr f1 = e1.create_font(L"Arial", 10, false, false, font::key::gf_strong);
				font::ptr f2 = e1.create_font(L"Tahoma", 10, true, false, font::key::gf_strong);
				void *pvf1 = f1.get();
				void *pvf2 = f2.get();
				mocks::logging_text_engine e2(loader, 5);
				font::ptr f3 = e2.create_font(L"Arial", 10, false, false, font::key::gf_strong);
				void *pvf3 = f3.get();

				// ACT
				f1.reset();
				e1.collect();

				// ASSERT
				assert_is_empty(e1.deletion_log);
				assert_equal(3u, *loader.allocated);

				// ACT
				f2.reset();
				e1.collect();

				// ASSERT
				void *reference1[] = { pvf1, };

				assert_equal(reference1, e1.deletion_log);
				assert_equal(2u, *loader.allocated);

				// ACT
				e1.collect();

				// ASSERT
				void *reference2[] = { pvf1, pvf2, };

				assert_equal(reference2, e1.deletion_log);
				assert_equal(1u, *loader.allocated);

				// ACT
				f3.reset();
				e2.collect();
				e2.collect();
				e2.collect();
				e2.collect();

				// ASSERT
				assert_is_empty(e2.deletion_log);
				assert_equal(1u, *loader.allocated);

				// ACT
				e2.collect();

				// ASSERT
				void *reference3[] = { pvf3, };

				assert_equal(reference3, e2.deletion_log);
				assert_equal(0u, *loader.allocated);
			}


			test( GarbageFontIsReusedForTheSameRequest )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { mocks::glyph(5.2, 0, c_outline_1), };
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
					make_pair(font::key(L"Tahoma", 10, true, false, font::key::gf_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				mocks::logging_text_engine e(loader, 1);
				font::ptr f1 = e.create_font(L"Arial", 10, false, false, font::key::gf_strong);
				font::ptr f2 = e.create_font(L"Tahoma", 10, true, false, font::key::gf_strong);
				void *pvf1 = f1.get();
				void *pvf2 = f2.get();

				loader.created_log.clear();
				f1.reset();
				f2.reset();

				// ACT
				f2 = e.create_font(L"Tahoma", 10, true, false, font::key::gf_strong);

				// ASSERT
				assert_equal(pvf2, f2.get());
				assert_is_empty(loader.created_log);

				// ACT
				f1 = e.create_font(L"Arial", 10, false, false, font::key::gf_strong);

				// ASSERT
				assert_equal(pvf1, f1.get());
				assert_is_empty(loader.created_log);

				// ACT
				e.collect();

				// ASSERT
				assert_equal(2u, *loader.allocated);
			}


			test( DestructionOfEngineWithNonEmptyGarbageDestroysGarbageFonts )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { mocks::glyph(5.2, 0, c_outline_1), };
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_none),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				scoped_ptr<mocks::logging_text_engine> e(new mocks::logging_text_engine(loader, 1));
				font::ptr f1 = e->create_font(L"Arial", 101, false, false, font::key::gf_none);
				font::ptr f2 = e->create_font(L"Arial", 15, false, false, font::key::gf_none);

				f1.reset();
				f2.reset();

				// ACT
				e.reset();

				// ASSERT
				assert_equal(0u, *loader.allocated);
			}


			test( DifferentFontsHaveDistinctRasterCaches )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { mocks::glyph(5.2, 0, c_outline_1), };
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 1000, false, false, font::key::gf_none),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 1);
				mocks::rasterizer target;
				font::ptr f1 = e.create_font(L"Arial", 101, false, false, font::key::gf_none);
				font::ptr f2 = e.create_font(L"Arial", 15, false, false, font::key::gf_none);
				font::ptr f3 = e.create_font(L"Arial", 10, false, false, font::key::gf_none);

				// ACT
				e.render_glyph(target, *f1, 0, 19.0f, -13.49f);
				e.render_glyph(target, *f2, 0, 19.0f, -13.0f);
				e.render_glyph(target, *f3, 0, 19.0f, -13.0f);

				// ASSERT
				assert_equal(3u, target.append_log.size());
				assert_not_equal(target.append_log[1].first, target.append_log[0].first);
				assert_not_equal(target.append_log[2].first, target.append_log[1].first);
				assert_not_equal(target.append_log[2].first, target.append_log[0].first);
			}

		end_test_suite
	}
}
