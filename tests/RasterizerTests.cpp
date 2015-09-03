#include <agge/rasterizer.h>

#include <agge/renderer.h>

#include "helpers.h"
#include "mocks.h"

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

namespace agge
{
	namespace tests
	{
		namespace
		{
			class passthrough_clipper
			{
			public:
				void move_to(real_t x, real_t y)
				{	_last_x = x, _last_y = y;	}

				template <typename LinesSinkT>
				void line_to(LinesSinkT &sink, real_t x, real_t y)
				{
					sink.line(_last_x, _last_y, x, y);
					move_to(x, y);
				}

			private:
				real_t _last_x, _last_y;
			};
		}

		begin_test_suite( RasterizerTests )
			test( DrawingAFigureWillRenderItToABitmap )
			{
				// INIT
				rasterizer<passthrough_clipper> ras;
				mocks::bitmap<uint8_t, 1, 1> bitmap1(6, 5);
				mocks::bitmap<uint16_t, 1> bitmap2(7, 6);
				renderer r;

				// ACT
				ras.move_to(3.1f, 0.3f);
				ras.line_to(-2.422f, 2.705f);
				ras.line_to(6.43f, 6.72f);
				ras.line_to(3.1f, 0.3f);
				ras.sort();
				r(bitmap1, 0, ras, mocks::blender<uint8_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint8_t reference1[] = {
					0x00, 0x0e, 0x70, 0x31, 0x00, 0x00, 0x00,
					0x92, 0xf2, 0xff, 0xb8, 0x00, 0x00, 0x00,
					0xff, 0xff, 0xff, 0xff, 0x3d, 0x00, 0x00,
					0xf4, 0xff, 0xff, 0xff, 0xc2, 0x00, 0x00,
					0x12, 0x7b, 0xe7, 0xff, 0xff, 0x47, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				};

				assert_equal(reference1, bitmap1.data);

				// ACT
				ras.reset();
				ras.move_to(1.0f, 1.0f);
				ras.line_to(1.0f, 5.0f);
				ras.line_to(4.5f, 5.0f);
				ras.line_to(4.5f, 1.0f);
				ras.line_to(1.0f, 1.0f);
				ras.sort();
				r(bitmap2, 0, ras, mocks::blender<uint16_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint16_t reference2[] = {
					0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
					0x0000, 0x01FF, 0x01FF, 0x01FF, 0x0180, 0x0000, 0x0000, 0x0000,
					0x0000, 0x01FF, 0x01FF, 0x01FF, 0x0180, 0x0000, 0x0000, 0x0000,
					0x0000, 0x01FF, 0x01FF, 0x01FF, 0x0180, 0x0000, 0x0000, 0x0000,
					0x0000, 0x01FF, 0x01FF, 0x01FF, 0x0180, 0x0000, 0x0000, 0x0000,
					0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				};

				assert_equal(reference2, bitmap2.data);
			}


			test( ClosingAPolygonDrawsLineToTheLastMoveToPoint )
			{
				// INIT
				rasterizer<passthrough_clipper> ras;
				mocks::bitmap<uint8_t> bitmap(5, 5);
				renderer r;

				// ACT
				ras.move_to(0.9f, 0.1f);
				ras.line_to(1.2f, 4.7f);
				ras.line_to(4.3f, 3.1f);
				ras.close_polygon();
				ras.sort();
				r(bitmap, 0, ras, mocks::blender<uint8_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint8_t reference1[] = {
					0x0f, 0x5e, 0x00, 0x00, 0x00,
					0x04, 0xfe, 0x7d, 0x00, 0x00,
					0x00, 0xf2, 0xff, 0x9a, 0x03,
					0x00, 0xe1, 0xf2, 0x83, 0x0c,
					0x00, 0x69, 0x14, 0x00, 0x00,
				};

				assert_equal(reference1, bitmap.data);

				// INIT
				bitmap.data.assign(bitmap.data.size(), 0);
				ras.reset();

				// ACT
				ras.move_to(1.2f, 2.1f);
				ras.line_to(1.2f, 4.7f);
				ras.line_to(4.3f, 3.1f);
				ras.close_polygon();
				ras.sort();
				r(bitmap, 0, ras, mocks::blender<uint8_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint8_t reference2[] = {
					0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x9d, 0x7b, 0x28, 0x00,
					0x00, 0xcd, 0xf2, 0x82, 0x09,
					0x00, 0x64, 0x14, 0x00, 0x00,
				};

				assert_equal(reference2, bitmap.data);
			}
		end_test_suite
	}
}
