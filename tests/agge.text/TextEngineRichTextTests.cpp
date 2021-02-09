#include <agge.text/text_engine.h>

#include "helpers.h"
#include "helpers_layout.h"
#include "mocks.h"
#include "outlines.h"

#include <agge/clipper.h>
#include <agge/rasterizer.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace
	{
		typedef rasterizer< clipper<int> > rasterizer_t;
		typedef text_engine_base::offset_conv offset;
		font_metrics c_fm1 = { 5.1f, 2.2f, 3.3f };
		font_metrics c_fm2 = { 10.0f, 4.0f, 2.0f };

		template <int n>
		real_t decimate(real_t value)
		{
			const int d = 1 << n;
			return static_cast<real_t>(static_cast<int>(value * d)) / d;
		}
	}

	inline bool operator ==(rasterizer_t lhs, rasterizer_t rhs)
	{
		lhs.compact();
		rhs.compact();
		if (lhs.min_y() == rhs.min_y() && lhs.height() == rhs.height() && lhs.width() == rhs.width())
		{
			for (int y = lhs.min_y(), max = y + lhs.height(); y != max; ++y)
				if (tests::mkvector(lhs[y].first, lhs[y].second) != tests::mkvector(rhs[y].first, rhs[y].second))
					return false;
			return true;
		}
		return false;
	}

	namespace tests
	{
		begin_test_suite( TextEngineRichTextTests )
			test( SingleLineTextIsAlignedAccordinglyToRequest )
			{
				mocks::font_accessor::char_to_index indices[] = {	{ L'a', 0 }, { L's', 1 }, { L't', 2 }	};
				mocks::font_accessor::glyph glyphs[] = {
					mocks::glyph(5.2, 0, c_outline_1),
					mocks::glyph(13.7, 0, c_outline_2),
					mocks::glyph(7.725, 0, c_outline_diamond),
				};
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(font_descriptor::create("Arial", 10, regular, false, hint_strong),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<rasterizer_t> e(loader, 4);
				font::ptr fnt = e.create_font(fonts[0].first);
				layout l(e);
				rasterizer_t target, reference;

				// positions: 0.0f, 5.2f, 18.9f, 26.625f, width: 34.35f
				l.process(simple_richtext("astt", "Arial", 10, regular, false, hint_strong));

				// ACT
				e.render(target, l, align_near, align_near, create_rect(17.32f, 190.0f, 50.0f, 250.0f));

				// ASSERT
				add_path(reference, offset(fnt->get_glyph(0)->get_outline(), decimate<4>(17.32f + 0.0f), decimate<4>(195.1f)));
				add_path(reference, offset(fnt->get_glyph(1)->get_outline(), decimate<4>(17.32f + 5.2f), decimate<4>(195.1f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(17.32f + 18.9f), decimate<4>(195.1f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(17.32f + 26.625f), decimate<4>(195.1f)));
				assert_equal(reference, target);

				// INIT
				reference.reset(), target.reset();

				// positions: 0.0f, 5.2f, 12.925f, width: 20.65f
				l.process(simple_richtext("att", "Arial", 10, regular, false, hint_strong));

				// ACT
				e.render(target, l, align_far, align_near, create_rect(17.32f, 191.05f, 50.0f, 250.0f));

				// ASSERT
				add_path(reference, offset(fnt->get_glyph(0)->get_outline(), decimate<4>(29.35f + 0.0f), decimate<4>(196.15f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(29.35f + 5.2f), decimate<4>(196.15f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(29.35f + 12.925f), decimate<4>(196.15f)));
				assert_equal(reference, target);

				// ACT
				e.render(target, l, align_far, align_far, create_rect(17.32f, 191.05f, 50.0f, 250.0f));

				// ASSERT
				add_path(reference, offset(fnt->get_glyph(0)->get_outline(), decimate<4>(29.35f + 0.0f), decimate<4>(247.8f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(29.35f + 5.2f), decimate<4>(247.8f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(29.35f + 12.925f), decimate<4>(247.8f)));
				assert_equal(reference, target);

				// ACT
				e.render(target, l, align_near, align_far, create_rect(0.3f, 191.05f, 50.0f, 250.0f));

				// ASSERT
				add_path(reference, offset(fnt->get_glyph(0)->get_outline(), decimate<4>(0.3f + 0.0f), decimate<4>(247.8f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(0.3f + 5.2f), decimate<4>(247.8f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(0.3f + 12.925f), decimate<4>(247.8f)));
				assert_equal(reference, target);

				// ACT
				e.render(target, l, align_near, align_center, create_rect(0.3f, 191.05f, 50.0f, 250.0f));

				// ASSERT
				add_path(reference, offset(fnt->get_glyph(0)->get_outline(), decimate<4>(0.3f + 0.0f), decimate<4>(221.975f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(0.3f + 5.2f), decimate<4>(221.975f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(0.3f + 12.925f), decimate<4>(221.975f)));
				assert_equal(reference, target);

				// ACT
				e.render(target, l, align_center, align_center, create_rect(0.3f, 191.05f, 50.0f, 252.0f));

				// ASSERT
				add_path(reference, offset(fnt->get_glyph(0)->get_outline(), decimate<4>(14.825f + 0.0f), decimate<4>(222.975f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(14.825f + 5.2f), decimate<4>(222.975f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(14.825f + 12.925f), decimate<4>(222.975f)));
				assert_equal(reference, target);
			}


			test( MultilineRichTextIsDrawnAccordinglyToRectAndPosition )
			{
				mocks::font_accessor::char_to_index indices[] = {	{ L'a', 0 }, { L's', 1 }, { L't', 2 }	};
				mocks::font_accessor::glyph glyphs[] = {
					mocks::glyph(5.2, 0, c_outline_1),
					mocks::glyph(13.7, 0, c_outline_2),
					mocks::glyph(7.725, 0, c_outline_diamond),
				};
				font_descriptor d[] = {
					font_descriptor::create("Arial", 10, regular, false, hint_strong),
					font_descriptor::create("Arial", 15, regular, false, hint_strong),
				};
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(d[0], mocks::font_accessor(c_fm1, indices, glyphs)),
					make_pair(d[1], mocks::font_accessor(c_fm2, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<rasterizer_t> e(loader, 4);
				font::ptr f1 = e.create_font(fonts[0].first);
				font::ptr f2 = e.create_font(fonts[1].first);
				rasterizer_t target, reference;
				richtext_t text;
				font_style_annotation a = {};

				a.basic.weight = regular;
				a.basic.hinting = hint_strong;
				text.set_base_annotation(a);

				// w: 26.625, h: 24.6
				text << style::family("Arial") << style::height(10) << "as\n"	// 18.9 x (7.3 + 3.3)
					<< style::family("Arial") << style::height(15) << "sat";	// 26.625 x (14 + 2)

				// ACT / ASSERT
				assert_equal(create_box(26.625f, 24.6f), e.measure(text));

				// ACT (baseline1 = 80.5, x_line1 = 81.1, baseline2 = 96, x_line2 = 73.375)
				e.render(target, text, align_far, align_far, create_rect(10.0f, 10.0f, 100.0f, 100.0f));

				// ASSERT
				add_path(reference, offset(f1->get_glyph(0)->get_outline(), decimate<4>(81.1f + 0.0f), decimate<4>(80.5f)));
				add_path(reference, offset(f1->get_glyph(1)->get_outline(), decimate<4>(81.1f + 5.2f), decimate<4>(80.5f)));
				add_path(reference, offset(f2->get_glyph(1)->get_outline(), decimate<4>(73.375f + 0.0f), decimate<4>(96.0f)));
				add_path(reference, offset(f2->get_glyph(0)->get_outline(), decimate<4>(73.375f + 13.7f), decimate<4>(96.0f)));
				add_path(reference, offset(f2->get_glyph(2)->get_outline(), decimate<4>(73.375f + 18.9f), decimate<4>(96.0f)));
				assert_equal(reference, target);

				// INIT
				reference.reset();
				target.reset();

				// ACT (baseline1 = 47.8, baseline2 = 63.3)
				e.render(target, text, align_near, align_center, create_rect(10.0f, 10.0f, 100.0f, 100.0f));

				// ASSERT
				add_path(reference, offset(f1->get_glyph(0)->get_outline(), decimate<4>(10.0f + 0.0f), decimate<4>(47.8f)));
				add_path(reference, offset(f1->get_glyph(1)->get_outline(), decimate<4>(10.0f + 5.2f), decimate<4>(47.8f)));
				add_path(reference, offset(f2->get_glyph(1)->get_outline(), decimate<4>(10.0f + 0.0f), decimate<4>(63.3f)));
				add_path(reference, offset(f2->get_glyph(0)->get_outline(), decimate<4>(10.0f + 13.7f), decimate<4>(63.3f)));
				add_path(reference, offset(f2->get_glyph(2)->get_outline(), decimate<4>(10.0f + 18.9f), decimate<4>(63.3f)));
				assert_equal(reference, target);
			}


			test( LayoutIsLimitedWhenRenderingTextIntoRect )
			{
				mocks::font_accessor::char_to_index indices[] = {	{ L'a', 0 },	};
				mocks::font_accessor::glyph glyphs[] = {
					mocks::glyph(7.2, 0, c_outline_1),
				};
				font_descriptor d = font_descriptor::create("Arial", 10, regular, false, hint_strong);
				pair<font_descriptor, mocks::font_accessor> fonts[] = {
					make_pair(d, mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<rasterizer_t> e(loader, 4);
				font::ptr f = e.create_font(fonts[0].first);
				rasterizer_t target, reference;
				richtext_t text;
				font_style_annotation a = {	d,	};

				text.set_base_annotation(a);
				text.append("aaaaa"); // w: 36

				// ACT / ASSERT
				assert_equal(create_box(36.0f, 7.3f), e.measure(text));

				// ACT
				e.render(target, text, align_near, align_near, create_rect(0.0f, 0.0f, 35.9f, 100.0f));

				// ASSERT
				add_path(reference, offset(f->get_glyph(0)->get_outline(), decimate<4>(0.0f), decimate<4>(5.1f)));
				add_path(reference, offset(f->get_glyph(0)->get_outline(), decimate<4>(7.2f), decimate<4>(5.1f)));
				add_path(reference, offset(f->get_glyph(0)->get_outline(), decimate<4>(14.4f), decimate<4>(5.1f)));
				add_path(reference, offset(f->get_glyph(0)->get_outline(), decimate<4>(21.6f), decimate<4>(5.1f)));
				add_path(reference, offset(f->get_glyph(0)->get_outline(), decimate<4>(0.0f), decimate<4>(15.7f)));
				assert_equal(reference, target);

				// INIT
				reference.reset();
				target.reset();

				// ACT
				e.render(target, text, align_near, align_near, create_rect(0.0f, 0.0f, 28.7f, 100.0f));

				// ASSERT
				add_path(reference, offset(f->get_glyph(0)->get_outline(), decimate<4>(0.0f), decimate<4>(5.1f)));
				add_path(reference, offset(f->get_glyph(0)->get_outline(), decimate<4>(7.2f), decimate<4>(5.1f)));
				add_path(reference, offset(f->get_glyph(0)->get_outline(), decimate<4>(14.4f), decimate<4>(5.1f)));
				add_path(reference, offset(f->get_glyph(0)->get_outline(), decimate<4>(0.0f), decimate<4>(15.7f)));
				add_path(reference, offset(f->get_glyph(0)->get_outline(), decimate<4>(7.2f), decimate<4>(15.7f)));
				assert_equal(reference, target);

				// ACT / ASSERT (previous limits do not affect measurements)
				assert_equal(create_box(36.0f, 7.3f), e.measure(text));
			}
		end_test_suite
	}
}
