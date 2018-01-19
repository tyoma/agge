#include <agge/platform/bitmap.h>

#include "surface.h"

#include <memory>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace platform
	{
		namespace tests
		{
			namespace
			{
				intptr_t row_ptr(raw_bitmap &b, count_t y)
				{	return reinterpret_cast<intptr_t>(b.row_ptr(y));	}

				intptr_t get_stride(raw_bitmap &b)
				{	return reinterpret_cast<intptr_t>(b.row_ptr(1)) - reinterpret_cast<intptr_t>(b.row_ptr(0));	}
				
				template <typename PixelT>
				void zero_bitmap(raw_bitmap &b)
				{
					for (count_t y = 0; y != b.height(); ++y)
					{
						PixelT *row = static_cast<PixelT *>(b.row_ptr(y));
						
						for (count_t x = 0; x != b.width(); ++x)
						{
							PixelT z = {};
							
							row[x] = z;
						}
					}
				}
			}

			begin_test_suite( RawBitmapTests )
				// Common Bitmap Interface
				test( CreatedBitmapPreservesSizeAttributes )
				{
					// INIT / ACT
					const raw_bitmap b1(10, 100, bpp32);
					const raw_bitmap b2(171, 731, bpp16);
					
					// ACT / ASSERT
					assert_equal(10u, b1.width());
					assert_equal(100u, b1.height());
					assert_equal(171u, b2.width());
					assert_equal(731u, b2.height());
				}


				test( CreatedBitmapAllocatesMemory )
				{
					// INIT / ACT
					raw_bitmap b1(1000, 1000, bpp32);
					raw_bitmap b2(700, 2000, bpp32);

					// ASSERT
					assert_not_null(b1.row_ptr(0));
					assert_not_null(b2.row_ptr(0));

					// ACT / ASSERT (writing memory in the end must not crash)
					memset(b1.row_ptr(999), 123, 1000);
					memset(b2.row_ptr(1999), 123, 700);
				}


				test( ConstRowPtrIsTheSameToNonConst )
				{
					// INIT
					raw_bitmap b(50, 80, bpp32);

					// ACT / ASSERT
					assert_equal(b.row_ptr(2), static_cast<const raw_bitmap &>(b).row_ptr(2));
					assert_equal(b.row_ptr(17), static_cast<const raw_bitmap &>(b).row_ptr(17));
				}


				test( BitmapAttributesAreChangedOnResize )
				{
					// INIT
					raw_bitmap b(20, 20, bpp32);

					// ACT
					b.resize(40, 20);

					// ASSERT
					assert_equal(40u, b.width());
					assert_equal(20u, b.height());

					// ACT
					b.resize(39, 50);

					// ASSERT
					assert_equal(39u, b.width());
					assert_equal(50u, b.height());

					// ACT
					b.resize(11, 13);

					// ASSERT
					assert_equal(11u, b.width());
					assert_equal(13u, b.height());
				}


				test( ResizingToALargerBitmapReallocatesBuffer )
				{
					// INIT
					raw_bitmap b(20, 20, bpp32);

					void *p = b.row_ptr(0);

					// ACT
					b.resize(40, 20);

					// ASSERT
					assert_not_equal(p, b.row_ptr(0));

					// INIT
					p = b.row_ptr(0);

					// ACT
					b.resize(40, 50);

					// ASSERT
					assert_not_equal(p, b.row_ptr(0));
				}


				test( ResizingToASmallerBitmapPreservesBuffer )
				{
					// INIT
					raw_bitmap b(20, 20, bpp32);

					void *p = b.row_ptr(0);

					// ACT
					b.resize(15, 20);

					// ASSERT
					assert_equal(p, b.row_ptr(0));

					// ACT
					b.resize(15, 15);

					// ASSERT
					assert_equal(p, b.row_ptr(0));
				}


				test( RestoringSizeToLargerPreservesBuffer )
				{
					// INIT
					raw_bitmap b(200, 200, bpp32);

					void *p = b.row_ptr(0);

					// ACT
					b.resize(15, 15);
					b.resize(200, 200);

					// ASSERT
					assert_equal(p, b.row_ptr(0));
					assert_equal(800, get_stride(b));
				}


				test( MaxingOutTheMemoryLeadsToExceptionAndPreviousStateIsPreserved )
				{
					// INIT
					count_t w = 30, h = 20;
					raw_bitmap b(w, h, bpp32);
					void *p = b.row_ptr(0);

					// ACT (must exit)
					for (;;)
					{
						const count_t new_w = 3 * w, new_h = 2 * h;

						try
						{
							b.resize(new_w, new_h);
							p = b.row_ptr(0);
						}
						catch (const bad_alloc &)
						{
							break;
						}
						w = new_w, h = new_h;
					}

					// ASSERT
					assert_equal(w, b.width());
					assert_equal(h, b.height());
					assert_equal(p, b.row_ptr(0));
					assert_equal(static_cast<int>(4 * w), get_stride(b));
				}


				// Win32 Bitmap Interface
				test( CreatedBitmapObeysWin32StrideRules )
				{
					// INIT
					raw_bitmap b1(10, 100, bpp32);
					raw_bitmap b2(171, 731, bpp16);

					// ACT / ASSERT
					assert_equal(40, get_stride(b1));
					assert_equal(120, row_ptr(b1, 3) - row_ptr(b1, 0));

					assert_equal(344, get_stride(b2));
					assert_equal(1032, row_ptr(b2, 3) - row_ptr(b2, 0));
				}


				test( BlittingToGDIContextCopiesBitmapContentToIt )
				{
					// INIT
					raw_bitmap b(5, 4, bpp32);
					pixel32 *p = static_cast<pixel32 *>(b.row_ptr(0));
					gdi_surface s(5, 4);

					zero_bitmap<pixel32>(b);
					p[0].components[1] = 0xFF;
					p[6].components[2] = 0xFF;
					p[12].components[3] = 0xFF;
					p[13].components[2] = 0xFF;
					p[19].components[1] = 0xFF;

					// ACT
					b.blit(s.lock(), 0, 0, 7, 5);
					s.unlock();

					// ASSERT
					uint8_t reference1[] = {
						0xFF,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF,
						0x00,0x00,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF,
						0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0x00,0x00,0xFF,
						0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0xFF,0x00,0x00,0xFF,
					};

					assert_is_true(reference1 == s);

					// ACT
					b.blit(s.lock(), 2, -1, 7, 5);
					s.unlock();

					// ASSERT
					uint8_t reference2[] = {
						0xFF,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF,
						0x00,0x00,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0xFF,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF,
						0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0x00,0x00,0xFF,
						0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0xFF, 0x00,0x00,0xFF,0xFF,
					};

					assert_is_true(reference2 == s);
				}


				test( BlittingToGDIContextOccursOnBoundsSpecified )
				{
					// INIT
					raw_bitmap b(10, 10, bpp32);
					gdi_surface s(5, 5);
					
					memset(b.row_ptr(0), 0xFF, 400);

					// ACT
					b.blit(s.lock(), 2, 1, 2, 3);
					s.unlock();

					// ASSERT
					uint8_t reference1[] = {
						0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
					};

					assert_is_true(reference1 == s);

					// INIT
					memset(s.data, 0x00, 100);

					// ACT
					b.blit(s.lock(), 1, 2, 4, 2);
					s.unlock();

					// ASSERT
					uint8_t reference2[] = {
						0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,
						0x00,0x00,0x00,0x00, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,
						0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
					};

					assert_is_true(reference2 == s);
				}


				test( BitmapIsAllocatedWithAtLeastRequestedExtraSpace )
				{
					// INIT / ACT
					raw_bitmap b8(7, 5, bpp8, 13);
					raw_bitmap b8_2(7, 5, bpp8, 1);
					raw_bitmap b16(6, 5, bpp16, 13);
					raw_bitmap b16_2(6, 5, bpp16, 1);
					raw_bitmap b32(7, 5, bpp32, 13);
					raw_bitmap b32_2(7, 5, bpp32, 1);

					// ASSERT
					assert_equal(20, get_stride(b8));
					assert_equal(8, get_stride(b8_2));
					assert_equal(28, get_stride(b16));
					assert_equal(16, get_stride(b16_2));
					assert_equal(44, get_stride(b32));
					assert_equal(32, get_stride(b32_2));
				}


				test( BitmapIsReAllocatedWithAtLeastExtraSpaceSpecifiedAtConstruction )
				{
					// INIT
					raw_bitmap b8(1, 5, bpp8, 2);
					raw_bitmap b16(1, 5, bpp16, 2);
					raw_bitmap b32(1, 5, bpp32, 2);

					// ACT
					b8.resize(40, 5);
					b16.resize(40, 5);
					b32.resize(40, 5);

					// ASSERT
					assert_equal(44, get_stride(b8));
					assert_equal(84, get_stride(b16));
					assert_equal(164, get_stride(b32));
				}

			end_test_suite
		}
	}
}
