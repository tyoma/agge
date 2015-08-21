#include <agge/stroker.h>

#include "helpers.h"
#include "mocks.h"

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			class passthrough_generator : public mocks::path
			{
			public:
				passthrough_generator()
					: kx(1.0f), ky(1.0f)
				{	}

				passthrough_generator(real_t kx_, real_t ky_)
					: kx(kx_), ky(ky_)
				{	}

				void remove_all()
				{
					points.clear();
					position = 0;
				}

				void add_vertex(real_t x, real_t y, int command)
				{
					point p = { kx * x, ky * y, command };
					points.push_back(p);
				}

			public:
				real_t kx, ky;
			};

			template <typename SourceT>
			mocks::path::point get(SourceT &source)
			{
				mocks::path::point p = { 0 };

				p.command = source.vertex(&p.x, &p.y);
				return p;
			}
		}

		begin_test_suite( PathGeneratorAdapterTests )
			test( EmptySourcePathAddsNothingToGenerator )
			{
				// INIT
				mocks::path empty;
				passthrough_generator g(1.0f, 1.0f);
				real_t x, y;

				// INIT / ACT
				path_generator_adapter<mocks::path, passthrough_generator> pg(empty, g);

				// ACT / ASSERT
				assert_equal(path_command_stop, (int)pg.vertex(&x, &y));

				// ASSERT
				assert_is_empty(g.points);
			}


			test( SinglePolylineIsPassedThroughAsIs )
			{
				// INIT
				mocks::path::point input1[] = {
					{ 1.0f, 17.0f, path_command_move_to },
					{ 17.1f, 19.7f, path_command_line_to },
					{ 11.0f, 23.0f, path_command_line_to },
					{ 2.3f, 17.3f, path_command_line_to },
				};
				mocks::path::point input2[] = {
					{ 1.0f, 17.0f, path_command_move_to },
					{ 17.1f, 19.7f, path_command_line_to },
					{ 11.0f, 23.0f, path_command_line_to },
					{ 2.3f, 17.3f, path_command_line_to },
					{ 17.1f, 19.7f, path_command_line_to },
					{ 11.0f, 23.0f, path_command_line_to },
					{ 1.0f, 17.0f, path_command_line_to },
				};
				mocks::path p1(input1);
				mocks::path p2(input2);
				passthrough_generator g1(2.0f, 1.0f);
				passthrough_generator g2(1.0f, 2.0f);

				path_generator_adapter<mocks::path, passthrough_generator> pg1(p1, g1);
				path_generator_adapter<mocks::path, passthrough_generator> pg2(p2, g2);

				// ACT
				mocks::path::point points1[] = { get(pg1), get(pg1), get(pg1), get(pg1), get(pg1), };
				mocks::path::point points2[] = { get(pg2), get(pg2), get(pg2), get(pg2), get(pg2), get(pg2), get(pg2), get(pg2), };

				// ASSERT
				mocks::path::point reference1[] = {
					{ 2.0f, 17.0f, path_command_move_to },
					{ 34.2f, 19.7f, path_command_line_to },
					{ 22.0f, 23.0f, path_command_line_to },
					{ 4.6f, 17.3f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};
				mocks::path::point reference2[] = {
					{ 1.0f, 34.0f, path_command_move_to },
					{ 17.1f, 39.4f, path_command_line_to },
					{ 11.0f, 46.0f, path_command_line_to },
					{ 2.3f, 34.6f, path_command_line_to },
					{ 17.1f, 39.4f, path_command_line_to },
					{ 11.0f, 46.0f, path_command_line_to },
					{ 1.0f, 34.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, points1);
				assert_equal(reference2, points2);
			}


			test( MultiPolylineIsPassedThroughAsIs )
			{
				// INIT
				mocks::path::point input[] = {
					{ 1.0f, 17.0f, path_command_move_to },
					{ 17.1f, 19.7f, path_command_line_to },
					{ 11.0f, 23.0f, path_command_move_to },
					{ 2.3f, 17.3f, path_command_line_to },
					{ 17.1f, 19.7f, path_command_line_to },
					{ 11.0f, 23.0f, path_command_line_to },
					{ 1.0f, 17.0f, path_command_line_to },
					{ 3.0f, 18.0f, path_command_line_to },
				};
				mocks::path p(input);
				passthrough_generator g;

				path_generator_adapter<mocks::path, passthrough_generator> pg(p, g);

				// ACT
				mocks::path::point points1[] = { get(pg), get(pg), };

				// ASSERT
				mocks::path::point reference1[] = {
					{ 1.0f, 17.0f, path_command_move_to },
					{ 17.1f, 19.7f, path_command_line_to },
				};

				assert_equal(reference1, points1);

				// ACT
				mocks::path::point points2[] = { get(pg), get(pg), get(pg), get(pg), get(pg), get(pg), get(pg), };

				// ASSERT
				mocks::path::point reference2[] = {
					{ 11.0f, 23.0f, path_command_move_to },
					{ 2.3f, 17.3f, path_command_line_to },
					{ 17.1f, 19.7f, path_command_line_to },
					{ 11.0f, 23.0f, path_command_line_to },
					{ 1.0f, 17.0f, path_command_line_to },
					{ 3.0f, 18.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, points2);
			}


			test( SubpathIsFullyPassedToGenerator )
			{
				// INIT
				mocks::path::point input[] = {
					{ 2.0f, 7.0f, path_command_move_to },
					{ 37.1f, 19.7f, path_command_line_to },
					{ 11.0f, 23.0f, path_command_move_to },
					{ 2.3f, 7.3f, path_command_line_to },
					{ 171.1f, 19.7f, path_command_line_to },
					{ 11.0f, 23.0f, path_command_line_to },
					{ 1.0f, 17.0f, path_command_line_to },
				};
				mocks::path p(input);
				passthrough_generator g;
				path_generator_adapter<mocks::path, passthrough_generator> pg(p, g);

				g.points.resize(3); // Resize to check if its cleared.

				// ACT
				get(pg);

				// ASSERT
				mocks::path::point reference1[] = {
					{ 2.0f, 7.0f, path_command_move_to },
					{ 37.1f, 19.7f, path_command_line_to },
				};

				assert_equal(reference1, g.points);

				// INIT
				g.points.resize(17);

				// ACT
				get(pg);

				// ASSERT
				assert_equal(17u, g.points.size()); // no change

				// ACT (new subpath)
				get(pg);

				// ASSERT
				mocks::path::point reference2[] = {
					{ 11.0f, 23.0f, path_command_move_to },
					{ 2.3f, 7.3f, path_command_line_to },
					{ 171.1f, 19.7f, path_command_line_to },
					{ 11.0f, 23.0f, path_command_line_to },
					{ 1.0f, 17.0f, path_command_line_to },
				};

				assert_equal(reference2, g.points);
			}

		end_test_suite
	}
}
