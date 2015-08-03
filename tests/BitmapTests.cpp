#include <agge/bitmap.h>

#include <vector>

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			class raw_bitmap
			{
			public:
				raw_bitmap(count_t width, count_t height, bits_per_pixel bpp)
					: _width(width), _height(height), _bpp(bpp), _data(width * height * ((int)bpp / 8))
				{	}

				count_t width() const
				{	return _width;	}

				count_t height() const
				{	return _height;	}

				bits_per_pixel bpp() const
				{	return _bpp;	}

				void *row_ptr(count_t y)
				{	return &_data[y * _width * ((int)_bpp / 8)];	}

				const void *row_ptr(count_t y) const
				{	return &_data[y * _width * ((int)_bpp / 8)];	}

			public:
				count_t _width, _height;
				bits_per_pixel _bpp;
				vector<uint8_t> _data;
			};
		}

		begin_test_suite( BitmapTests )
			test( WidthAndHeightArePassedThroughToRawBitmapOnCreation )
			{
				// INIT / ACT
				bitmap<pixel32, raw_bitmap> b1(640, 480);
				bitmap<pixel32, raw_bitmap> b2(1200, 800);

				// ASSERT
				assert_equal(640u, static_cast<raw_bitmap &>(b1).width());
				assert_equal(480u, static_cast<raw_bitmap &>(b1).height());
				assert_equal(1200u, b2.width());
				assert_equal(800u, b2.height());
			}

			
			test( ProperBPPIsPassedToConstructor )
			{
				// INIT / ACT
				bitmap<pixel32, raw_bitmap> b1(10, 10);
				bitmap<pixel24, raw_bitmap> b2(10, 10);
				bitmap<pixel16, raw_bitmap> b3(10, 10);
				bitmap<uint8_t, raw_bitmap> b4(10, 10);

				// ASSERT
				assert_equal(bpp32, static_cast<raw_bitmap &>(b1).bpp());
				assert_equal(bpp24, b2.bpp());
				assert_equal(bpp16, b3.bpp());
				assert_equal(bpp8, b4.bpp());
			}


			test( RowPtrIsRepresentedWithPixelPointer )
			{
				// INIT
				typedef bitmap<pixel32, raw_bitmap> bitmap32;
				typedef bitmap<pixel24, raw_bitmap> bitmap24;

				bitmap32 b1(10, 10);
				bitmap24 b2(10, 10);

				// ACT
				pixel32 *p1 = static_cast<bitmap32::pixel *>(b1.row_ptr(3));
				pixel32 *p2 = b1.row_ptr(4);
				pixel24 *p3 = static_cast<bitmap24::pixel *>(b2.row_ptr(7));
				pixel24 *p4 = b2.row_ptr(1);

				// ASSERT
				assert_equal(p1, static_cast<raw_bitmap &>(b1).row_ptr(3));
				assert_equal(p2, static_cast<raw_bitmap &>(b1).row_ptr(4));
				assert_equal(p3, static_cast<raw_bitmap &>(b2).row_ptr(7));
				assert_equal(p4, static_cast<raw_bitmap &>(b2).row_ptr(1));
			}


			test( ConstRowPtrIsRepresentedWithPixelPointer )
			{
				// INIT
				typedef bitmap<pixel32, raw_bitmap> bitmap32;
				typedef bitmap<pixel24, raw_bitmap> bitmap24;

				const bitmap32 b1(10, 10);
				const bitmap24 b2(10, 10);

				// ACT
				const pixel32 *p1 = static_cast<const bitmap32::pixel *>(b1.row_ptr(3));
				const pixel32 *p2 = b1.row_ptr(4);
				const pixel24 *p3 = static_cast<const bitmap24::pixel *>(b2.row_ptr(7));
				const pixel24 *p4 = b2.row_ptr(1);

				// ASSERT
				assert_equal(p1, static_cast<const raw_bitmap &>(b1).row_ptr(3));
				assert_equal(p2, static_cast<const raw_bitmap &>(b1).row_ptr(4));
				assert_equal(p3, static_cast<const raw_bitmap &>(b2).row_ptr(7));
				assert_equal(p4, static_cast<const raw_bitmap &>(b2).row_ptr(1));
			}
		end_test_suite
	}
}
