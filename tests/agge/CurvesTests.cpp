#include <agge/curves.h>

#include "assertex.h"
#include "mocks.h"

#include <agge/path.h>

#include <ut/test.h>

namespace agge
{
	namespace tests
	{
		begin_test_suite( QBezierCurveTests )
			test( QBezier2SegmentProducesIterator )
			{
				// INIT
				real_t x = 11.0f, y = 13.0f;
				qbezier b(0.0f, 0.0f, 1.0f, 1.0f, 2.0f, 0.0f, 1.0f);

				// ACT / ASSERT
				assert_equal(path_command_move_to, b.vertex(&x, &y));

				// ACT / ASSERT
				assert_equal(0.0f, x);
				assert_equal(0.0f, y);
			}


			test( CoarseQBezier2ProducesLine )
			{
				// INIT
				qbezier b1(0.0f, 0.0f, 1.0f, 1.0f, 2.0f, 0.0f, 100.0f /* ridiculously big step */);

				// ACT
				mocks::path::point points1[] = { vertex(b1), vertex(b1), vertex(b1), };

				// ASSERT
				mocks::path::point reference1[] = { moveto(0.0f, 0.0f), lineto(2.0f, 0.0f), stop(), };

				assert_equal(reference1, points1);

				// INIT
				qbezier b2(10.0f, 11.0f, 1.0f, 1.0f, 13.0f, 17.0f, 100.0f /* ridiculously big step */);

				// ACT
				mocks::path::point points2[] = { vertex(b2), vertex(b2), vertex(b2), };

				// ASSERT
				mocks::path::point reference2[] = { moveto(10.0f, 11.0f), lineto(13.0f, 17.0f), stop(), };

				assert_equal(reference2, points2);
			}


			test( QBezierIteratorProducesNumberOfVerticesAccordinglyToStep )
			{
				// INIT
				real_t x, y;
				qbezier b1(0.0f, 0.0f, 0.5f, 0.6f, 1.0f, 2.0f, 0.5f);

				// ACT / ASSERT
				assert_equal(path_command_move_to, b1.vertex(&x, &y));
				assert_equal(path_command_line_to, b1.vertex(&x, &y));
				assert_equal(path_command_line_to, b1.vertex(&x, &y));
				assert_equal(path_command_stop, b1.vertex(&x, &y));

				// INIT
				qbezier b2(0.0f, 0.0f, 0.5f, 0.6f, 1.0f, 2.0f, 0.2f);

				// ACT / ASSERT
				assert_equal(path_command_move_to, b2.vertex(&x, &y));
				assert_equal(path_command_line_to, b2.vertex(&x, &y));
				assert_equal(path_command_line_to, b2.vertex(&x, &y));
				assert_equal(path_command_line_to, b2.vertex(&x, &y));
				assert_equal(path_command_line_to, b2.vertex(&x, &y));
				assert_equal(path_command_line_to, b2.vertex(&x, &y));
				assert_equal(path_command_stop, b2.vertex(&x, &y));
			}


			test( QBezierIteratorProducesExpectedVerticesAccordinglyToParameters )
			{
				// INIT
				qbezier b1(-3.5f, 7.4f, 1.0f, 2.0f, 10.1f, 3.8f, 0.111f);

				// ACT
				mocks::path::point points1[] = {
					vertex(b1), vertex(b1), vertex(b1), vertex(b1), vertex(b1),
					vertex(b1), vertex(b1), vertex(b1), vertex(b1), vertex(b1),
					vertex(b1), vertex(b1),
				};

				// ASSERT
				mocks::path::point reference1[] = {
					moveto(-3.50000000f, 7.40000010f),
					lineto(-2.44432354f, 6.28991079f),
					lineto(-1.27529359f, 5.35724497f),
					lineto(0.00708937645f, 4.60200071f),
					lineto(1.40282583f, 4.02417898f),
					lineto(2.91191530f, 3.62378001f),
					lineto(4.53435755f, 3.40080309f),
					lineto(6.27015400f, 3.35524869f),
					lineto(8.11930275f, 3.48711658f),
					lineto(10.0818052f, 3.79640722f),
					lineto(10.1000004f, 3.79999995f),
					stop(),
				};

				assert_equal(reference1, points1);

				// INIT
				qbezier b2(1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.3f);

				// ACT
				mocks::path::point points2[] = {
					vertex(b2), vertex(b2), vertex(b2), vertex(b2), vertex(b2), vertex(b2),
				};

				// ASSERT
				mocks::path::point reference2[] = {
					moveto(1.00000000f, 1.00000000f),
					lineto(0.399999976f, 0.579999983f),
					lineto(-0.200000033f, 0.519999981f),
					lineto(-0.800000072f, 0.820000052f),
					lineto(-1.00000000f, 1.00000000f),
					stop(),
				};

				assert_equal(reference2, points2);
			}


