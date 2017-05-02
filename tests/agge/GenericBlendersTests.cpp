#include <agge/blenders_generic.h>

#include <algorithm>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			template <typename OrderT>
			struct pixel_rgbx
			{
				uint8_t components[4];

				bool operator ==(const pixel_rgbx &rhs) const
				{
					return components[OrderT::R] == rhs.components[OrderT::R] && components[OrderT::G] == rhs.components[OrderT::G]
						&& components[OrderT::B] == rhs.components[OrderT::B];
				}
			};
		}

		begin_test_suite( GenericRGBXBlenderTests )
			test( BlendingWhiteOnBlackProducesGrayDependingOnCover )
			{
				typedef mocks::pixel_rgbx<order_rgba> pixel;

				// INIT
				const int n = 7;
				const pixel black = { 0 };
				const uint8_t covers[n] = { 0x00, 0x12, 0x21, 0x71, 0x81, 0xF0, 0xFF };
				blender_solid_color_rgb<pixel, order_rgba> whiteness(0xFF, 0xFF, 0xFF);
				pixel dest[n];

				fill_n(dest, n, black);

				// ACT
				whiteness(dest + 0, 0, 0, 1, covers + 0);
				whiteness(dest + 1, 0, 0, 1, covers + 1);
				whiteness(dest + 2, 0, 0, 1, covers + 2);
				whiteness(dest + 3, 0, 0, 1, covers + 3);
				whiteness(dest + 4, 0, 0, 1, covers + 4);
				whiteness(dest + 5, 0, 0, 1, covers + 5);
				whiteness(dest + 6, 0, 0, 1, covers + 6);

				// ASSERT
				pixel reference[] = {
					{ 0x00, 0x00, 0x00, 0x00 },
					{ 0x11, 0x11, 0x11, 0x11 },
					{ 0x20, 0x20, 0x20, 0x20 },
					{ 0x70, 0x70, 0x70, 0x70 },
					{ 0x80, 0x80, 0x80, 0x80 },
					{ 0xEF, 0xEF, 0xEF, 0xEF },
					{ 0xFF, 0xFF, 0xFF, 0xFF },
				};

				assert_equal(reference, dest);
			}


			test( BlendingBlackOnWhiteProducesGrayDependingOnInvertedCover )
			{
				typedef mocks::pixel_rgbx<order_rgba> pixel;

				// INIT
				const int n = 7;
				const pixel white = { 0xFF, 0xFF, 0xFF, 0xFF };
				const uint8_t covers[n] = { 0x00, 0x12, 0x21, 0x71, 0x81, 0xF0, 0xFF };
				blender_solid_color_rgb<pixel, order_rgba> blackness(0x00, 0x00, 0x00);
				pixel dest[n];

				fill_n(dest, n, white);

				// ACT
				blackness(dest + 0, 0, 0, 1, covers + 0);
				blackness(dest + 1, 0, 0, 1, covers + 1);
				blackness(dest + 2, 0, 0, 1, covers + 2);
				blackness(dest + 3, 0, 0, 1, covers + 3);
				blackness(dest + 4, 0, 0, 1, covers + 4);
				blackness(dest + 5, 0, 0, 1, covers + 5);
				blackness(dest + 6, 0, 0, 1, covers + 6);

				// ASSERT
				pixel reference[] = {
					{ 0xFF, 0xFF, 0xFF, 0xFF },
					{ 0xED, 0xED, 0xED, 0xED },
					{ 0xDE, 0xDE, 0xDE, 0xDE },
					{ 0x8E, 0x8E, 0x8E, 0x8E },
					{ 0x7E, 0x7E, 0x7E, 0x7E },
					{ 0x0F, 0x0F, 0x0F, 0x0F },
					{ 0x00, 0x00, 0x00, 0x00 },
				};

				assert_equal(reference, dest);
			}


			test( BlendingFullyOpaquePixelIsCopied )
			{
				typedef mocks::pixel_rgbx<order_rgba> pixel1;
				typedef mocks::pixel_rgbx<order_bgra> pixel2;
				typedef mocks::pixel_rgbx<order_argb> pixel3;
				typedef mocks::pixel_rgbx<order_abgr> pixel4;

				// INIT
				const uint8_t cover = 0xFF;
				blender_solid_color_rgb<pixel1, order_rgba> b1(0x10, 0x20, 0x30);
				blender_solid_color_rgb<pixel2, order_bgra> b2(0x10, 0x20, 0x30);
				blender_solid_color_rgb<pixel3, order_argb> b3(0x10, 0x20, 0x30);
				blender_solid_color_rgb<pixel4, order_abgr> b4(0x10, 0x20, 0x30);
				pixel1 dest1 = { 0 };
				pixel2 dest2 = { 0 };
				pixel3 dest3 = { 0 };
				pixel4 dest4 = { 0 };

				// ACT
				b1(&dest1, 0, 0, 1, &cover);
				b2(&dest2, 0, 0, 1, &cover);
				b3(&dest3, 0, 0, 1, &cover);
				b4(&dest4, 0, 0, 1, &cover);

				// ASSERT
				pixel1 reference1 = { 0x10, 0x20, 0x30 };
				pixel2 reference2 = { 0x30, 0x20, 0x10 };
				pixel3 reference3 = { 0, 0x10, 0x20, 0x30 };
				pixel4 reference4 = { 0, 0x30, 0x20, 0x10 };

				assert_equal(reference1, dest1);
				assert_equal(reference2, dest2);
				assert_equal(reference3, dest3);
				assert_equal(reference4, dest4);
			}


			test( BlendingHalfOpaquePixelIsBlended )
			{
				typedef mocks::pixel_rgbx<order_rgba> pixel1;
				typedef mocks::pixel_rgbx<order_bgra> pixel2;
				typedef mocks::pixel_rgbx<order_argb> pixel3;
				typedef mocks::pixel_rgbx<order_abgr> pixel4;

				// INIT
				const uint8_t cover = 0xA0;
				blender_solid_color_rgb<pixel1, order_rgba> b1(0x10, 0x20, 0x30);
				blender_solid_color_rgb<pixel2, order_bgra> b2(0x10, 0x20, 0x30);
				blender_solid_color_rgb<pixel3, order_argb> b3(0x10, 0x20, 0x30);
				blender_solid_color_rgb<pixel4, order_abgr> b4(0x10, 0x20, 0x30);
				pixel1 dest1 = { 0x25, 0x47, 0x9A, 0xAB };
				pixel2 dest2 = { 0x25, 0x47, 0x9A, 0xAB };
				pixel3 dest3 = { 0x25, 0x47, 0x9A, 0xAB };
				pixel4 dest4 = { 0x25, 0x47, 0x9A, 0xAB };

				// ACT
				b1(&dest1, 0, 0, 1, &cover);
				b2(&dest2, 0, 0, 1, &cover);
				b3(&dest3, 0, 0, 1, &cover);
				b4(&dest4, 0, 0, 1, &cover);

				// ASSERT
				pixel1 reference1 = { 0x17, 0x2E, 0x57 };
				pixel2 reference2 = { 0x2B, 0x2E, 0x43 };
				pixel3 reference3 = { 0, 0x24, 0x4D, 0x5D };
				pixel4 reference4 = { 0, 0x38, 0x4D, 0x49 };

				assert_equal(reference1, dest1);
				assert_equal(reference2, dest2);
				assert_equal(reference3, dest3);
				assert_equal(reference4, dest4);
			}


			test( BlendingSeriesOfCoversIsSupported )
			{
				typedef mocks::pixel_rgbx<order_rgba> pixel;

				// INIT
				const int n = 3;
				const pixel white = { 0x80, 0x80, 0x80, 0x80 };
				const uint8_t covers[n] = { 0x10, 0x90, 0xB8, };
				blender_solid_color_rgb<pixel, order_rgba> b(0x70, 0xD0, 0x90);
				pixel dest[n];

				fill_n(dest, n, white);

				// ACT
				b(dest, 0, 0, n, covers);

				// ASSERT
				pixel reference[] = {
					{ 0x7E, 0x85, 0x81, },
					{ 0x76, 0xAD, 0x89, },
					{ 0x74, 0xB9, 0x8B, },
				};

				assert_equal(reference, dest);
			}


			test( AlphaIsMixedFromSrcAlphaAndCover )
			{
				typedef mocks::pixel_rgbx<order_rgba> pixel;

				// INIT
				const int n = 4;
				const pixel back = { 0x20, 0x80, 0xE0, 0x80 };
				const uint8_t covers[n] = { 0x10, 0x90, 0xA0, 0xD0, };
				blender_solid_color_rgb<pixel, order_rgba> b1(0x80, 0x80, 0x80, 0x30);
				blender_solid_color_rgb<pixel, order_rgba> b2(0x80, 0x80, 0x80, 0xC0);
				pixel dest1[n];
				pixel dest2[n];

				fill_n(dest1, n, back);
				fill_n(dest2, n, back);

				// ACT
				b1(dest1, 0, 0, n, covers);
				b2(dest2, 0, 0, n, covers);

				// ASSERT
				pixel reference1[] = {
					{ 0x21, 0x80, 0xDE, },
					{ 0x2A, 0x80, 0xD5, },
					{ 0x2B, 0x80, 0xD4, },
					{ 0x2E, 0x80, 0xD1, },
				};
				pixel reference2[] = {
					{ 0x24, 0x80, 0xDB, },
					{ 0x48, 0x80, 0xB7, },
					{ 0x4D, 0x80, 0xB2, },
					{ 0x5A, 0x80, 0xA5, },
				};

				assert_equal(reference1, dest1);
				assert_equal(reference2, dest2);
			}


			test( PixelCopyAppliesRequiredAmountOfPixels )
			{
				typedef mocks::pixel_rgbx<order_rgba> pixel;

				// INIT
				pixel dest[5] = { 0 };
				blender_solid_color_rgb<pixel, order_rgba> b1(0x80, 0x80, 0x80, 0x00);
				blender_solid_color_rgb<pixel, order_rgba> b2(0x14, 0x14, 0x14, 0x00);

				// ACT
				b1(dest, 0, 0, 3);

				// ASSERT
				pixel reference1[] = {
					{ 0x80, 0x80, 0x80, },
					{ 0x80, 0x80, 0x80, },
					{ 0x80, 0x80, 0x80, },
					{ 0x00, 0x00, 0x00, },
					{ 0x00, 0x00, 0x00, },
				};

				assert_equal(reference1, dest);

				// ACT
				b2(dest, 0, 0, 2);

				// ASSERT
				pixel reference2[] = {
					{ 0x14, 0x14, 0x14, },
					{ 0x14, 0x14, 0x14, },
					{ 0x80, 0x80, 0x80, },
					{ 0x00, 0x00, 0x00, },
					{ 0x00, 0x00, 0x00, },
				};

				assert_equal(reference2, dest);

				// ACT
				b1(dest, 0, 0, 5);

				// ASSERT
				pixel reference3[] = {
					{ 0x80, 0x80, 0x80, },
					{ 0x80, 0x80, 0x80, },
					{ 0x80, 0x80, 0x80, },
					{ 0x80, 0x80, 0x80, },
					{ 0x80, 0x80, 0x80, },
				};

				assert_equal(reference3, dest);
			}


			test( PixelComponentsAreSetAccordinglyToOrderOnCopy )
			{
				typedef mocks::pixel_rgbx<order_bgra> pixel1;
				typedef mocks::pixel_rgbx<order_argb> pixel2;

				// INIT
				pixel1 dest1 = { 0 };
				pixel2 dest2 = { 0 };
				blender_solid_color_rgb<pixel1, order_bgra> b1(0x01, 0x02, 0x03);
				blender_solid_color_rgb<pixel2, order_argb> b2(0x11, 0x12, 0x13);

				// ACT
				b1(&dest1, 0, 0, 1);
				b2(&dest2, 0, 0, 1);

				// ASSERT
				pixel1 reference1 = { 0x03, 0x02, 0x01, };
				pixel2 reference2 = { 0, 0x11, 0x12, 0x13, };

				assert_equal(reference1, dest1);
				assert_equal(reference2, dest2);
			}

		end_test_suite
	}
}
