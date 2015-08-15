#include <agge/clipper.h>

#include "helpers.h"
#include "mocks.h"

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

namespace agge
{
	namespace tests
	{
		begin_test_suite( ClipperTests )
			test( YClippingFlagsAreCalculatedAccordinglyToTheBox )
			{
				// INIT
				rect<int> box1 = { 10, -100, 100, 1000 };
				rect<float> box2 = { -10.3f, 5.2f, -3.1f, 17.7f };

				// ACT / ASSERT
				assert_equal(y1_clipped, clipping_y(-101, box1));
				assert_equal(0, clipping_y(-100, box1));
				assert_equal(0, clipping_y(1000, box1));
				assert_equal(y2_clipped, clipping_y(1001, box1));
				assert_equal(y1_clipped, clipping_y(5.199f, box2));
				assert_equal(y2_clipped, clipping_y(17.7001f, box2));
			}


			test( ClippingFlagsAreCalculatedAccordinglyToTheBox )
			{
				// INIT
				rect<int> box1 = { 10, -100, 100, 1000 };
				rect<float> box2 = { -10.3f, 5.2f, -3.1f, 17.7f };

				// ACT / ASSERT
				assert_equal(x1_clipped, clipping(9, 10, box1));
				assert_equal(0, clipping(10, 50, box1));
				assert_equal(0, clipping(100, 99, box1));
				assert_equal(x2_clipped, clipping(101, 99, box1));
				assert_equal(x1_clipped, clipping(-10.301f, 6.0f, box2));
				assert_equal(x2_clipped, clipping(-3.099f, 7.0f, box2));

				assert_equal(y1_clipped, clipping(15, -101, box1));
				assert_equal(y2_clipped, clipping(-5.0f, 17.701f, box2));

				assert_equal(x1_clipped | y2_clipped, clipping(5, 1010, box1));
				assert_equal(x2_clipped | y1_clipped, clipping(-3.0f, 5.0f, box2));
			}


			test( UnboundClipperDrawsLinesAsIs )
			{
				// INIT
				clipper<int> clipper_int;
				clipper<float> clipper_float;
				mocks::vector_rasterizer<int> ras_int;
				mocks::vector_rasterizer<float> ras_float;

				// ACT
				clipper_int.move_to(-0x7FFFFFFF, -0x7FFFFFFF);
				clipper_int.line_to(ras_int, 0x7FFFFFFF, 0x6FFFFFFF);
				clipper_int.line_to(ras_int, 0x6FFFFFFF, 0x7FFFFFFF);
				clipper_int.line_to(ras_int, -0x7FFFFFFF, -0x7FFFFFFF);

				clipper_float.move_to(-1e-30f, -1e-30f);
				clipper_float.line_to(ras_float, 1e-30f, 0.9e-30f);
				clipper_float.line_to(ras_float, 0.9e-30f, 1e-30f);
				clipper_float.line_to(ras_float, -1e-30f, -0.9e-30f);
				clipper_float.line_to(ras_float, -1e-30f, -1e-30f);

				// ASSERT
				mocks::coords_pair<int> reference1[] = {
					{ -0x7FFFFFFF, -0x7FFFFFFF, +0x7FFFFFFF, +0x6FFFFFFF },
					{ +0x7FFFFFFF, +0x6FFFFFFF, +0x6FFFFFFF, +0x7FFFFFFF },
					{ +0x6FFFFFFF, +0x7FFFFFFF, -0x7FFFFFFF, -0x7FFFFFFF },
				};
				mocks::coords_pair<float> reference2[] = {
					{ -1e-30f, -1e-30f, +1e-30f, +0.9e-30f },
					{ +1e-30f, +0.9e-30f, +0.9e-30f, +1e-30f },
					{ +0.9e-30f, +1e-30f, -1e-30f, -0.9e-30f },
					{ -1e-30f, -0.9e-30f, -1e-30f, -1e-30f },
				};

				assert_equal(reference1, ras_int.segments);
				assert_equal(reference2, ras_float.segments);
			}


			test( DrawingWithinBoundsDrawsLinesAsIs )
			{
				// INIT
				clipper<int> c1;
				clipper<float> c2;
				mocks::vector_rasterizer<int> r1;
				mocks::vector_rasterizer<float> r2;
				rect<int> b1 = { 120, 1002, 140, 1050 };
				rect<float> b2 = { -30.0f, -25.5f, -10.0f, 0.0f };

				c1.set(b1);
				c2.set(b2);

				// ACT
				c1.move_to(120, 1003);
				c1.line_to(r1, 139, 1003);
				c1.line_to(r1, 139, 1050);
				c1.line_to(r1, 120, 1050);
				c1.line_to(r1, 120, 1003);

				c2.move_to(-20.0f, -25.5f);
				c2.line_to(r2, -10.9f, -1.0f);
				c2.line_to(r2, -28.0f, -23.3f);
				c2.line_to(r2, -20.0f, -25.5f);

				// ASSERT
				mocks::coords_pair<int> reference1[] = {
					{ 120, 1003, 139, 1003 },
					{ 139, 1003, 139, 1050 },
					{ 139, 1050, 120, 1050 },
					{ 120, 1050, 120, 1003 },
				};
				mocks::coords_pair<float> reference2[] = {
					{ -20.0f, -25.5f, -10.9f, -1.0f },
					{ -10.9f, -1.0f, -28.0f, -23.3f },
					{ -28.0f, -23.3f, -20.0f, -25.5f },
				};

				assert_equal(reference1, r1.segments);
				assert_equal(reference2, r2.segments);
			}


			test( TrianglesExceedingXAreClippedToTrapezoids )
			{
				// INIT
				clipper<int> c1;
				mocks::vector_rasterizer<int> r1;
				rect<int> b1 = { 10, -1000, 40, 1000 };

				c1.set(b1);

				// ACT
				c1.move_to(0, 0);
				c1.line_to(r1, 50, 0);
				c1.line_to(r1, 50, 5);
				c1.line_to(r1, 0, 0);

				// ASSERT
				mocks::coords_pair<int> reference1[] = {
					{ 10, 0, 40, 0 },
					{ 40, 0, 40, 4 },
					{ 40, 4, 10, 1 },
					{ 10, 1, 10, 0 },
				};

				assert_equal(reference1, r1.segments);
			}

		end_test_suite
	}
}
