#include <agge.text/text_engine.h>

#include "helpers.h"
#include "helpers_layout.h"
#include "mocks.h"
#include "outlines.h"

#include <agge/path.h>
#include <agge.text/limit.h>
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
			font_metrics c_fm1 = { 1.1f, 2.2f, 3.3f };
			font_metrics c_fm2 = { 4.4f, 5.6f, 7.1f };

			template <typename ContainerT>
			vector<font_descriptor> get_descriptors(const ContainerT &created_log)
			{
				vector<font_descriptor> result;

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
					e.create_font(font_descriptor::create("arial", 13, regular, false, hint_strong)),
					e.create_font(font_descriptor::create("helvetica", 13, regular, false, hint_strong)),
					e.create_font(font_descriptor::create("tahoma", 13, regular, false, hint_strong)),
					e.create_font(font_descriptor::create("tahoma", 14, regular, false, hint_strong)),
					e.create_font(font_descriptor::create("helvetica", 13, bold, false, hint_strong)),
					e.create_font(font_descriptor::create("helvetica", 13, regular, true, hint_strong)),
					e.create_font(font_descriptor::create("helvetica", 13, bold, true, hint_strong)),
					e.create_font(font_descriptor::create("helvetica", 13, bold, true, hint_vertical)),
					e.create_font(font_descriptor::create("helvetica", 13, bold, true, hint_none)),
					e.create_font(font_descriptor::create("helvetica", 15, bold, true, hint_none)),
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
					e.create_font(font_descriptor::create("arial", 13, regular, false, hint_strong)),
					e.create_font(font_descriptor::create("ARial", 13, regular, false, hint_strong)),
					e.create_font(font_descriptor::create("arial", 15, regular, false, hint_vertical)),
					e.create_font(font_descriptor::create("Arial", 15, regular, false, hint_vertical)),
					e.create_font(font_descriptor::create("arial", 13, regular, true, hint_none)),
					e.create_font(font_descriptor::create("arial", 13, regular, true, hint_none)),
					e.create_font(font_descriptor::create("tahoma", 143, bold, false, hint_strong)),
					e.create_font(font_descriptor::create("taHOMA", 143, bold, false, hint_strong)),
				};

				// ASSERT
				assert_equal(fonts[0], fonts[1]);
				assert_equal(fonts[2], fonts[3]);
				assert_equal(fonts[4], fonts[5]);
				assert_equal(fonts[6], fonts[7]);

				// ACT
				font::ptr f2 = e.create_font(font_descriptor::create("arial", 13, regular, false, hint_strong));

				// ASSERT
				assert_equal(fonts[0], f2);
			}


			test( CreatingTheFontCreatesFontAccessor )
			{
				// INIT
				mocks::fonts_loader loader;
				text_engine_base e(loader);

				// ACT
				font::ptr f11 = e.create_font(font_descriptor::create("arial", 13, regular, false, hint_strong));
				font::ptr f12 = e.create_font(font_descriptor::create("tahoma", 29, bold, false, hint_vertical));
				font::ptr f13 = e.create_font(font_descriptor::create("helvetica", 15, regular, true, hint_none /* freely scalable */));

				// ASSERT
				font_descriptor fd1[] = {
					font_descriptor::create("arial", 13, regular, false, hint_strong),
					font_descriptor::create("tahoma", 29, bold, false, hint_vertical),
					font_descriptor::create("helvetica", 1000, regular, true, hint_none),
				};

				assert_equal(fd1, get_descriptors(loader.created_log));

				// INIT
				loader.created_log.clear();

				// ACT
				font::ptr f21 = e.create_font(font_descriptor::create("arial", 15, bold, true, hint_none /* freely scalable */));

				// ASSERT
				font_descriptor fd2[] = {
					font_descriptor::create("arial", 1000, bold, true, hint_none),
				};

				assert_equal(fd2, get_descriptors(loader.created_log));
			}


			test( ScalableFontAccessorIsCreatedOnceForDifferentFontSizes )
			{
				// INIT
				mocks::fonts_loader loader;
				text_engine_base e(loader);

				// ACT
				font::ptr f11 = e.create_font(font_descriptor::create("arial", 13, regular, true, hint_none));
				font::ptr f12 = e.create_font(font_descriptor::create("arial", 29, regular, true, hint_none));
				font::ptr f13 = e.create_font(font_descriptor::create("arial", 140, regular, true, hint_none));

				// ASSERT
				font_descriptor fd1[] = {
					font_descriptor::create("arial", 1000, regular, true, hint_none),
				};

				assert_equal(fd1, get_descriptors(loader.created_log));

				// ACT
				font::ptr f21 = e.create_font(font_descriptor::create("times", 31, bold, false, hint_none));
				font::ptr f22 = e.create_font(font_descriptor::create("times", 91, bold, false, hint_none));

				// ASSERT
				font_descriptor fd2[] = {
					font_descriptor::create("arial", 1000, regular, true, hint_none),
					font_descriptor::create("times", 1000, bold, false, hint_none),
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
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 1000, regular, false, hint_none),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				mocks::font_accessor &arial_accessor = loader.fonts[font_descriptor::create("Arial", 1000, regular, false,
					hint_none)];
				text_engine_base e(loader);
				font::ptr f1 = e.create_font(font_descriptor::create("Arial", 17, regular, false, hint_none));
				font::ptr f2 = e.create_font(font_descriptor::create("Arial", 210, regular, false, hint_none));

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
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Tahoma", 1000, regular, false, hint_none),
						mocks::font_accessor(c_fm1, indices, glyphs1)),
					make_pair(font_descriptor::create("Verdana", 1000, bold, false, hint_none),
						mocks::font_accessor(c_fm2, indices, glyphs2)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine_base e(loader);

				// ACT
				font::ptr f = e.create_font(font_descriptor::create("Tahoma", 17, regular, false, hint_none));
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
				f = e.create_font(font_descriptor::create("Verdana", 710, bold, false, hint_none));
				g1 = f->get_glyph(0);
				pod_vector<glyph::path_point> o3 = convert_copy(g1->get_outline());
				g2 = f->get_glyph(1);
				pod_vector<glyph::path_point> o4 = convert_copy(g2->get_outline());

				// ASSERT
				assert_equal(0.71 * c_fm2, f->get_metrics());
				assert_equal(c_outline_2, (1 / 0.71) * o3);
				assert_equal(c_outline_1, (1 / 0.71) * o4);
			}


			test( DifferentNamesCanProduceTheSameFont )
			{
				// INIT
				mocks::font_accessor accessors[] = {	mocks::font_accessor(), mocks::font_accessor(),	};
				accessors[0].descriptor = font_descriptor::create("plain #1", 10, bold, false);
				accessors[1].descriptor = font_descriptor::create("plain #2", 1000, regular, false);
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Tahoma", 11, regular, false, hint_strong), accessors[0]),
					make_pair(font_descriptor::create("Segoe", 10, regular, false, hint_strong), accessors[0]),
					make_pair(font_descriptor::create("Verdana", 1000, bold, false, hint_vertical), accessors[1]),
					make_pair(font_descriptor::create("Verdana,Helvetica", 1000, bold, false, hint_vertical), accessors[1]),
					make_pair(font_descriptor::create("Verdana,sans-serif", 1000, bold, false, hint_vertical), accessors[1]),
					make_pair(font_descriptor::create("Verdana,Helvetica", 100, bold, false, hint_none), accessors[1]),
					make_pair(font_descriptor::create("Verdana,sans-serif", 10, bold, false, hint_none), accessors[1]),
				};
				mocks::fonts_loader loader(fonts);
				text_engine_base e(loader);

				// ACT
				font::ptr f1 = e.create_font(font_descriptor::create("Tahoma", 11, regular, false, hint_strong));
				font::ptr f2 = e.create_font(font_descriptor::create("Verdana", 1000, bold, false, hint_vertical));
				font::ptr f3 = e.create_font(font_descriptor::create("Verdana,Helvetica", 100, bold, false, hint_none));

				// ACT / ASSERT
				assert_equal(f1, e.create_font(font_descriptor::create("Segoe", 10, regular, false, hint_strong)));
				assert_equal(f2, e.create_font(font_descriptor::create("verdana,helvetica", 1000, bold, false, hint_vertical)));
				assert_equal(f2, e.create_font(font_descriptor::create("Verdana,Sans-Serif", 1000, bold, false, hint_vertical)));
				assert_not_equal(f3, e.create_font(font_descriptor::create("Verdana,Sans-Serif", 10, bold, false, hint_none)));
			}


			test( GlyphsRequestedAreRenderedIntoRasterizerProvidedAtAnExpectedIntegerPosition )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, { L'b', 1 }, { L'c', 2 }, };
				mocks::font_accessor::glyph glyphs[] = {
					mocks::glyph(0, 0, c_outline_1), mocks::glyph(0, 0, c_outline_2), mocks::glyph(0, 0, c_outline_diamond),
				};
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader);
				mocks::rasterizer target;
				font::ptr f = e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));

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
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader);
				mocks::rasterizer target;
				font::ptr f = e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));

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
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 1);
				mocks::rasterizer target;
				font::ptr f = e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));

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
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e1(loader, 2);
				text_engine<mocks::rasterizer> e2(loader, 3);
				mocks::rasterizer target;
				font::ptr f1 = e1.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));
				font::ptr f2 = e2.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));

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


			test( GlyphRunIsRenderedInTheProperOrder )
			{
				// INIT
				mocks::font_accessor::char_to_index font_indices[] = { { L'a', 0 }, { L'w', 1 }, };
				mocks::font_accessor::glyph font_glyphs[] = {
					mocks::glyph(5, 0, c_outline_1),
					mocks::glyph(7, 0, c_outline_2),
				};
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm2, font_indices, font_glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 0);
				font::ptr f = e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));
				mocks::rasterizer target;
				positioned_glyph glyphs_[] = {
					{ 0, { 5.0f, 0.0f } }, { 1, { 7.0f, 0.0f } }, { 1, { 7.0f, 2.0f } }, { 0, { 5.0f, -3.0f } }, { 0, { 7.0f, 2.0f } }
				};
				positioned_glyphs_container_t glyphs = mkpodvector(glyphs_ + 0, glyphs_ + 5);
				glyph_run gr(glyphs);

				gr.begin_index = 0, gr.end_index = 3;
				gr.font_ = f;
				gr.offset = zero();

				// ACT
				e.render(target, gr, create_point(0.0f, 3.0f));

				// ASSERT
				assert_equal(3u, target.append_log.size());
				assert_equal(0, target.append_log[0].second.x);
				assert_equal(3, target.append_log[0].second.y);
				assert_equal(c_outline_1, target.append_log[0].first->path);
				assert_equal(5, target.append_log[1].second.x);
				assert_equal(3, target.append_log[1].second.y);
				assert_equal(c_outline_2, target.append_log[1].first->path);
				assert_equal(12, target.append_log[2].second.x);
				assert_equal(3, target.append_log[2].second.y);
				assert_equal(c_outline_2, target.append_log[2].first->path);

				// INIT
				target.append_log.clear();
				gr.begin_index = 1, gr.end_index = 5;
				gr.font_ = f;
				gr.offset = zero();

				// ACT
				e.render(target, gr, create_point(17.0f, 90.0f));

				// ASSERT
				assert_equal(4u, target.append_log.size());
				assert_equal(17, target.append_log[0].second.x);
				assert_equal(90, target.append_log[0].second.y);
				assert_equal(c_outline_2, target.append_log[0].first->path);
				assert_equal(24, target.append_log[1].second.x);
				assert_equal(90, target.append_log[1].second.y);
				assert_equal(c_outline_2, target.append_log[1].first->path);
				assert_equal(31, target.append_log[2].second.x);
				assert_equal(92, target.append_log[2].second.y);
				assert_equal(c_outline_1, target.append_log[2].first->path);
				assert_equal(36, target.append_log[3].second.x);
				assert_equal(89, target.append_log[3].second.y);
				assert_equal(c_outline_1, target.append_log[3].first->path);
			}


			test( SingleLineLayoutGlyphsAreRenderedInTheProperOrder1 )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, { L'w', 1 }, };
				mocks::font_accessor::glyph glyphs[] = {
					mocks::glyph(5, 0, c_outline_1),
					mocks::glyph(7, 0, c_outline_2),
				};
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 0);
				mocks::rasterizer target;
				layout l1;

				l1.process(simple_richtext("aww", "Arial", 10, regular, false, hint_strong), limit::none(), e);

				// ACT
				e.render(target, l1, create_point(0.0f, 0.0f));

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
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 0);
				mocks::rasterizer target;
				layout l;

				l.process(simple_richtext("wa", "Arial", 10, regular, false, hint_strong), limit::none(), e);

				// ACT
				e.render(target, l, create_point(0.0f, 0.0f));

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
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 0);
				mocks::rasterizer target;
				layout l;

				l.process(simple_richtext("aww\nww\na", "Arial", 10, regular, false, hint_strong), limit::none(), e);

				// ACT
				e.render(target, l, create_point(0.0f, 0.0f));

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
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 0);
				mocks::rasterizer target;
				layout l;

				l.process(simple_richtext("aaa", "Arial", 10, regular, false, hint_strong), limit::none(), e);

				// ACT
				e.render(target, l, create_point(7.7f, 13.2f));

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


			test( SimpleStringIsRenderedAsAGlyphRun )
			{
				// INIT
				mocks::font_accessor::char_to_index font_indices[] = { { L'a', 0 }, { L'w', 1 }, { L'z', 2 }, };
				mocks::font_accessor::glyph font_glyphs[] = {
					mocks::glyph(5, 0, c_outline_1),
					mocks::glyph(7, 0, c_outline_2),
					mocks::glyph(4, 2.1, c_outline_diamond),
				};
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm2, font_indices, font_glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 0);
				font::ptr f = e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));
				mocks::rasterizer target;

				// ACT
				e.render_string(target, *f, "aww", align_near, 0.0f, 3.0f);

				// ASSERT
				assert_equal(3u, target.append_log.size());
				assert_equal(0, target.append_log[0].second.x);
				assert_equal(3, target.append_log[0].second.y);
				assert_equal(c_outline_1, target.append_log[0].first->path);
				assert_equal(5, target.append_log[1].second.x);
				assert_equal(3, target.append_log[1].second.y);
				assert_equal(c_outline_2, target.append_log[1].first->path);
				assert_equal(12, target.append_log[2].second.x);
				assert_equal(3, target.append_log[2].second.y);
				assert_equal(c_outline_2, target.append_log[2].first->path);

				// INIT
				target.append_log.clear();

				// ACT
				e.render_string(target, *f, "wwaza", align_near, 17.0f, 90.0f);

				// ASSERT
				assert_equal(5u, target.append_log.size());
				assert_equal(17, target.append_log[0].second.x);
				assert_equal(90, target.append_log[0].second.y);
				assert_equal(c_outline_2, target.append_log[0].first->path);
				assert_equal(24, target.append_log[1].second.x);
				assert_equal(90, target.append_log[1].second.y);
				assert_equal(c_outline_2, target.append_log[1].first->path);
				assert_equal(31, target.append_log[2].second.x);
				assert_equal(90, target.append_log[2].second.y);
				assert_equal(c_outline_1, target.append_log[2].first->path);
				assert_equal(36, target.append_log[3].second.x);
				assert_equal(90, target.append_log[3].second.y);
				assert_equal(c_outline_diamond, target.append_log[3].first->path);
				assert_equal(40, target.append_log[4].second.x);
				assert_equal(92, target.append_log[4].second.y);
				assert_equal(c_outline_1, target.append_log[4].first->path);
			}


			test( CenterAndFarAlignmentsShiftTheGlyphRunAccordingly )
			{
				// INIT
				mocks::font_accessor::char_to_index font_indices[] = { { L'a', 0 }, { L'w', 1 }, { L'z', 2 }, };
				mocks::font_accessor::glyph font_glyphs[] = {
					mocks::glyph(5, 0, c_outline_1),
					mocks::glyph(7, 0, c_outline_2),
					mocks::glyph(4, 2, c_outline_diamond),
				};
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm2, font_indices, font_glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 0);
				font::ptr f = e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));
				mocks::rasterizer target;

				// ACT
				e.render_string(target, *f, "awwz", align_far, 20.0f, 0.0f);

				// ASSERT
				assert_equal(4u, target.append_log.size());
				assert_equal(-3, target.append_log[0].second.x);
				assert_equal(0, target.append_log[0].second.y);
				assert_equal(c_outline_1, target.append_log[0].first->path);
				assert_equal(2, target.append_log[1].second.x);
				assert_equal(0, target.append_log[1].second.y);
				assert_equal(c_outline_2, target.append_log[1].first->path);
				assert_equal(9, target.append_log[2].second.x);
				assert_equal(0, target.append_log[2].second.y);
				assert_equal(c_outline_2, target.append_log[2].first->path);
				assert_equal(16, target.append_log[3].second.x);
				assert_equal(0, target.append_log[3].second.y);
				assert_equal(c_outline_diamond, target.append_log[3].first->path);

				// INIT
				target.append_log.clear();

				// ACT
				e.render_string(target, *f, "awwwz", align_center, 20.0f, 7.0f);

				// ASSERT
				assert_equal(5u, target.append_log.size());
				assert_equal(5, target.append_log[0].second.x);
				assert_equal(7, target.append_log[0].second.y);
				assert_equal(c_outline_1, target.append_log[0].first->path);
				assert_equal(10, target.append_log[1].second.x);
				assert_equal(7, target.append_log[1].second.y);
				assert_equal(c_outline_2, target.append_log[1].first->path);
				assert_equal(17, target.append_log[2].second.x);
				assert_equal(7, target.append_log[2].second.y);
				assert_equal(c_outline_2, target.append_log[2].first->path);
				assert_equal(24, target.append_log[3].second.x);
				assert_equal(7, target.append_log[3].second.y);
				assert_equal(c_outline_2, target.append_log[3].first->path);
				assert_equal(31, target.append_log[4].second.x);
				assert_equal(7, target.append_log[4].second.y);
				assert_equal(c_outline_diamond, target.append_log[4].first->path);
			}


			test( OnlyGlyphsFittingIntoWidthAreDisplayed )
			{
				// INIT
				mocks::font_accessor::char_to_index font_indices[] = { { L'a', 0 }, { L'w', 1 }, { L'z', 2 }, };
				mocks::font_accessor::glyph font_glyphs[] = {
					mocks::glyph(5, 0, c_outline_1),
					mocks::glyph(7, 0, c_outline_2),
					mocks::glyph(4, 0, c_outline_diamond),
				};
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm2, font_indices, font_glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 0);
				font::ptr f = e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));
				mocks::rasterizer target;

				// ACT
				e.render_string(target, *f, "awwzwzwzaw", align_near, 0.0f, 0.0f, 23.0f);

				// ASSERT
				assert_equal(4u, target.append_log.size());

				// INIT
				target = mocks::rasterizer();

				// ACT
				e.render_string(target, *f, "awwzwzwzaw", align_far, 0.0f, 0.0f, 22.99f);

				// ASSERT
				assert_equal(3u, target.append_log.size());

				// INIT
				target = mocks::rasterizer();

				// ACT
				e.render_string(target, *f, "awwzwzwzaw", align_center, 0.0f, 0.0f, 29.99f);

				// ASSERT
				assert_equal(4u, target.append_log.size());

				// INIT
				target = mocks::rasterizer();

				// ACT
				e.render_string(target, *f, "awwzwzwzaw", align_near, 0.0f, 0.0f, 30.0f);

				// ASSERT
				assert_equal(5u, target.append_log.size());

				// INIT
				target = mocks::rasterizer();

				// ACT
				e.render_string(target, *f, "awwzwzwzaw", align_far, 0.0f, 0.0f, 56.99f);

				// ASSERT
				assert_equal(9u, target.append_log.size());
			}


			test( FarAndCenterAligmentAreHeldIfLimitedByWidth )
			{
				// INIT
				mocks::font_accessor::char_to_index font_indices[] = { { L'a', 0 }, { L'w', 1 }, { L'z', 2 }, };
				mocks::font_accessor::glyph font_glyphs[] = {
					mocks::glyph(5, 0, c_outline_1),
					mocks::glyph(8, 1, c_outline_2),
					mocks::glyph(4, 0, c_outline_diamond),
				};
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm2, font_indices, font_glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 0);
				font::ptr f = e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));
				mocks::rasterizer target;

				// ACT
				e.render_string(target, *f, "awwzwzwzaw", align_far, 40.0f, 0.0f, 21.1f);

				// ASSERT
				assert_equal(3u, target.append_log.size());
				assert_equal(19, target.append_log[0].second.x);
				assert_equal(0, target.append_log[0].second.y);
				assert_equal(c_outline_1, target.append_log[0].first->path);
				assert_equal(24, target.append_log[1].second.x);
				assert_equal(0, target.append_log[1].second.y);
				assert_equal(c_outline_2, target.append_log[1].first->path);
				assert_equal(32, target.append_log[2].second.x);
				assert_equal(1, target.append_log[2].second.y);
				assert_equal(c_outline_2, target.append_log[2].first->path);

				// INIT
				target = mocks::rasterizer();

				// ACT
				e.render_string(target, *f, "zwwawzwzaw", align_center, 40.0f, 0.0f, 33.0f);

				// ASSERT
				assert_equal(5u, target.append_log.size());
				assert_equal(23, target.append_log[0].second.x);
				assert_equal(0, target.append_log[0].second.y);
				assert_equal(c_outline_diamond, target.append_log[0].first->path);
				assert_equal(27, target.append_log[1].second.x);
				assert_equal(0, target.append_log[1].second.y);
				assert_equal(c_outline_2, target.append_log[1].first->path);
				assert_equal(35, target.append_log[2].second.x);
				assert_equal(1, target.append_log[2].second.y);
				assert_equal(c_outline_2, target.append_log[2].first->path);
				assert_equal(43, target.append_log[3].second.x);
				assert_equal(2, target.append_log[3].second.y);
				assert_equal(c_outline_1, target.append_log[3].first->path);
				assert_equal(48, target.append_log[4].second.x);
				assert_equal(2, target.append_log[4].second.y);
				assert_equal(c_outline_2, target.append_log[4].first->path);
			}


			ignored_test( ReleasedFontIsNotifiedAboutImmidiatelyWhenNoCollectionIsRequired )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { mocks::glyph(5.2, 0, c_outline_1), };
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				mocks::logging_text_engine e(loader, 0);
				font::ptr f = e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));
				void *pvf = f.get();

				// ACT
				f = font::ptr();

				// ASSERT
				void *reference[] = { pvf, };

				assert_equal(reference, e.deletion_log);
			}


			ignored_test( DestroyedFontReleasesUnderlyingAccessor )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { mocks::glyph(5.2, 0, c_outline_1), };
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 1000, regular, false, hint_none),
						mocks::font_accessor(c_fm2, indices, glyphs)),
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				mocks::logging_text_engine e(loader, 0);
				font::ptr f[] = {
					e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_none)),
					e.create_font(font_descriptor::create("Arial", 11, regular, false, hint_none)),
					e.create_font(font_descriptor::create("Arial", 11, regular, false, hint_strong)),
				};

				// ACT
				f[0] = font::ptr();

				// ASSERT
				assert_equal(2u, *loader.allocated);

				// ACT
				f[1] = font::ptr();

				// ASSERT
				assert_equal(1u, *loader.allocated);

				// ACT
				f[2] = font::ptr();

				// ASSERT
				assert_equal(0u, *loader.allocated);
			}


			ignored_test( DestroyedAccessorGetsReacquiredOnFontReCreation )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { mocks::glyph(5.2, 0, c_outline_1), };
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 1000, regular, false, hint_none),
						mocks::font_accessor(c_fm2, indices, glyphs)),
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				mocks::logging_text_engine e(loader, 0);
				font::ptr f = e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_none));

				f = font::ptr();

				// ACT
				f = e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_none));

				// ASSERT
				font_descriptor fd[] = {
					font_descriptor::create("Arial", 1000, regular, false, hint_none),
					font_descriptor::create("Arial", 1000, regular, false, hint_none),
				};

				assert_equal(fd, get_descriptors(loader.created_log));
			}


			ignored_test( ReleasedFontIsNotifiedUponOnPredefinedCollectionCycle )
			{
				// INIT
				mocks::font_accessor::char_to_index indices[] = { { L'a', 0 }, };
				mocks::font_accessor::glyph glyphs[] = { mocks::glyph(5.2, 0, c_outline_1), };
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
					make_pair(font_descriptor::create("Tahoma", 10, bold, false, hint_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				mocks::logging_text_engine e1(loader, 2);
				font::ptr f1 = e1.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));
				font::ptr f2 = e1.create_font(font_descriptor::create("Tahoma", 10, bold, false, hint_strong));
				void *pvf1 = f1.get();
				void *pvf2 = f2.get();
				mocks::logging_text_engine e2(loader, 5);
				font::ptr f3 = e2.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));
				void *pvf3 = f3.get();

				// ACT
				f1 = font::ptr();
				e1.collect();

				// ASSERT
				assert_is_empty(e1.deletion_log);
				assert_equal(3u, *loader.allocated);

				// ACT
				f2 = font::ptr();
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
				f3 = font::ptr();
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
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
					make_pair(font_descriptor::create("Tahoma", 10, bold, false, hint_strong),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				mocks::logging_text_engine e(loader, 1);
				font::ptr f1 = e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));
				font::ptr f2 = e.create_font(font_descriptor::create("Tahoma", 10, bold, false, hint_strong));
				void *pvf1 = f1.get();
				void *pvf2 = f2.get();

				loader.created_log.clear();
				f1 = font::ptr();
				f2 = font::ptr();

				// ACT
				f2 = e.create_font(font_descriptor::create("Tahoma", 10, bold, false, hint_strong));

				// ASSERT
				assert_equal(pvf2, f2.get());
				assert_is_empty(loader.created_log);

				// ACT
				f1 = e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_strong));

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
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_none),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				scoped_ptr<mocks::logging_text_engine> e(new mocks::logging_text_engine(loader, 1));
				font::ptr f1 = e->create_font(font_descriptor::create("Arial", 101, regular, false, hint_none));
				font::ptr f2 = e->create_font(font_descriptor::create("Arial", 15, regular, false, hint_none));

				f1 = font::ptr();
				f2 = font::ptr();

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
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 1000, regular, false, hint_none),
						mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<mocks::rasterizer> e(loader, 1);
				mocks::rasterizer target;
				font::ptr f1 = e.create_font(font_descriptor::create("Arial", 101, regular, false, hint_none));
				font::ptr f2 = e.create_font(font_descriptor::create("Arial", 15, regular, false, hint_none));
				font::ptr f3 = e.create_font(font_descriptor::create("Arial", 10, regular, false, hint_none));

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