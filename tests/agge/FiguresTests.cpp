#include <agge/figures.h>

#include "assertex.h"
#include "mocks.h"

#include <agge/path.h>

#include <ut/test.h>

namespace agge
{
	namespace tests
	{
		begin_test_suite( LineTests )
			test( LineProducesIterator )
			{
				// INIT
				real_t x, y;

				// INIT / ACT
				line l(1.0f, 13.0f, 20.0f, 17.0f);

				// ACT / ASSERT
				assert_equal(path_command_move_to, l.vertex(&x, &y));
				assert_equal(path_command_line_to, l.vertex(&x, &y));
				assert_equal(path_command_end_poly, l.vertex(&x, &y));
				assert_equal(path_command_stop, l.vertex(&x, &y));
				assert_equal(path_command_stop, l.vertex(&x, &y));
			}


			test( LineIteratorListsItsVertices )
			{
				// INIT
				line l1(10.1f, 20.2f, 30.0f, 4.7f);

				// ACT
				mocks::path::point points1[] = { vertex(l1), vertex(l1), vertex(l1), vertex(l1), };

				// ASSERT
				mocks::path::point reference1[] = {
					moveto(10.1f, 20.2f), lineto(30.0f, 4.7f), { 0.0f, 0.0f, path_command_end_poly }, stop(),
				};

				assert_equal(reference1, points1);

				// INIT
				line l2(11.1f, 12.2f, 13.3f, 1.7f);

				// ACT
				mocks::path::point points2[] = { vertex(l2), vertex(l2), vertex(l2), vertex(l2), };

				// ASSERT
				mocks::path::point reference2[] = {
					moveto(11.1f, 12.2f), lineto(13.3f, 1.7f), { 0.0f, 0.0f, path_command_end_poly }, stop(),
				};

				assert_equal(reference2, points2);
			}


			test( RewindingRectangleIteratorStartsOverWithPoints )
			{
				// INIT
				real_t x, y;
				line l(10.1f, 20.2f, 30.0f, 4.7f);

				l.vertex(&x, &y);

				// ACT
				l.rewind(0);

				// ASSERT
				assert_equal(path_command_move_to, l.vertex(&x, &y));
				assert_equal(path_command_line_to, l.vertex(&x, &y));
				assert_equal(path_command_end_poly, l.vertex(&x, &y));
				assert_equal(path_command_stop, l.vertex(&x, &y));
			}
		end_test_suite


		begin_test_suite( RectangleTests )
			test( RectangleProducesIterator )
			{
				// INIT
				real_t x, y;

				// INIT / ACT
				rectangle r(10.0f, 20.0f, 30.0f, 40.0f);

				// ACT / ASSERT
				assert_equal(path_command_move_to, r.vertex(&x, &y));
				assert_equal(path_command_line_to, r.vertex(&x, &y));
				assert_equal(path_command_line_to, r.vertex(&x, &y));
				assert_equal(path_command_line_to, r.vertex(&x, &y));
				assert_equal(path_command_end_poly | path_flag_close, r.vertex(&x, &y));
				assert_equal(path_command_stop, r.vertex(&x, &y));
				assert_equal(path_command_stop, r.vertex(&x, &y));
			}


			test( RectangleIteratorListsItsVertices )
			{
				// INIT
				rectangle r1(10.1f, 20.2f, 30.0f, 4.7f);

				// ACT
				mocks::path::point points1[] = { vertex(r1), vertex(r1), vertex(r1), vertex(r1), vertex(r1), vertex(r1), };

				// ASSERT
				mocks::path::point reference1[] = {
					moveto(10.1f, 20.2f), lineto(30.0f, 20.2f), lineto(30.0f, 4.7f), lineto(10.1f, 4.7f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close }, stop(),
				};

				assert_equal(reference1, points1);

				// INIT
				rectangle r2(11.1f, 12.2f, 13.3f, 1.7f);

				// ACT
				mocks::path::point points2[] = { vertex(r2), vertex(r2), vertex(r2), vertex(r2), vertex(r2), vertex(r2), };

				// ASSERT
				mocks::path::point reference2[] = {
					moveto(11.1f, 12.2f), lineto(13.3f, 12.2f), lineto(13.3f, 1.7f), lineto(11.1f, 1.7f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close }, stop(),
				};

				assert_equal(reference2, points2);
			}


			test( RewindingRectangleIteratorStartsOverWithPoints )
			{
				// INIT
				rectangle r(10.1f, 20.2f, 30.0f, 4.7f);

				vertex(r), vertex(r);

				// ACT
				r.rewind(0);

				// ASSERT
				mocks::path::point points[] = { vertex(r), vertex(r), vertex(r), vertex(r), vertex(r), vertex(r), };
				mocks::path::point reference[] = {
					moveto(10.1f, 20.2f), lineto(30.0f, 20.2f), lineto(30.0f, 4.7f), lineto(10.1f, 4.7f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close }, stop(),
				};

				assert_equal(reference, points);
			}
		end_test_suite
	}
}
