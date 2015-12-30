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
				rect<real_t> box2 = { -10.3f, 5.2f, -3.1f, 17.7f };

				// ACT / ASSERT
				assert_equal(y1_clipped, clipping_y(-101, box1));
				assert_equal(0, clipping_y(-100, box1));
				assert_equal(0, clipping_y(1000, box1));
				assert_equal(y2_clipped, clipping_y(1001, box1));
				assert_equal(y1_clipped, clipping_y(static_cast<real_t>(5.199f), box2));
				assert_equal(y2_clipped, clipping_y(static_cast<real_t>(17.7001f), box2));
			}


			test( ClippingFlagsAreCalculatedAccordinglyToTheBox )
			{
				// INIT
				rect<int> box1 = { 10, -100, 100, 1000 };
				rect<real_t> box2 = { -10.3f, 5.2f, -3.1f, 17.7f };

				// ACT / ASSERT
				assert_equal(x1_clipped, clipping(9, 10, box1));
				assert_equal(0, clipping(10, 50, box1));
				assert_equal(0, clipping(100, 99, box1));
				assert_equal(x2_clipped, clipping(101, 99, box1));
				assert_equal(x1_clipped, clipping(static_cast<real_t>(-10.301f), static_cast<real_t>(6.0f), box2));
				assert_equal(x2_clipped, clipping(static_cast<real_t>(-3.099f), static_cast<real_t>(7.0f), box2));

				assert_equal(y1_clipped, clipping(15, -101, box1));
				assert_equal(y2_clipped, clipping(static_cast<real_t>(-5.0f), static_cast<real_t>(17.701f), box2));

				assert_equal(x1_clipped | y2_clipped, clipping(5, 1010, box1));
				assert_equal(x2_clipped | y1_clipped, clipping(static_cast<real_t>(-3.0f), static_cast<real_t>(5.0f), box2));
			}


			test( UnboundClipperDrawsLinesAsIs )
			{
				// INIT
				clipper<int> clipper_int;
				clipper<real_t> clipper_float;
				mocks::vector_rasterizer<int> ras_int;
				mocks::vector_rasterizer<real_t> ras_float;

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
				mocks::coords_pair<real_t> reference2[] = {
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
				clipper<real_t> c2;
				mocks::vector_rasterizer<int> r1;
				mocks::vector_rasterizer<real_t> r2;
				rect<int> b1 = { 120, 1002, 140, 1050 };
				rect<real_t> b2 = { -30.0f, -25.5f, -10.0f, 0.0f };

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
				mocks::coords_pair<real_t> reference2[] = {
					{ -20.0f, -25.5f, -10.9f, -1.0f },
					{ -10.9f, -1.0f, -28.0f, -23.3f },
					{ -28.0f, -23.3f, -20.0f, -25.5f },
				};

				assert_equal(reference1, r1.segments);
				assert_equal(reference2, r2.segments);
			}


			test( TrianglesExceedingBothXAreClippedToTrapezoids1 )
			{
				// INIT
				clipper<int> c;
				clipper<real_t> cf;
				mocks::vector_rasterizer<int> r;
				mocks::vector_rasterizer<real_t> rf;

				c.set(create_rect(10, -1000, 40, 1000));

				// ACT
				c.move_to(0, 0);
				c.line_to(r, 50, 0);
				c.line_to(r, 50, 5);
				c.line_to(r, 0, 0);

				// ASSERT
				mocks::coords_pair<int> reference1[] = {
					{ 10, 0, 40, 0 },
					{ 40, 0, 40, 4 },
					{ 40, 4, 10, 1 },
					{ 10, 1, 10, 0 },
				};

				assert_equal(reference1, r.segments);

				// INIT
				r.segments.clear();

				// ACT
				c.move_to(-20, 3);
				c.line_to(r, 52, 3);
				c.line_to(r, 52, 39);
				c.line_to(r, -20, 3);

				// ASSERT
				mocks::coords_pair<int> reference2[] = {
					{ 10, 3, 40, 3 },
					{ 40, 3, 40, 33 },
					{ 40, 33, 10, 18 },
					{ 10, 18, 10, 3 },
				};

				assert_equal(reference2, r.segments);

				// INIT
				cf.set(create_rect<real_t>(-2.5f, -1000.0f, -0.5f, 1000.0f));

				// ACT
				cf.move_to(-3.0f, 7.0f);
				cf.line_to(rf, 0.0f, 7.0f);
				cf.line_to(rf, 0.0f, 8.5f);
				cf.line_to(rf, -3.0f, 7.0f);

				// ASSERT
				mocks::coords_pair<real_t> reference3[] = {
					{ -2.5f, 7.0f, -0.5f, 7.0f },
					{ -0.5f, 7.0f, -0.5f, 8.25f },
					{ -0.5f, 8.25f, -2.5f, 7.25 },
					{ -2.5f, 7.25, -2.5f, 7.0f },
				};

				assert_equal(reference3, rf.segments);
			}



			test( TrianglesExceedingBothXAreClippedToTrapezoids2 )
			{
				// INIT
				clipper<int> c;
				clipper<real_t> cf;
				mocks::vector_rasterizer<int> r;
				mocks::vector_rasterizer<real_t> rf;

				c.set(create_rect(20, -1000, 70, 1000));

				// ACT
				c.move_to(0, 0);
				c.line_to(r, 0, 40);
				c.line_to(r, 80, 0);
				c.line_to(r, 0, 0);

				// ASSERT
				mocks::coords_pair<int> reference1[] = {
					{ 20, 0, 20, 30 },
					{ 20, 30, 70, 5 },
					{ 70, 5, 70, 0 },
					{ 70, 0, 20, 0 },
				};

				assert_equal(reference1, r.segments);

				// INIT
				r.segments.clear();

				// ACT
				c.move_to(-20, 3);
				c.line_to(r, -20, -27);
				c.line_to(r, 80, 3);
				c.line_to(r, -20, 3);

				// ASSERT
				mocks::coords_pair<int> reference2[] = {
					{ 20, 3, 20, -15 },
					{ 20, -15, 70, 0 },
					{ 70, 0, 70, 3 },
					{ 70, 3, 20, 3 },
				};

				assert_equal(reference2, r.segments);

				// INIT
				cf.set(create_rect<real_t>(-2.5f, -1000.0f, -0.5f, 1000.0f));

				// ACT
				cf.move_to(-13.0f, 5.0f);
				cf.line_to(rf, -13.0f, 15.0f);
				cf.line_to(rf, 7.0f, 5.0f);
				cf.line_to(rf, -13.0f, 5.0f);

				// ASSERT
				mocks::coords_pair<real_t> reference3[] = {
					{ -2.5f, 5.0f, -2.5f, 9.75f },
					{ -2.5f, 9.75f, -0.5f, 8.75f },
					{ -0.5f, 8.75f, -0.5f, 5.0f },
					{ -0.5f, 5.0f, -2.5f, 5.0f },
				};

				assert_equal(reference3, rf.segments);
			}


			test( TrianglesExceedingRXAreClippedToTriangles )
			{
				// INIT
				clipper<int> c;
				clipper<real_t> cf;
				mocks::vector_rasterizer<int> r;
				mocks::vector_rasterizer<real_t> rf;

				c.set(create_rect(-1000, -1000, 27, 1000));

				// ACT
				c.move_to(-3, 5);
				c.line_to(r, 47, 40);
				c.line_to(r, 64, -27);
				c.line_to(r, -3, 5);

				// ASSERT
				mocks::coords_pair<int> reference1[] = {
					{ -3, 5, 27, 26 },
					{ 27, 26, 27, -10 },
					{ 27, -10, -3, 5 },
				};

				assert_equal(reference1, r.segments);

				// INIT
				r.segments.clear();

				// ACT
				c.move_to(12, 7);
				c.line_to(r, 112, 7);
				c.line_to(r, 112, 47);
				c.line_to(r, 12, 7);

				// ASSERT
				mocks::coords_pair<int> reference2[] = {
					{ 12, 7, 27, 7 },
					{ 27, 7, 27, 13, },
					{ 27, 13, 12, 7 },
				};

				assert_equal(reference2, r.segments);

				// INIT
				cf.set(create_rect<real_t>(-1000.0f, -1000.0f, 2.7f, 1000.0f));

				// ACT
				cf.move_to(1.2f, 0.7f);
				cf.line_to(rf, 11.2f, 0.7f);
				cf.line_to(rf, 11.2f, 4.7f);
				cf.line_to(rf, 1.2f, 0.7f);

				// ASSERT
				mocks::coords_pair<real_t> reference3[] = {
					{ 1.2f, 0.7f, 2.7f, 0.7f },
					{ 2.7f, 0.7f, 2.7f, 1.3f, },
					{ 2.7f, 1.3f, 1.2f, 0.7f },
				};

				assert_equal(reference3, rf.segments);
			}


			test( TrianglesExceedingLXAreClippedToQuadrilaterals )
			{
				// INIT
				clipper<int> c;
				clipper<real_t> cf;
				mocks::vector_rasterizer<int> r;
				mocks::vector_rasterizer<real_t> rf;

				c.set(create_rect(10, -1000, 1000, 1000));

				// ACT
				c.move_to(-20, 13);
				c.line_to(r, 25, 103);
				c.line_to(r, 52, 25);
				c.line_to(r, -20, 13);

				// ASSERT
				mocks::coords_pair<int> reference1[] = {
					{ 10, 13, 10, 73 },
					{ 10, 73, 25, 103 },
					{ 25, 103, 52, 25 },
					{ 52, 25, 10, 18 },
					{ 10, 18, 10, 13 },
				};

				assert_equal(reference1, r.segments);

				// INIT
				cf.set(create_rect<real_t>(100.1f, -10000.0f, 10000.0f, 10000.0f));

				// ACT
				cf.move_to(-199.9f, 130.3f);
				cf.line_to(rf, 520.1f, 250.3f);
				cf.line_to(rf, 250.1f, 1030.3f);
				cf.line_to(rf, -199.9f, 130.3f);

				// ASSERT
				mocks::coords_pair<real_t> reference2[] = {
					{ 100.1f, 130.3f, 100.1f, 180.3f },
					{ 100.1f, 180.3f, 520.1f, 250.3f },
					{ 520.1f, 250.3f, 250.1f, 1030.3f },
					{ 250.1f, 1030.3f, 100.1f, 730.3f },
					{ 100.1f, 730.3f, 100.1f, 130.3f },
				};

				assert_equal(reference2, rf.segments);
			}


			test( TrianglesExceedingBothYAreClippedToBoundingLines )
			{
				// INIT
				clipper<int> c;
				clipper<real_t> cf;
				mocks::vector_rasterizer<int> r;
				mocks::vector_rasterizer<real_t> rf;

				c.set(create_rect(-1000, 10, 1000, 40));

				// ACT
				c.move_to(0, 0);
				c.line_to(r, 0, 50);
				c.line_to(r, 5, 50);
				c.line_to(r, 0, 0);

				// ASSERT
				mocks::coords_pair<int> reference1[] = {
					{ 0, 10, 0, 40 },
					{ 4, 40, 1, 10 },
				};

				assert_equal(reference1, r.segments);

				// INIT
				r.segments.clear();

				// ACT
				c.move_to(3, -20);
				c.line_to(r, 3, 52);
				c.line_to(r, 39, 52);
				c.line_to(r, 3, -20);

				// ASSERT
				mocks::coords_pair<int> reference2[] = {
					{ 3, 10, 3, 40 },
					{ 33, 40, 18, 10 },
				};

				assert_equal(reference2, r.segments);

				// INIT
				cf.set(create_rect<real_t>(-1000.0f, -2.5f, 1000.0f, -0.5f));

				// ACT
				cf.move_to(7.0f, -3.0f);
				cf.line_to(rf, 7.0f, 0.0f);
				cf.line_to(rf, 8.5f, 0.0f);
				cf.line_to(rf, 7.0f, -3.0f);

				// ASSERT
				mocks::coords_pair<real_t> reference3[] = {
					{ 7.0f, -2.5f, 7.0f, -0.5f },
					{ 8.25f,-0.5f, 7.25f, -2.5f },
				};

				assert_equal(reference3, rf.segments);

				// INIT
				rf.segments.clear();

				// ACT
				cf.move_to(7.0f, -3.0f);
				cf.line_to(rf, 8.5f, 0.0f);
				cf.line_to(rf, 7.0f, 0.0f);
				cf.line_to(rf, 7.0f, -3.0f);

				// ASSERT
				mocks::coords_pair<real_t> reference4[] = {
					{ 7.25f, -2.5f, 8.25f, -0.5f },
					{ 7.0f, -0.5f, 7.0f, -2.5f },
				};

				assert_equal(reference4, rf.segments);
			}


			test( LastPointClippingIsAdjustedOnWindowSetting )
			{
				// INIT
				clipper<float> c;
				mocks::vector_rasterizer<float> r;

				c.move_to(10.0f, 100.0f);

				// ACT
				c.set(create_rect<real_t>(-1000.0f, 10.0f, 1000.0f, 40.0f));
				c.line_to(r, 20.0f, 20.0f);

				// ASSERT
				mocks::coords_pair<real_t> reference1[] = {
					{ 17.5f, 40.0f, 20.0f, 20.0f },
				};

				assert_equal(reference1, r.segments);

				// ACT
				c.set(create_rect<real_t>(-1000.0f, 30.0f, 1000.0f, 1000.0f));
				c.line_to(r, 10.0f, 100.0f);

				// ASSERT
				mocks::coords_pair<real_t> reference2[] = {
					{ 17.5f, 40.0f, 20.0f, 20.0f },
					{ 18.75f, 30.0f, 10.0f, 100.0f },
				};

				assert_equal(reference2, r.segments);
			}
		end_test_suite
	}
}
