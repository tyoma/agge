#include <agge/path.h>

#include "mocks.h"

#include <ut/assert.h>
#include <ut/test.h>
#include <vector>

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
		}

		begin_test_suite( PathGeneratorAdapterTests )
			test( EmptySourcePathAddsNothingToGenerator )
			{
				// INIT
				const mocks::path empty;
				passthrough_generator g(1.0f, 1.0f);
				real_t x, y;

				// INIT / ACT
				path_generator_adapter<mocks::path, passthrough_generator> pg = assist(empty, g);

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
				const mocks::path p1(input1);
				const mocks::path p2(input2);
				passthrough_generator g1(2.0f, 1.0f);
				passthrough_generator g2(1.0f, 2.0f);

				path_generator_adapter<mocks::path, passthrough_generator> pg1 = assist(p1, g1);
				path_generator_adapter<mocks::path, passthrough_generator> pg2 = assist(p2, g2);

				// ACT
				mocks::path::point points1[] = { vertex(pg1), vertex(pg1), vertex(pg1), vertex(pg1), vertex(pg1), };
				mocks::path::point points2[] = { vertex(pg2), vertex(pg2), vertex(pg2), vertex(pg2), vertex(pg2), vertex(pg2), vertex(pg2), vertex(pg2), };

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
				mocks::path::point input1[] = {
					{ 1.0f, 17.0f, path_command_move_to },
					{ 17.1f, 19.7f, path_command_line_to },
					{ 11.0f, 23.0f, path_command_move_to },
					{ 2.3f, 17.3f, path_command_line_to },
					{ 17.1f, 19.7f, path_command_line_to },
					{ 11.0f, 23.0f, path_command_line_to },
					{ 1.0f, 17.0f, path_command_line_to },
					{ 3.0f, 18.0f, path_command_line_to },
				};
				mocks::path p1(input1);
				passthrough_generator g;

				path_generator_adapter<mocks::path, passthrough_generator> pg1 = assist(p1, g);

				// ACT
				mocks::path::point points1[] = { vertex(pg1), vertex(pg1), };

				// ASSERT
				mocks::path::point reference1[] = {
					{ 1.0f, 17.0f, path_command_move_to },
					{ 17.1f, 19.7f, path_command_line_to },
				};

				assert_equal(reference1, points1);

				// ACT
				mocks::path::point points2[] = { vertex(pg1), vertex(pg1), vertex(pg1), vertex(pg1), vertex(pg1), vertex(pg1), vertex(pg1), };

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

				// INIT
				mocks::path::point input2[] = {
					{ 1.0f, 17.0f, path_command_move_to },
					{ 17.1f, 19.7f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_end_poly },
					{ 11.0f, 23.0f, path_command_move_to },
					{ 1.0f, 17.0f, path_command_line_to },
					{ 3.0f, 18.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					{ 11.0f, 23.0f, path_command_move_to },
					{ 1.0f, 17.0f, path_command_line_to },
					{ 3.0f, 18.0f, path_command_line_to | path_flag_close  },
				};
				mocks::path p2(input2);

				path_generator_adapter<mocks::path, passthrough_generator> pg2 = assist(p2, g);

				// ACT
				mocks::path::point points3[] = {
					vertex(pg2), vertex(pg2), vertex(pg2),
					vertex(pg2), vertex(pg2), vertex(pg2), vertex(pg2),
					vertex(pg2), vertex(pg2), vertex(pg2),
				};

				// ASSERT
				mocks::path::point reference3[] = {
					{ 11.0f, 23.0f, path_command_move_to },
					{ 1.0f, 17.0f, path_command_line_to },
					{ 3.0f, 18.0f, path_command_line_to | path_flag_close  },
				};

				assert_equal(input2, points3);
				assert_equal(reference3, g.points);
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
				path_generator_adapter<mocks::path, passthrough_generator> pg = assist(p, g);

				g.points.resize(3); // Resize to check if its cleared.

				// ACT
				vertex(pg);

				// ASSERT
				mocks::path::point reference1[] = {
					{ 2.0f, 7.0f, path_command_move_to },
					{ 37.1f, 19.7f, path_command_line_to },
				};

				assert_equal(reference1, g.points);

				// INIT
				g.points.resize(17);

				// ACT
				vertex(pg);

				// ASSERT
				assert_equal(17u, g.points.size()); // no change

				// ACT (new subpath)
				vertex(pg);

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

			class mock_path_sink
			{
			public:
				void move_to(real_t x, real_t y)
				{
					mocks::path::point p = { x, y, path_command_move_to };
					points.push_back(p);
				}

				void line_to(real_t x, real_t y)
				{
					mocks::path::point p = { x, y, path_command_line_to };
					points.push_back(p);
				}

				void close_polygon()
				{
					mocks::path::point p = { 0.0f, 0.0f, path_flag_close };
					points.push_back(p);
				}

			public:
				vector<mocks::path::point> points;
			};

			test( AddingPathToSinkTranslateMoves )
			{
				// INIT
				mocks::path::point input1[] = {
					{ 2.0f, 7.0f, path_command_move_to },
					{ 11.0f, 23.0f, path_command_move_to },
				};
				mocks::path::point input2[] = {
					{ 17.0f, 13.0f, path_command_move_to },
					{ 3.0f, 7.0f, path_command_move_to },
					{ 29.0f, 19.0f, path_command_move_to },
				};
				const mocks::path p1(input1), p2(input2);
				mock_path_sink s;

				// ACT
				add_path(s, p1);

				// ASSERT
				assert_equal(input1, s.points);

				// INIT
				s.points.clear();

				// ACT
				add_path(s, p2);

				// ASSERT
				assert_equal(input2, s.points);
			}


			test( AddingPathToSinkTranslateLineTo )
			{
				// INIT
				mocks::path::point input[] = {
					{ 2.0f, 7.0f, path_command_line_to },
					{ 11.0f, 23.0f, path_command_line_to },
					{ 3.0f, 7.0f, path_command_line_to },
				};
				const mocks::path p(input);
				mock_path_sink s;

				// ACT
				add_path(s, p);

				// ASSERT
				assert_equal(input, s.points);
			}


			test( AddingPathToSinkTranslateCloses )
			{
				// INIT
				mocks::path::point input[] = {
					{ 11.0f, 23.0f, path_command_line_to | path_flag_close },
					{ 3.0f, 7.0f, path_command_line_to | path_flag_close },
				};
				const mocks::path p(input);
				mock_path_sink s;

				// ACT
				add_path(s, p);

				// ASSERT
				mocks::path::point reference[] = {
					{ 11.0f, 23.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_flag_close },
					{ 3.0f, 7.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_flag_close },
				};

				assert_equal(reference, s.points);
			}


			test( PathIteratorRewoundOnStart )
			{
				// INIT
				mocks::path::point input[] = {
					{ 10.0f, 13.0f, path_command_move_to },
					{ 11.0f, 23.0f, path_command_line_to },
					{ 3.0f, 7.0f, path_command_line_to },
				};
				mocks::path p(input);
				mock_path_sink s;
				real_t dummy;

				p.vertex(&dummy, &dummy);
				p.vertex(&dummy, &dummy);

				// ACT
				add_path(s, p);

				// ASSERT
				assert_equal(input, s.points);
			}

		end_test_suite
	}
}
