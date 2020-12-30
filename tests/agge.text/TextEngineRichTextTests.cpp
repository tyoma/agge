#include <agge.text/text_engine.h>

#include "helpers.h"
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
		font::metrics c_fm1 = { 5.1f, 2.2f, 3.3f };

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
				pair<font::key, mocks::font_accessor> fonts[] = {
					make_pair(font::key(L"Arial", 10, false, false, font::key::gf_strong),
						mocks::font_accessor(c_fm1, indices, glyphs)),
				};
				mocks::fonts_loader loader(fonts);
				text_engine<rasterizer_t> e(loader, 4);
				font::ptr fnt = e.create_font(L"Arial", 10, false, false, font::key::gf_strong);
				layout l(fnt);
				rasterizer_t target, reference;

				// positions: 0.0f, 5.2f, 18.9f, 26.625f, width: 34.35f
				l.process(L"astt");

				// ACT
				e.render(target, l, near_, near_, create_rect(17.32f, 190.0f, 50.0f, 250.0f));

				// ASSERT
				add_path(reference, offset(fnt->get_glyph(0)->get_outline(), decimate<4>(17.32f + 0.0f), decimate<4>(195.1f)));
				add_path(reference, offset(fnt->get_glyph(1)->get_outline(), decimate<4>(17.32f + 5.2f), decimate<4>(195.1f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(17.32f + 18.9f), decimate<4>(195.1f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(17.32f + 26.625f), decimate<4>(195.1f)));
				assert_equal(reference, target);

				// INIT
				reference.reset(), target.reset();

				// positions: 0.0f, 5.2f, 12.925f, width: 20.65f
				l.process(L"att");

				// ACT
				e.render(target, l, far_, near_, create_rect(17.32f, 191.05f, 50.0f, 250.0f));

				// ASSERT
				add_path(reference, offset(fnt->get_glyph(0)->get_outline(), decimate<4>(29.35f + 0.0f), decimate<4>(196.15f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(29.35f + 5.2f), decimate<4>(196.15f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(29.35f + 12.925f), decimate<4>(196.15f)));
				assert_equal(reference, target);

				// ACT
				e.render(target, l, far_, far_, create_rect(17.32f, 191.05f, 50.0f, 250.0f));

				// ASSERT
				add_path(reference, offset(fnt->get_glyph(0)->get_outline(), decimate<4>(29.35f + 0.0f), decimate<4>(247.8f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(29.35f + 5.2f), decimate<4>(247.8f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(29.35f + 12.925f), decimate<4>(247.8f)));
				assert_equal(reference, target);

				// ACT
				e.render(target, l, near_, far_, create_rect(0.3f, 191.05f, 50.0f, 250.0f));

				// ASSERT
				add_path(reference, offset(fnt->get_glyph(0)->get_outline(), decimate<4>(0.3f + 0.0f), decimate<4>(247.8f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(0.3f + 5.2f), decimate<4>(247.8f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(0.3f + 12.925f), decimate<4>(247.8f)));
				assert_equal(reference, target);

				// ACT
				e.render(target, l, near_, center, create_rect(0.3f, 191.05f, 50.0f, 250.0f));

				// ASSERT
				add_path(reference, offset(fnt->get_glyph(0)->get_outline(), decimate<4>(0.3f + 0.0f), decimate<4>(221.975f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(0.3f + 5.2f), decimate<4>(221.975f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(0.3f + 12.925f), decimate<4>(221.975f)));
				assert_equal(reference, target);

				// ACT
				e.render(target, l, center, center, create_rect(0.3f, 191.05f, 50.0f, 252.0f));

				// ASSERT
				add_path(reference, offset(fnt->get_glyph(0)->get_outline(), decimate<4>(14.825f + 0.0f), decimate<4>(222.975f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(14.825f + 5.2f), decimate<4>(222.975f)));
				add_path(reference, offset(fnt->get_glyph(2)->get_outline(), decimate<4>(14.825f + 12.925f), decimate<4>(222.975f)));
				assert_equal(reference, target);
			}
		end_test_suite
	}
}
