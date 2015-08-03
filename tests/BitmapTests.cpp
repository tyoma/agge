#include <agge/bitmap.h>

#include "helpers.h"
#include "mocks.h"

#include <vector>

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			class raw_bitmap
			{
			public:
				raw_bitmap(count_t width, count_t height, bits_per_pixel bpp)
					: _width(width), _height(height), _bpp(bpp), data(width * height * ((int)bpp / 8))
				{	}

				count_t width() const
				{	return _width;	}

				count_t height() const
				{	return _height;	}

				bits_per_pixel bpp() const
				{	return _bpp;	}

				void *row_ptr(count_t y)
				{	return &data[y * _width * ((int)_bpp / 8)];	}

				const void *row_ptr(count_t y) const
				{	return &data[y * _width * ((int)_bpp / 8)];	}

				vector<uint8_t> data;

			public:
				count_t _width, _height;
				bits_per_pixel _bpp;
			};
		}

		begin_test_suite( BitmapTests )
			test( WidthAndHeightArePassedThroughToRawBitmapOnCreation )
			{
				// INIT / ACT
				bitmap<pixel32, mocks::raw_bitmap> b1(640, 480);
				bitmap<pixel32, mocks::raw_bitmap> b2(1200, 800);

				// ASSERT
				assert_equal(640u, static_cast<mocks::raw_bitmap &>(b1).width());
				assert_equal(480u, static_cast<mocks::raw_bitmap &>(b1).height());
				assert_equal(1200u, b2.width());
				assert_equal(800u, b2.height());
			}

			
			test( ProperBPPIsPassedToConstructor )
			{
				// INIT / ACT
				bitmap<pixel32, mocks::raw_bitmap> b1(10, 10);
				bitmap<pixel24, mocks::raw_bitmap> b2(10, 10);
				bitmap<pixel16, mocks::raw_bitmap> b3(10, 10);
				bitmap<uint8_t, mocks::raw_bitmap> b4(10, 10);

				// ASSERT
				assert_equal(bpp32, static_cast<mocks::raw_bitmap &>(b1).bpp());
				assert_equal(bpp24, b2.bpp());
				assert_equal(bpp16, b3.bpp());
				assert_equal(bpp8, b4.bpp());
			}


			test( RowPtrIsRepresentedWithPixelPointer )
			{
				// INIT
				typedef bitmap<pixel32, mocks::raw_bitmap> bitmap32;
				typedef bitmap<pixel24, mocks::raw_bitmap> bitmap24;

				bitmap32 b1(10, 10);
				bitmap24 b2(10, 10);

				// ACT
				pixel32 *p1 = static_cast<bitmap32::pixel *>(b1.row_ptr(3));
				pixel32 *p2 = b1.row_ptr(4);
				pixel24 *p3 = static_cast<bitmap24::pixel *>(b2.row_ptr(7));
				pixel24 *p4 = b2.row_ptr(1);

				// ASSERT
				assert_equal(p1, static_cast<mocks::raw_bitmap &>(b1).row_ptr(3));
				assert_equal(p2, static_cast<mocks::raw_bitmap &>(b1).row_ptr(4));
				assert_equal(p3, static_cast<mocks::raw_bitmap &>(b2).row_ptr(7));
				assert_equal(p4, static_cast<mocks::raw_bitmap &>(b2).row_ptr(1));
			}


			test( ConstRowPtrIsRepresentedWithPixelPointer )
			{
				// INIT
				typedef bitmap<pixel32, mocks::raw_bitmap> bitmap32;
				typedef bitmap<pixel24, mocks::raw_bitmap> bitmap24;

				const bitmap32 b1(10, 10);
				const bitmap24 b2(10, 10);

				// ACT
				const pixel32 *p1 = static_cast<const bitmap32::pixel *>(b1.row_ptr(3));
				const pixel32 *p2 = b1.row_ptr(4);
				const pixel24 *p3 = static_cast<const bitmap24::pixel *>(b2.row_ptr(7));
				const pixel24 *p4 = b2.row_ptr(1);

				// ASSERT
				assert_equal(p1, static_cast<const mocks::raw_bitmap &>(b1).row_ptr(3));
				assert_equal(p2, static_cast<const mocks::raw_bitmap &>(b1).row_ptr(4));
				assert_equal(p3, static_cast<const mocks::raw_bitmap &>(b2).row_ptr(7));
				assert_equal(p4, static_cast<const mocks::raw_bitmap &>(b2).row_ptr(1));
			}


			test( BitmapCopyCopiesPixelsRowWise )
			{
				// INIT
				typedef bitmap<pixel32, mocks::raw_bitmap> bitmap32;
				
				bitmap32 source(4, 5);
				mocks::bitmap<pixel32, 0> destination(5, 6);
				const bitmap32 &csource = source;

				uint8_t data1[] = {
					0xFF,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
					0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0xFF,0x00,0x00, 0x00,0x00,0x00,0x00,
					0x00,0x00,0x00,0xFF, 0x00,0xCE,0xFF,0x00, 0xFF,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
					0x00,0x00,0xFF,0x00, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
					0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0xFF,0x00,0x00,0xFF,
				};

				source.data = mkvector(data1);
				destination.data[4].c0 = 0xCD;
				destination.data[9].c1 = 0xCD;

				// ACT
				copy(csource, 0, 0, destination, 0, 0, 4, 5);

				// ASSERT
				pixel32 reference1[] = {
					{ 0xFF,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x00 }, { 0xCD,0x00,0x00,0x00 },
					{ 0x00,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x00 }, { 0x00,0xFF,0x00,0x00 }, { 0x00,0x00,0x00,0x00 }, { 0x00,0xCD,0x00,0x00 },
					{ 0x00,0x00,0x00,0xFF }, { 0x00,0xCE,0xFF,0x00 }, { 0xFF,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x00 },
					{ 0x00,0x00,0xFF,0x00 }, { 0x00,0x00,0x00,0xFF }, { 0x00,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x00 },
					{ 0x00,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x00 }, { 0xFF,0x00,0x00,0xFF }, { 0x00,0x00,0x00,0x00 },
					{ 0x00,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x00 },
				};

				assert_equal(reference1, destination.data);
			}


		end_test_suite
	}
}
