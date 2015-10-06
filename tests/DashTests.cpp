#include <agge/dash.h>

#include "mocks.h"

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

namespace agge
{
	namespace tests
	{
		namespace
		{
			template <typename SourceT>
			mocks::path::point vertex(SourceT &source)
			{
				mocks::path::point p = { 0 };

				p.command = source.vertex(&p.x, &p.y);
				return p;
			}
		}

		begin_test_suite( DashTests )
			test( SegmentAreOutputtedAsIsIfDashIsLong )
			{
				// INIT
				dash d;

				// INIT / ACT
				d.add_dash(6.0f, 1.0f);

				// ACT
				d.move_to(1.1f, 17.0f);
				d.line_to(4.1f, 13.0f);
				mocks::path::point result1[] = { vertex(d), vertex(d), vertex(d), };

				// ASSERT
				mocks::path::point reference1[] = {
					{ 1.1f, 17.0f, path_command_move_to },
					{ 4.1f, 13.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, result1);

				// ACT
				d.remove_all();
				d.move_to(1.0f, 1.0f);
				d.line_to(2.5f, 3.0f);
				d.line_to(2.7f, 3.1f);
				mocks::path::point result2[] = { vertex(d), vertex(d), vertex(d), vertex(d), };

				// ASSERT
				mocks::path::point reference2[] = {
					{ 1.0f, 1.0f, path_command_move_to },
					{ 2.5f, 3.0f, path_command_line_to },
					{ 2.7f, 3.1f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, result2);
			}


			ignore( SegmentIsLimitedToADash )
			{
				// INIT
				mocks::path::point input1[] = {
					{ 1.3f, 17.0f, path_command_move_to },
					{ 5.8f, 11.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};
				mocks::path::point input2[] = {
					{ 1.0f, -1.0f, path_command_move_to },
					{ 1.0f, 10.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};
				mocks::path::point input3[] = {
					{ 1.9f, 2.7f, path_command_move_to },
					{ 3.9f, 4.7f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};
				mocks::path p1(input1);
				mocks::path p2(input2);
				mocks::path p3(input3);
				dash d;

				// INIT / ACT
				d.add_dash(5.0f, 10.0f);

				// ACT
				d.move_to(1.3f, 17.0f);
				d.line_to(5.8f, 11.0f);
				mocks::path::point result1[] = { vertex(d), vertex(d), vertex(d), };

				// ASSERT
				mocks::path::point reference1[] = {
					{ 1.3f, 17.0f, path_command_move_to },
					{ 4.3f, 13.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, result1);

				// ACT
				d.remove_all();
				d.move_to(1.0f, -1.0f);
				d.line_to(1.0f, 10.0f);
				mocks::path::point result2[] = { vertex(d), vertex(d), vertex(d), };

				// ASSERT
				mocks::path::point reference2[] = {
					{ 1.0f, -1.0f, path_command_move_to },
					{ 1.0f, 4.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, result2);

				// INIT / ACT
				d.reset();
				d.add_dash(sqrt(2.0f), 10.0f);

				// ACT
				d.remove_all();
				d.move_to(1.9f, 2.7f);
				d.line_to(3.9f, 4.7f);
				mocks::path::point result3[] = { vertex(d), vertex(d), vertex(d), };

				// ASSERT
				mocks::path::point reference3[] = {
					{ 1.9f, 2.7f, path_command_move_to },
					{ 2.9f, 3.7f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference3, result3);
			}
		end_test_suite
	}
}