			test( RewindingQuadraticBezierIteratorStartsOverWithPoints )
			{
				// INIT
				qbezier b(1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.3f);

				vertex(b), vertex(b);

				// ACT
				b.rewind(0);

				// ASSERT
				mocks::path::point points[] = { vertex(b), vertex(b), };
				mocks::path::point reference[] = { moveto(1.00000000f, 1.00000000f), lineto(0.399999976f, 0.579999983f), };

				assert_equal(reference, points);
			}

		end_test_suite


		begin_test_suite( CBezierCurveTests )
			test( CBezierIteratorProducesNumberOfVerticesAccordinglyToStep )
			{
				// INIT
				real_t x, y;
				cbezier c1(0.0f, 0.0f, 0.5f, 0.6f, 0.0f, 0.0f, 1.0f, 2.0f, 0.5f);

				// ACT / ASSERT
				assert_equal(path_command_move_to, c1.vertex(&x, &y));
				assert_equal(path_command_line_to, c1.vertex(&x, &y));
				assert_equal(path_command_line_to, c1.vertex(&x, &y));
				assert_equal(path_command_stop, c1.vertex(&x, &y));

				// INIT
				cbezier c2(0.0f, 0.0f, 0.5f, 0.6f, 0.0f, 0.0f, 1.0f, 2.0f, 0.2f);

				// ACT / ASSERT
				assert_equal(path_command_move_to, c2.vertex(&x, &y));
				assert_equal(path_command_line_to, c2.vertex(&x, &y));
				assert_equal(path_command_line_to, c2.vertex(&x, &y));
				assert_equal(path_command_line_to, c2.vertex(&x, &y));
				assert_equal(path_command_line_to, c2.vertex(&x, &y));
				assert_equal(path_command_line_to, c2.vertex(&x, &y));
				assert_equal(path_command_stop, c2.vertex(&x, &y));
			}


			test( CBezierIteratorProducesExpectedVerticesAccordinglyToParameters )
			{
				// INIT
				cbezier c1(-3.5f, 7.4f, 1.0f, 2.0f, 0.2f, 0.3f, 10.1f, 3.8f, 0.111f);

				// ACT
				mocks::path::point points1[] = {
					vertex(c1), vertex(c1), vertex(c1), vertex(c1), vertex(c1),
					vertex(c1), vertex(c1), vertex(c1), vertex(c1), vertex(c1),
					vertex(c1), vertex(c1),
				};

				// ASSERT
				mocks::path::point reference1[] = {
					moveto(-3.50000000f, 7.40000010f),
					lineto(-2.17552185f, 5.74061489f),
					lineto(-1.11155891f, 4.36706400f),
					lineto(-0.176818520f, 3.2916567f),
					lineto(0.759991825f, 2.52670193f),
					lineto(1.83016467f, 2.08450842f),
					lineto(3.16499233f, 1.97738409f),
					lineto(4.89576817f, 2.21763802f),
					lineto(7.15378380f, 2.81757903f),
					lineto(10.0703335f, 3.78951573f),
					lineto(10.1000004f, 3.79999995f),
					stop(),
				};

				assert_equal(reference1, points1);

				// INIT
				cbezier c2(1.0f, 1.0f, 0.0f, 0.0f, 0.8f, 0.7f, -1.0f, 1.0f, 0.3f);

				// ACT
				mocks::path::point points2[] = { vertex(c2), vertex(c2), vertex(c2), vertex(c2), vertex(c2), vertex(c2), };

				// ASSERT
				mocks::path::point reference2[] = {
					moveto(1.00000000f, 1.00000000f),
					lineto(0.467199981f, 0.502300024f),
					lineto(0.193599969f, 0.582399964f),
					lineto(-0.533600152f, 0.900100052f),
					lineto(-1.00000000f, 1.00000000f),
					stop(),
				};

				assert_equal(reference2, points2);
			}


			test( RewindingCubicBezierIteratorStartsOverWithPoints )
			{
				// INIT
				cbezier c(1.0f, 1.0f, 0.0f, 0.0f, 0.8f, 0.7f, -1.0f, 1.0f, 0.3f);

				vertex(c), vertex(c);

				// ACT
				c.rewind(0);

				// ASSERT
				mocks::path::point points[] = { vertex(c), vertex(c), };
				mocks::path::point reference[] = { moveto(1.00000000f, 1.00000000f), lineto(0.467199981f, 0.502300024f), };

				assert_equal(reference, points);
			}

		end_test_suite
	}
}
