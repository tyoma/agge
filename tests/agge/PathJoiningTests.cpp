#include <agge/path.h>

#include "assertex.h"
#include "mocks.h"

#include <ut/test.h>

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			class empty_path
			{
			public:
				int vertex(real_t *, real_t *)
				{	return path_command_stop;	}
			};
		}

		begin_test_suite( PathJoningTests )
			test( EmptyInnerPathsProduceEmptyJoinedPath )
			{
				// INIT
				const mocks::path empty1;
				const mocks::empty_path empty2;
				real_t x, y;

				// INIT / ACT
				join<join<mocks::path>, mocks::empty_path> joined = join<mocks::path, void>(empty1) & empty2;

				// ACT / ASSERT
				assert_equal(path_command_stop, joined.vertex(&x, &y));
			}


			test( JoinedPathReturnsPointsFromOnePathIfTheOtherIsEmpty )
			{
				// INIT
				mocks::path::point input[] = { moveto(10.0f, 13.0f), lineto(11.0f, 23.0f), lineto(3.0f, 7.0f), };
				const mocks::path p(input);
				const mocks::empty_path empty;

				// INIT / ACT
				join<join<mocks::path>, mocks::empty_path> joined1 = joining(p) & empty;

				// ACT
				mocks::path::point points1[] = { vertex(joined1), vertex(joined1), vertex(joined1), vertex(joined1), };

				// ASSERT
				mocks::path::point reference1[] = { moveto(10.0f, 13.0f), lineto(11.0f, 23.0f), lineto(3.0f, 7.0f), stop(), };

				assert_equal(reference1, points1);

				// INIT / ACT
				join<join<mocks::empty_path>, mocks::path> joined2 = joining(empty) & p;

				// ACT
				mocks::path::point points2[] = { vertex(joined2), vertex(joined2), vertex(joined2), vertex(joined2), };

				// ASSERT
				mocks::path::point reference2[] = { lineto(10.0f, 13.0f), lineto(11.0f, 23.0f), lineto(3.0f, 7.0f), stop(), };

				assert_equal(reference2, points2);
			}


			test( JoinedPathReturnsAllPointsFromNestedPaths )
			{
				// INIT
				mocks::path::point input1[] = { moveto(10.0f, 13.0f), lineto(11.0f, 23.0f), lineto(3.0f, 7.0f), };
				mocks::path::point input2[] = { lineto(-10.0f, -13.0f), lineto(3.0f, 7.0f), };
				const mocks::path p1(input1), p2(input2);

				// INIT / ACT
				join<join<mocks::path>, mocks::path> joined = joining(p1) & p2;

				// ACT
				mocks::path::point points[] = {
					vertex(joined), vertex(joined), vertex(joined),
					vertex(joined), vertex(joined),
					vertex(joined),
				};

				// ASSERT
				mocks::path::point reference[] = {
					moveto(10.0f, 13.0f), lineto(11.0f, 23.0f), lineto(3.0f, 7.0f),
					lineto(-10.0f, -13.0f), lineto(3.0f, 7.0f),
					stop(),
				};

				assert_equal(reference, points);
			}


			test( SecondPathIsJoinedViaLineTo )
			{
				// INIT
				mocks::path::point input1[] = { moveto(10.0f, 13.0f), lineto(11.0f, 23.0f), };
				mocks::path::point input2[] = { moveto(-10.0f, -13.0f), lineto(3.0f, 7.0f), };
				const mocks::path p1(input1), p2(input2);
				join<join<mocks::path>, mocks::path> joined = joining(p1) & p2;

				// ACT
				mocks::path::point points[] = {
					vertex(joined), vertex(joined),
					vertex(joined), vertex(joined),
					vertex(joined),
				};

				// ASSERT
				mocks::path::point reference[] = {
					moveto(10.0f, 13.0f), lineto(11.0f, 23.0f),
					lineto(-10.0f, -13.0f), lineto(3.0f, 7.0f),
					stop(),
				};

				assert_equal(reference, points);
			}


			test( RewindingPathProducesAllThePoints )
			{
				// INIT
				mocks::path::point input1[] = { moveto(10.0f, 13.0f), lineto(11.0f, 23.0f), };
				mocks::path::point input2[] = { moveto(-10.0f, -13.0f), lineto(3.0f, 7.0f), };
				const mocks::path p1(input1), p2(input2);
				join<join<mocks::path>, mocks::path> joined = joining(p1) & p2;
				real_t x, y;

				joined.vertex(&x, &y);

				// ACT
				joined.rewind(0);
				mocks::path::point points1[] = {
					vertex(joined), vertex(joined),
					vertex(joined), vertex(joined),
					vertex(joined),
				};

				// ASSERT
				mocks::path::point reference[] = {
					moveto(10.0f, 13.0f), lineto(11.0f, 23.0f),
					lineto(-10.0f, -13.0f), lineto(3.0f, 7.0f),
					stop(),
				};

				assert_equal(reference, points1);

				// ACT
				joined.rewind(0);
				mocks::path::point points2[] = {
					vertex(joined), vertex(joined),
					vertex(joined), vertex(joined),
					vertex(joined),
				};

				// ASSERT
				assert_equal(reference, points2);

				// INIT
				mocks::path empty;
				join<join<mocks::path>, mocks::path> joined2 = joining(empty) & p2;

				joined2.vertex(&x, &y);

				// ACT
				joined2.rewind(0);
				mocks::path::point points3[] = {
					vertex(joined2), vertex(joined2),
					vertex(joined2),
				};

				// ASSERT
				mocks::path::point reference3[] = {
					lineto(-10.0f, -13.0f), lineto(3.0f, 7.0f),
					stop(),
				};

				assert_equal(reference3, points3);
			}
		end_test_suite
	}
}
