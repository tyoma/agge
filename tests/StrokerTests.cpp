#include <agge/stroker.h>

//#include <misc/libraries/aggx/aggx_vcgen_stroke.h>

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
			mocks::path::point moveto(real_t x, real_t y)
			{
				mocks::path::point p = { x, y, path_command_move_to };
				return p;
			}

			mocks::path::point lineto(real_t x, real_t y)
			{
				mocks::path::point p = { x, y, path_command_line_to };
				return p;
			}


			class passthrough_cap : public stroke::cap
			{
				virtual void calc(points &output, real_t w, const point_r &v0, real_t d, const point_r &v1) const
				{
					output.push_back(create_point(w, d));
					output.push_back(v0);
					output.push_back(v1);
				}
			};


			class passthrough_join : public stroke::join
			{
				virtual void calc(points &output, real_t w, const point_r &v0, real_t d01, const point_r &v1, real_t d12, const point_r &v2) const
				{
					output.push_back(create_point(w, d01));
					output.push_back(v0);
					output.push_back(v1);
					output.push_back(v2);
					output.push_back(create_point(d12, 0.0f));
				}
			};


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


		begin_test_suite( PathStrokeTests )
			test( HorizontalLineIsTransformedToARect )
			{
				// INIT
				stroke s;

				s.width(2.1f);

				// ACT
				s.add_vertex(1.3f, 5.81f, path_command_move_to);
				s.add_vertex(7.1f, 5.81f, path_command_line_to);
				mocks::path::point points1[] = { get(s), get(s), get(s), get(s), get(s), get(s), };

				// ASSERT
				mocks::path::point reference1[] = {
					{ 1.3f, 6.86f, path_command_move_to },
					{ 1.3f, 4.76f, path_command_line_to },
					{ 7.1f, 4.76f, path_command_line_to },
					{ 7.1f, 6.86f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_end_poly }, { 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, points1);

				// INIT
				s.width(1.5f);
				s.remove_all();

				// ACT
				s.add_vertex(108.3f, -15.1f, path_command_move_to);
				s.add_vertex(-5.0f, -15.1f, path_command_line_to);
				mocks::path::point points2[] = { get(s), get(s), get(s), get(s), get(s), get(s), };

				// ASSERT
				mocks::path::point reference2[] = {
					{ 108.3f, -15.85f, path_command_move_to },
					{ 108.3f, -14.35f, path_command_line_to },
					{ -5.0f, -14.35f, path_command_line_to },
					{ -5.0f, -15.85f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_end_poly }, { 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, points2);
			}


			test( VerticalLineIsTransformedToARect )
			{
				// INIT
				stroke s;

				s.width(7.0f);

				// ACT
				s.add_vertex(1.3f, -5.31f, path_command_move_to);
				s.add_vertex(1.3f, 1.8f, path_command_line_to);
				mocks::path::point points1[] = { get(s), get(s), get(s), get(s), get(s), get(s), };

				// ASSERT
				mocks::path::point reference1[] = {
					{ -2.2f, -5.31f, path_command_move_to },
					{ 4.8f, -5.31f, path_command_line_to },
					{ 4.8f, 1.8f, path_command_line_to },
					{ -2.2f, 1.8f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_end_poly },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, points1);

				// INIT
				s.width(4.0f);
				s.remove_all();

				// ACT
				s.add_vertex(-108.3f, 15.1f, path_command_move_to);
				s.add_vertex(-108.3f, -10.0f, path_command_line_to);
				mocks::path::point points2[] = { get(s), get(s), get(s), get(s), get(s), get(s), };

				// ASSERT
				mocks::path::point reference2[] = {
					{ -106.3f, 15.1f, path_command_move_to },
					{ -110.3f, 15.1f, path_command_line_to },
					{ -110.3f, -10.0f, path_command_line_to },
					{ -106.3f, -10.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_end_poly },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, points2);
			}


			test( InclinedLineIsTransformedToARect )
			{
				// INIT
				stroke s;

				s.width(2.0f);

				// ACT
				s.add_vertex(1.0f, -5.0f, path_command_move_to);
				s.add_vertex(26.9808f, 10.0f, path_command_line_to);
				mocks::path::point points1[] = { get(s), get(s), get(s), get(s), get(s), get(s), };

				// ASSERT
				mocks::path::point reference1[] = {
					{ 0.5f, -4.13397f, path_command_move_to },
					{ 1.5f, -5.86603f, path_command_line_to },
					{ 27.4808f, 9.13397f, path_command_line_to },
					{ 26.4808f, 10.8660f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_end_poly }, { 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, points1);

				// INIT
				s.width(4.0f / 0.707107f);
				s.remove_all();

				// ACT
				s.add_vertex(10.0f, -10.0f, path_command_move_to);
				s.add_vertex(0.0f, 0.0f, path_command_line_to);
				mocks::path::point points2[] = { get(s), get(s), get(s), get(s), get(s), get(s), };

				// ASSERT
				mocks::path::point reference2[] = {
					{ 8.0f, -12.0f, path_command_move_to },
					{ 12.0f, -8.0f, path_command_line_to },
					{ 2.0f, 2.0f, path_command_line_to },
					{ -2.0f, -2.0f, path_command_line_to },
					{ 0.0f, 0.0f, path_command_end_poly }, { 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, points2);
			}


			test( StrokerGeneratesSinglePointSequenceForOpenSegment )
			{
				// INIT
				stroke s;

				s.add_vertex(0.0f, 0.0f, path_command_move_to);
				s.add_vertex(0.0f, 1.0f, path_command_line_to);

				s.set_cap(passthrough_cap());
				s.set_join(passthrough_join());
				s.width(4.0f);

				// ACT
				mocks::path::point points1[] = { get(s), get(s), get(s), get(s), get(s), get(s), get(s), get(s), };

				// ASSERT
				mocks::path::point reference1[] = {
					moveto(2.0f, 1.0f), lineto(0.0f, 0.0f), lineto(0.0f, 1.0f),
					lineto(2.0f, 1.0f), lineto(0.0f, 1.0f), lineto(0.0f, 0.0f),
					{ 0.0f, 0.0f, path_command_end_poly }, { 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, points1);

				// INIT
				s.remove_all();
				s.width(3.0f);
				s.add_vertex(1.0f, 2.0f, path_command_move_to);
				s.add_vertex(4.0f, 6.0f, path_command_line_to);

				// ACT
				mocks::path::point points2[] = { get(s), get(s), get(s), get(s), get(s), get(s), get(s), get(s), };

				// ASSERT
				mocks::path::point reference2[] = {
					moveto(1.5f, 5.0f), lineto(1.0f, 2.0f), lineto(4.0f, 6.0f),
					lineto(1.5f, 5.0f), lineto(4.0f, 6.0f), lineto(1.0f, 2.0f),
					{ 0.0f, 0.0f, path_command_end_poly }, { 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, points2);
			}


			test( StrokerGeneratesSinglePointSequenceForOpenPolyline )
			{
				// INIT
				stroke s;

				s.add_vertex(1.0f, 1.0f, path_command_move_to);
				s.add_vertex(4.0f, 5.0f, path_command_line_to);
				s.add_vertex(4.0f, 15.0f, path_command_line_to);

				s.set_cap(passthrough_cap());
				s.set_join(passthrough_join());
				s.width(1.0f);

				// ACT
				mocks::path::point points1[] = {
					get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s),
				};

				// ASSERT
				mocks::path::point reference1[] = {
					moveto(0.5f, 5.0f), lineto(1.0f, 1.0f), lineto(4.0f, 5.0f),
					lineto(0.5f, 5.0f), lineto(1.0f, 1.0f), lineto(4.0f, 5.0f), lineto(4.0f, 15.0f), lineto(10.0f, 0.0f),
					lineto(0.5f, 10.0f), lineto(4.0f, 15.0f), lineto(4.0f, 5.0f),
					lineto(0.5f, 10.0f), lineto(4.0f, 15.0f), lineto(4.0f, 5.0f), lineto(1.0f, 1.0f), lineto(5.0f, 0.0f),
					{ 0.0f, 0.0f, path_command_end_poly }, { 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, points1);

				// INIT
				s.remove_all();
				s.width(3.4f);
				s.add_vertex(1.0f, 1.0f, path_command_move_to);
				s.add_vertex(1.0f, 0.0f, path_command_line_to);
				s.add_vertex(5.0f, 3.0f, path_command_line_to);
				s.add_vertex(5.0f, 15.0f, path_command_line_to);

				// ACT
				mocks::path::point points2[] = {
					get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s),
				};

				// ASSERT
				mocks::path::point reference2[] = {
					moveto(1.7f, 1.0f), lineto(1.0f, 1.0f), lineto(1.0f, 0.0f),
					lineto(1.7f, 1.0f), lineto(1.0f, 1.0f), lineto(1.0f, 0.0f), lineto(5.0f, 3.0f), lineto(5.0f, 0.0f),
					lineto(1.7f, 5.0f), lineto(1.0f, 0.0f), lineto(5.0f, 3.0f), lineto(5.0f, 15.0f), lineto(12.0f, 0.0f),
					lineto(1.7f, 12.0f), lineto(5.0f, 15.0f), lineto(5.0f, 3.0f),
					lineto(1.7f, 12.0f), lineto(5.0f, 15.0f), lineto(5.0f, 3.0f), lineto(1.0f, 0.0f), lineto(5.0f, 0.0f),
					lineto(1.7f, 5.0f), lineto(5.0f, 3.0f), lineto(1.0f, 0.0f), lineto(1.0f, 1.0f), lineto(1.0f, 0.0f),
					{ 0.0f, 0.0f, path_command_end_poly }, { 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, points2);
			}

			
			test( TwoOutlinesAreGeneratedForPolygons )
			{
				// INIT
				stroke s;

				s.add_vertex(1.0f, 1.0f, path_command_move_to);
				s.add_vertex(4.0f, 5.0f, path_command_line_to);
				s.add_vertex(4.0f, 15.0f, path_command_line_to);
				s.add_vertex(0.0f, 0.0f, path_command_end_poly);

				s.set_join(passthrough_join());
				s.width(2.0f);

				// ACT
				mocks::path::point points1[] = {
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s),
				};

				// ASSERT
				mocks::path::point reference1[] = {
					moveto(1.0f, sqrt(205.0f)), lineto(4.0f, 15.0f), lineto(1.0f, 1.0f), lineto(4.0f, 5.0f), lineto(5.0f, 0.0f),
					lineto(1.0f, 5.0f), lineto(1.0f, 1.0f), lineto(4.0f, 5.0f), lineto(4.0f, 15.0f), lineto(10.0f, 0.0f),
					lineto(1.0f, 10.0f), lineto(4.0f, 5.0f), lineto(4.0f, 15.0f), lineto(1.0f, 1.0f), lineto(sqrt(205.0f), 0.0f),
					{ 0.0f, 0.0f, path_command_end_poly },
					moveto(1.0f, 5.0f), lineto(4.0f, 5.0f), lineto(1.0f, 1.0f), lineto(4.0f, 15.0f), lineto(sqrt(205.0f), 0.0f),
					lineto(1.0f, sqrt(205.0f)), lineto(1.0f, 1.0f), lineto(4.0f, 15.0f), lineto(4.0f, 5.0f), lineto(10.0f, 0.0f),
					lineto(1.0f, 10.0f), lineto(4.0f, 15.0f), lineto(4.0f, 5.0f), lineto(1.0f, 1.0f), lineto(5.0f, 0.0f),
					{ 0.0f, 0.0f, path_command_end_poly }, { 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, points1);

				// INIT
				s.remove_all();

				s.add_vertex(1.0f, 1.0f, path_command_move_to);
				s.add_vertex(5.0f, 2.0f, path_command_line_to);
				s.add_vertex(4.0f, 6.0f, path_command_line_to);
				s.add_vertex(0.0f, 5.0f, path_command_line_to);
				s.add_vertex(0.0f, 0.0f, path_command_end_poly);

				// ACT
				mocks::path::point points2[] = {
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s), get(s), get(s), get(s),
					get(s), get(s),
				};

				// ASSERT
				real_t l = sqrt(17.0f);
				mocks::path::point reference2[] = {
					moveto(1.0f, l), lineto(0.0f, 5.0f), lineto(1.0f, 1.0f), lineto(5.0f, 2.0f), lineto(l, 0.0f),
					lineto(1.0f, l), lineto(1.0f, 1.0f), lineto(5.0f, 2.0f), lineto(4.0f, 6.0f), lineto(l, 0.0f),
					lineto(1.0f, l), lineto(5.0f, 2.0f), lineto(4.0f, 6.0f), lineto(0.0f, 5.0f), lineto(l, 0.0f),
					lineto(1.0f, l), lineto(4.0f, 6.0f), lineto(0.0f, 5.0f), lineto(1.0f, 1.0f), lineto(l, 0.0f),
					{ 0.0f, 0.0f, path_command_end_poly },
					moveto(1.0f, l), lineto(5.0f, 2.0f), lineto(1.0f, 1.0f), lineto(0.0f, 5.0f), lineto(l, 0.0f),
					lineto(1.0f, l), lineto(1.0f, 1.0f), lineto(0.0f, 5.0f), lineto(4.0f, 6.0f), lineto(l, 0.0f),
					lineto(1.0f, l), lineto(0.0f, 5.0f), lineto(4.0f, 6.0f), lineto(5.0f, 2.0f), lineto(l, 0.0f),
					lineto(1.0f, l), lineto(4.0f, 6.0f), lineto(5.0f, 2.0f), lineto(1.0f, 1.0f), lineto(l, 0.0f),
					{ 0.0f, 0.0f, path_command_end_poly }, { 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, points2);
			}


			test( EmptyStrokerStopsImmediately )
			{
				// INIT
				stroke s;

				// ACT / ASSERT
				assert_equal(path_command_stop, get(s).command);

				// INIT
				s.add_vertex(1.0f, 2.0f, path_command_move_to);

				// ACT / ASSERT
				assert_equal(path_command_stop, get(s).command);

				// INIT
				s.add_vertex(1.0f, 2.0f, path_command_line_to);
				s.add_vertex(0.0f, 0.0f, path_command_end_poly);

				// ACT / ASSERT
				assert_equal(path_command_stop, get(s).command);
			}

		end_test_suite
	}
}
