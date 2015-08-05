#include <agge/renderer_parallel.h>

#include "helpers.h"
#include "mocks.h"
#include "mt.h"

#include <map>
#include <utee/ut/assert.h>
#include <utee/ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			typedef vector<thread_id> thread_mapping;

			uint8_t map_thread(thread_mapping &mapping, mutex &mtx)
			{
				const thread_id tid = this_thread_id();

				mtx.lock();
				for (uint8_t i = 0; i < mapping.size(); ++i)
					if (tid == mapping[i])
					{
						mtx.unlock();
						return i;
					}
				uint8_t threadnum = static_cast<uint8_t>(mapping.size());
				mapping.push_back(tid);
				mtx.unlock();
				return threadnum;
			}

			template <typename CoverT, typename BitmapT>
			int get_scanline_thread(BitmapT &b, int y)
			{
				typename BitmapT::pixel *p = b.row_ptr(y);
				int threadnum = -1;

				const uint8_t offset = sizeof(CoverT) * 8;
				const BitmapT::pixel mask = (1 << offset) - 1;

				for (count_t i = 0; i != b.width(); ++i, ++p)
					if (*p & mask)
					{
						int pixel_threadnum = *p >> offset;
						
						if (threadnum == -1)
							threadnum = pixel_threadnum;
						assert_equal(threadnum, pixel_threadnum);
					}
				return threadnum;
			}
		}

		namespace mocks
		{
			template <typename PixelT, typename CoverT>
			class thread_mapping_blender
			{
			public:
				typedef PixelT pixel;
				typedef CoverT cover_type;

			public:
				thread_mapping_blender(thread_mapping &mapping, mutex &mtx)
					: _mapping(mapping), _mtx(mtx)
				{	}

				void operator ()(PixelT *pixels, int /*x*/, int /*y*/, count_t length, const cover_type *covers) const
				{
					for (; length; --length, ++pixels, ++covers)
						*pixels = static_cast<pixel>(static_cast<int>(*covers)
							+ (map_thread(_mapping, _mtx) << sizeof(cover_type) * 8));
				}

			private:
				const thread_mapping_blender &operator =(const thread_mapping_blender &rhs);

			private:
				thread_mapping &_mapping;
				mutex &_mtx;
			};
		}

		begin_test_suite( RendererParallelTests )
			test( SingleThreadedRenderingIsDoneInSingleThread )
			{
				// INIT
				const mocks::cell cells1[] = { { 0, 0, 0x11 }, { 2, 0, -0x03 }, { 4, 0, -0x0E }, };
				const mocks::cell cells2[] = { { 1, 0, 0xAB }, { 2, 0, -0x1E }, { 5, 0, -0x8D }, };
				const mocks::cell cells3[] = { { 1, 0, 0xA0 }, { 9, 0, -0xA0 }, };
				mocks::mask<8>::scanline_cells cells[] = {
					make_pair(begin(cells1), end(cells1)),
					make_pair(begin(cells2), end(cells2)),
					make_pair(begin(cells3), end(cells3)),
				};
				mocks::mask_full<8> mask(cells, 1);
				mocks::bitmap<uint16_t> bitmap(5, 4);
				thread_mapping mapping;
				mutex mtx;
				mocks::thread_mapping_blender<uint16_t, uint8_t> blender(mapping, mtx);

				renderer_parallel r(1);

				// ACT
				r(bitmap, 0, mask, blender, mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint8_t reference[] = {
					0x0000, 0x0000, 0x0000,	0x0000, 0x0000,
					0x0011, 0x0011, 0x000E, 0x000E, 0x0000,
					0x0000, 0x00AB, 0x008D,	0x008D, 0x008D,
					0x0000, 0x00A0, 0x00A0,	0x00A0, 0x00A0,
				};

				assert_equal(reference, bitmap.data);
			}


			test( RowsAreProcessedInInterlacedOrderInDedicatedThreads )
			{
				// INIT
				const mocks::cell cells1[] = { { 0, 0, 0x10 }, { 3, 0, -0x10 }, };
				const mocks::cell cells2[] = { { 1, 0, 0x10 }, { 4, 0, -0x10 }, };
				const mocks::cell cells3[] = { { 2, 0, 0x10 }, { 5, 0, -0x10 }, };
				const mocks::cell cells4[] = { { 1, 0, 0x10 }, { 4, 0, -0x10 }, };
				const mocks::cell cells5[] = { { 0, 0, 0x10 }, { 3, 0, -0x10 }, };
				const mocks::cell cells6[] = { { 1, 0, 0x10 }, { 4, 0, -0x10 }, };
				mocks::mask<8>::scanline_cells cells[] = {
					make_pair(begin(cells1), end(cells1)),
					make_pair(begin(cells2), end(cells2)),
					make_pair(begin(cells3), end(cells3)),
					make_pair(begin(cells4), end(cells4)),
					make_pair(begin(cells5), end(cells5)),
					make_pair(begin(cells6), end(cells6)),
				};
				mocks::mask_full<8> mask(cells, 0);
				mocks::bitmap<uint16_t> bitmap(5, 6);
				thread_mapping mapping;
				mutex mtx;
				mocks::thread_mapping_blender<uint16_t, uint8_t> blender(mapping, mtx);

				// ACT
				renderer_parallel r1(2);
				r1(bitmap, 0, mask, blender, mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				assert_equal(get_scanline_thread<uint8_t>(bitmap, 0), get_scanline_thread<uint8_t>(bitmap, 2));
				assert_equal(get_scanline_thread<uint8_t>(bitmap, 0), get_scanline_thread<uint8_t>(bitmap, 4));

				assert_not_equal(get_scanline_thread<uint8_t>(bitmap, 0), get_scanline_thread<uint8_t>(bitmap, 1));
				assert_equal(get_scanline_thread<uint8_t>(bitmap, 1), get_scanline_thread<uint8_t>(bitmap, 3));
				assert_equal(get_scanline_thread<uint8_t>(bitmap, 1), get_scanline_thread<uint8_t>(bitmap, 5));

				// ACT
				renderer_parallel r2(3);
				r2(bitmap, 0, mask, blender, mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				assert_equal(get_scanline_thread<uint8_t>(bitmap, 0), get_scanline_thread<uint8_t>(bitmap, 3));

				assert_not_equal(get_scanline_thread<uint8_t>(bitmap, 0), get_scanline_thread<uint8_t>(bitmap, 1));
				assert_equal(get_scanline_thread<uint8_t>(bitmap, 1), get_scanline_thread<uint8_t>(bitmap, 4));

				assert_not_equal(get_scanline_thread<uint8_t>(bitmap, 0), get_scanline_thread<uint8_t>(bitmap, 2));
				assert_not_equal(get_scanline_thread<uint8_t>(bitmap, 1), get_scanline_thread<uint8_t>(bitmap, 2));
				assert_equal(get_scanline_thread<uint8_t>(bitmap, 2), get_scanline_thread<uint8_t>(bitmap, 5));

				// ACT
				renderer_parallel r3(5);
				r3(bitmap, 0, mask, blender, mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				assert_equal(get_scanline_thread<uint8_t>(bitmap, 0), get_scanline_thread<uint8_t>(bitmap, 5));
				assert_not_equal(get_scanline_thread<uint8_t>(bitmap, 0), get_scanline_thread<uint8_t>(bitmap, 1));
			}


			test( RendererPopulatesBitmapWithMaskDataAccordingToWindow )
			{
				// INIT
				const mocks::cell cells1[] = { { 0, 0, 0x11 }, { 3, 0, -0x03 }, { 7, 0, -0x0E }, };
				const mocks::cell cells2[] = { { 6, 0, 0xAB }, { 9, 0, -0x1E }, { 10, 0, -0x8D }, };
				const mocks::cell cells3[] = { { 1, 0, 0xA0 }, { 9, 0, -0xA0 }, };
				const mocks::mask<8>::scanline_cells cells[] = {
					make_pair(begin(cells1), end(cells1)),
					make_pair(begin(cells2), end(cells2)),
					make_pair(begin(cells3), end(cells3)),
				};
				const mocks::mask_full<8> mask1(cells, 3);
				mocks::bitmap<uint8_t> bitmap1(11, 4);
				const mocks::blender<uint8_t, uint8_t> blender1;
				const rect_i window = mkrect_sized(-1, 3, 11, 100);

				renderer_parallel r(2);

				// ACT
				r(bitmap1, &window, mask1, blender1, mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint8_t reference1[] = {
					0x00, 0x11, 0x11, 0x11, 0x0E, 0x0E, 0x0E, 0x0E, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xAB, 0xAB, 0x8D,
					0x00, 0x00, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				};

				assert_equal(reference1, bitmap1.data);
			}
		end_test_suite
	}
}
