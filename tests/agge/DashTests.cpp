#include <agge/dash.h>

#include "mocks.h"

#include <ut/assert.h>
#include <ut/test.h>

namespace agge
{
	namespace tests
	{
		begin_test_suite( DashTests )
			test( SegmentAreOutputtedAsIsIfDashIsLong )
			{
				// INIT
				dash d;

				// INIT / ACT
				d.add_dash(6.0f, 1.0f);

				// ACT
				move_to(d, 1.1f, 17.0f);
				line_to(d, 4.1f, 13.0f);
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
				move_to(d, 1.0f, 1.0f);
				line_to(d, 2.5f, 3.0f);
				line_to(d, 2.7f, 3.1f);
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


			test( SegmentIsLimitedToADash )
			{
				// INIT
				dash d;

				// INIT / ACT
				d.add_dash(5.0f, 10.0f);

				// ACT
				move_to(d, 1.3f, 17.0f);
				line_to(d, 5.8f, 11.0f);
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
				move_to(d, 1.0f, -1.0f);
				line_to(d, 1.0f, 10.0f);
				mocks::path::point result2[] = { vertex(d), vertex(d), vertex(d), };

				// ASSERT
				mocks::path::point reference2[] = {
					{ 1.0f, -1.0f, path_command_move_to },
					{ 1.0f, 4.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, result2);

				// INIT / ACT
				d.remove_all_dashes();
				d.add_dash(sqrt(2.0f), 10.0f);

				// ACT
				d.remove_all();
				move_to(d, 1.9f, 2.7f);
				line_to(d, 3.9f, 4.7f);
				mocks::path::point result3[] = { vertex(d), vertex(d), vertex(d), };

				// ASSERT
				mocks::path::point reference3[] = {
					{ 1.9f, 2.7f, path_command_move_to },
					{ 2.9f, 3.7f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference3, result3);
			}


			test( ThePatternIsAppliedPeriodically )
			{
				// INIT
				dash d;

				d.add_dash(5.0f, 10.0f);

				// ACT
				move_to(d, 0.0f, 0.0f);
				line_to(d, 44.99f, 0.0f);
				mocks::path::point result1[] = {
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d),
				};

				// ASSERT
				mocks::path::point reference1[] = {
					{ 0.0f, 0.0f, path_command_move_to },
					{ 5.0f, 0.0f, path_command_line_to },
					{ 15.0f, 0.0f, path_command_move_to },
					{ 20.0f, 0.0f, path_command_line_to },
					{ 30.0f, 0.0f, path_command_move_to },
					{ 35.0f, 0.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, result1);

				// INIT
				d.remove_all_dashes();
				d.add_dash(5.0f, 7.0f);
				d.remove_all();

				// ACT (pattern end coincides with segment end)
				move_to(d, 0.0f, 0.0f);
				line_to(d, 24.0f, 0.0f);
				mocks::path::point result2[] = {
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d),
				};

				// ASSERT
				mocks::path::point reference2[] = {
					{ 0.0f, 0.0f, path_command_move_to },
					{ 5.0f, 0.0f, path_command_line_to },
					{ 12.0f, 0.0f, path_command_move_to },
					{ 17.0f, 0.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, result2);
			}


			test( PointsWithinGapsAreNotEmitted )
			{
				// INIT
				dash d;

				d.add_dash(2.5f, 4.0f);

				// ACT
				move_to(d, 0.0f, 0.0f);
				line_to(d, 3.0f, 0.0f);
				line_to(d, 6.0f, 0.0f);
				line_to(d, 9.1f, 0.0f);
				line_to(d, 12.0f, 0.0f);
				line_to(d, 15.0f, 0.0f);

				mocks::path::point result[] = {
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d),
				};

				// ASSERT
				mocks::path::point reference[] = {
					{ 0.0f, 0.0f, path_command_move_to }, { 2.5f, 0.0f, path_command_line_to },
					{ 6.5f, 0.0f, path_command_move_to }, { 9.0f, 0.0f, path_command_line_to },
					{ 13.0f, 0.0f, path_command_move_to }, { 15.0f, 0.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference, result);
			}


			test( EmittedSourcePointsCanBeFollowedWithGap )
			{
				// INIT
				dash d;

				d.add_dash(3.0f, 2.5f);

				// ACT
				move_to(d, 0.0f, 0.0f);
				line_to(d, 0.0f, 2.0f);
				line_to(d, 0.0f, 4.0f);
				line_to(d, 0.0f, 6.0f);
				line_to(d, 0.0f, 8.0f);

				mocks::path::point result[] = {
					vertex(d), vertex(d), vertex(d),
					vertex(d), vertex(d), vertex(d),
					vertex(d),
				};

				// ASSERT
				mocks::path::point reference[] = {
					{ 0.0f, 0.0f, path_command_move_to }, { 0.0f, 2.0f, path_command_line_to }, { 0.0f, 3.0f, path_command_line_to },
					{ 0.0f, 5.5f, path_command_move_to }, { 0.0f, 6.0f, path_command_line_to }, { 0.0f, 8.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference, result);
			}


			test( MultipleDashPatternIsSupported )
			{
				// INIT
				dash d;

				d.add_dash(3.0f, 1.0f);
				d.add_dash(2.0f, 0.5f);

				// ACT
				move_to(d, 1.0f, 2.0f);
				line_to(d, 5.5f, 2.0f);
				line_to(d, 5.5f, 12.0f);

				mocks::path::point result1[] = {
					vertex(d), vertex(d),
					vertex(d), vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d),
				};

				// ASSERT
				mocks::path::point reference1[] = {
					{ 1.0f, 2.0f, path_command_move_to }, { 4.0f, 2.0f, path_command_line_to },
					{ 5.0f, 2.0f, path_command_move_to }, { 5.5f, 2.0f, path_command_line_to }, { 5.5f, 3.5f, path_command_line_to },
					{ 5.5f, 4.0f, path_command_move_to }, { 5.5f, 7.0f, path_command_line_to },
					{ 5.5f, 8.0f, path_command_move_to }, { 5.5f, 10.0f, path_command_line_to },
					{ 5.5f, 10.5f, path_command_move_to }, { 5.5f, 12.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, result1);
			}


			test( ClosedPolygonProducesDashesForTheClosingSegment )
			{
				// INIT
				dash d;

				d.add_dash(2.0f, 1.0f);

				// ACT
				move_to(d, -2.0f, 2.0f);
				line_to(d, -2.0f, -1.0f);
				line_to(d, 2.0f, -1.0f);
				end_poly(d, true);

				mocks::path::point result[] = {
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d),
				};

				// ASSERT
				mocks::path::point reference[] = {
					{ -2.0f, 2.0f, path_command_move_to }, { -2.0f, 0.0f, path_command_line_to },
					{ -2.0f, -1.0f, path_command_move_to }, { 0.0f, -1.0f, path_command_line_to },
					{ 1.0f, -1.0f, path_command_move_to }, { 2.0f, -1.0f, path_command_line_to }, { 1.2f, -0.4f, path_command_line_to },
					{ 0.4f, 0.2f, path_command_move_to }, { -1.2f, 1.4f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference, result);
			}


			test( DashStartIsRespectedForASingleDashWhenGapIsHit )
			{
				// INIT
				dash d;

				d.add_dash(2.0f, 1.0f);

				// ACT
				d.dash_start(0.3f);

				move_to(d, 0.0f, 0.0f);
				line_to(d, 6.0f, 0.0f);

				mocks::path::point result1[] = {
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d),
				};

				// ASSERT
				mocks::path::point reference1[] = {
					{ 0.0f, 0.0f, path_command_move_to }, { 1.7f, 0.0f, path_command_line_to },
					{ 2.7f, 0.0f, path_command_move_to }, { 4.7f, 0.0f, path_command_line_to },
					{ 5.7f, 0.0f, path_command_move_to }, { 6.0f, 0.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, result1);

				// INIT
				d.remove_all();

				// ACT
				d.dash_start(0.43f);

				move_to(d, 0.0f, 0.0f);
				line_to(d, 0.2f, 0.0f);
				line_to(d, 3.5f, 0.0f);

				mocks::path::point result2[] = {
					vertex(d), vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d),
				};

				// ASSERT
				mocks::path::point reference2[] = {
					{ 0.0f, 0.0f, path_command_move_to }, { 0.2f, 0.0f, path_command_line_to }, { 1.57f, 0.0f, path_command_line_to },
					{ 2.57f, 0.0f, path_command_move_to }, { 3.5f, 0.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, result2);
			}


			test( DashStartIsRespectedForASingleDashWhenDashIsHit )
			{
				// INIT
				dash d;

				d.add_dash(2.0f, 1.0f);

				// ACT
				d.dash_start(2.15f);

				move_to(d, 0.0f, 0.0f);
				line_to(d, 6.0f, 0.0f);

				mocks::path::point result1[] = {
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d),
				};

				// ASSERT
				mocks::path::point reference1[] = {
					{ 0.85f, 0.0f, path_command_move_to }, { 2.85f, 0.0f, path_command_line_to },
					{ 3.85f, 0.0f, path_command_move_to }, { 5.85f, 0.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, result1);
			}


			test( DashStartIsRespectedForAWholePeriod )
			{
				// INIT
				dash d;

				d.add_dash(2.0f, 1.0f);
				d.add_dash(1.0f, 3.0f);
				d.add_dash(0.5f, 1.2f);

				// ACT
				d.dash_start(8.7f);

				move_to(d, 0.0f, 0.0f);
				line_to(d, 9.5f, 0.0f);

				mocks::path::point result[] = {
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d),
				};

				// ASSERT
				mocks::path::point reference[] = {
					{ 0.0f, 0.0f, path_command_move_to }, { 2.0f, 0.0f, path_command_line_to },
					{ 3.0f, 0.0f, path_command_move_to }, { 4.0f, 0.0f, path_command_line_to },
					{ 7.0f, 0.0f, path_command_move_to }, { 7.5f, 0.0f, path_command_line_to },
					{ 8.7f, 0.0f, path_command_move_to }, { 9.5f, 0.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference, result);
			}


			test( DashStartIsRespectedForAMultiDashWhenDashIsHit )
			{
				// INIT
				dash d;

				d.add_dash(2.0f, 1.0f);
				d.add_dash(1.0f, 3.5f);
				d.add_dash(0.5f, 0.5f);

				// ACT
				d.dash_start(0.3f);

				move_to(d, 0.0f, 0.0f);
				line_to(d, 9.5f, 0.0f);

				mocks::path::point result1[] = {
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d),
				};

				// ASSERT
				mocks::path::point reference1[] = {
					{ 0.0f, 0.0f, path_command_move_to }, { 1.7f, 0.0f, path_command_line_to },
					{ 2.7f, 0.0f, path_command_move_to }, { 3.7f, 0.0f, path_command_line_to },
					{ 7.2f, 0.0f, path_command_move_to }, { 7.7f, 0.0f, path_command_line_to },
					{ 8.2f, 0.0f, path_command_move_to }, { 9.5f, 0.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, result1);

				// INIT
				d.remove_all();

				// ACT
				d.dash_start(7.75f);

				move_to(d, 0.0f, 0.0f);
				line_to(d, 9.5f, 0.0f);

				mocks::path::point result2[] = {
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d),
				};

				// ASSERT
				mocks::path::point reference2[] = {
					{ 0.0f, 0.0f, path_command_move_to }, { 0.25f, 0.0f, path_command_line_to },
					{ 0.75f, 0.0f, path_command_move_to }, { 2.75f, 0.0f, path_command_line_to },
					{ 3.75f, 0.0f, path_command_move_to }, { 4.75f, 0.0f, path_command_line_to },
					{ 8.25f, 0.0f, path_command_move_to }, { 8.75f, 0.0f, path_command_line_to },
					{ 9.25f, 0.0f, path_command_move_to }, { 9.5f, 0.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, result2);

				// INIT
				d.remove_all();

				// ACT
				d.dash_start(7.75f + 8.5f);

				move_to(d, 0.0f, 0.0f);
				line_to(d, 9.5f, 0.0f);

				mocks::path::point result3[] = {
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d),
				};

				// ASSERT
				assert_equal(reference2, result3);

				// INIT
				d.remove_all();

				// ACT
				d.dash_start(7.75f + 3.0f * 8.5f);

				move_to(d, 0.0f, 0.0f);
				line_to(d, 9.5f, 0.0f);

				mocks::path::point result4[] = {
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d), vertex(d),
					vertex(d),
				};

				// ASSERT
				assert_equal(reference2, result4);
			}
		end_test_suite
	}
}
