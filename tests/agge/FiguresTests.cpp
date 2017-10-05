#include <agge/figures.h>

#include "assertex.h"
#include "mocks.h"

#include <agge/path.h>

#include <ut/test.h>

namespace agge
{
	namespace tests
	{
		begin_test_suite( FiguresTests )
			test( RectangleProducesIterator )
			{
				// INIT
				const rectangle r(10, 20, 30, 40);

				// ACT
				rectangle::iterator i = r.iterate();

				// ACT / ASSERT
				real_t x, y;

				assert_equal(path_command_move_to, i.vertex(&x, &y));
				assert_equal(path_command_line_to, i.vertex(&x, &y));
				assert_equal(path_command_line_to, i.vertex(&x, &y));
				assert_equal(path_command_line_to, i.vertex(&x, &y));
				assert_equal(path_command_end_poly | path_flag_close, i.vertex(&x, &y));
				assert_equal(path_command_stop, i.vertex(&x, &y));
				assert_equal(path_command_stop, i.vertex(&x, &y));
			}


			test( RectangleIteratorListsItsVertices )
			{
				// INIT
				rectangle r1(10.1f, 20.2f, 30.0f, 4.7f);
				rectangle::iterator i1 = r1.iterate();

				// ACT
				mocks::path::point points1[] = { vertex(i1), vertex(i1), vertex(i1), vertex(i1), vertex(i1), vertex(i1), };

				// ASSERT
				mocks::path::point reference1[] = {
					moveto(10.1f, 20.2f), lineto(30.0f, 20.2f), lineto(30.0f, 4.7f), lineto(10.1f, 4.7f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close }, { 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, points1);

				// INIT
				rectangle r2(11.1f, 12.2f, 13.3f, 1.7f);
				rectangle::iterator i2 = r2.iterate();

				// ACT
				mocks::path::point points2[] = { vertex(i2), vertex(i2), vertex(i2), vertex(i2), vertex(i2), vertex(i2), };

				// ASSERT
				mocks::path::point reference2[] = {
					moveto(11.1f, 12.2f), lineto(13.3f, 12.2f), lineto(13.3f, 1.7f), lineto(11.1f, 1.7f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close }, { 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, points2);
			}
		end_test_suite
	}
}
