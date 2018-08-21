#include <agge/stroke.h>

#include <agge/stroke_features.h>

#include "helpers.h"
#include "mocks.h"

#include <memory>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			template <typename BaseT>
			class counted : public BaseT
			{
			public:
				counted(size_t &alive)
					: _alive(alive)
				{	++_alive;	}

				counted(const counted &other)
					: _alive(other._alive)
				{	++_alive;	}

				~counted()
				{	--_alive;	}

			private:
				const counted &operator =(const counted &rhs);

			private:
				size_t &_alive;
			};

			class passthrough_cap : public counted<stroke::cap>
			{
			public:
				passthrough_cap(size_t &alive)
					: counted<stroke::cap>(alive)
				{	}

			private:
				virtual void calc(points &output, real_t w, const point_r &v0, real_t d, const point_r &v1) const
				{
					output.push_back(create_point(w, d));
					output.push_back(v0);
					output.push_back(v1);
				}
			};


			class passthrough_join : public counted<stroke::join>
			{
			public:
				passthrough_join(size_t &alive)
					: counted<stroke::join>(alive)
				{	}

			private:
				virtual void calc(points &output, real_t w, const point_r &v0, real_t d01, const point_r &v1, real_t d12, const point_r &v2) const
				{
					output.push_back(create_point(w, d01));
					output.push_back(v0);
					output.push_back(v1);
					output.push_back(v2);
					output.push_back(create_point(d12, static_cast<real_t>(0)));
				}
			};


			class partial_passthrough_join : public stroke::join
			{
				virtual void calc(points &output, real_t /*w*/, const point_r &/*v0*/, real_t /*d01*/, const point_r &v1,
					real_t /*d12*/, const point_r &/*v2*/) const
				{	output.push_back(v1);	}
			};
		}


		begin_test_suite( PathStrokeTests )

			size_t amount_alive;

			test( HorizontalLineIsTransformedToARect )
			{
				// INIT
				stroke s;

				s.width(2.1f);
				s.set_cap(caps::butt());

				// ACT
				move_to(s, 1.3f, 5.81f);
				line_to(s, 7.1f, 5.81f);
				mocks::path::point points1[] = { vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), };

				// ASSERT
				mocks::path::point reference1[] = {
					moveto(1.3f, 6.86f), lineto(1.3f, 4.76f),
					lineto(7.1f, 4.76f), lineto(7.1f, 6.86f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, points1);

				// INIT
				s.width(1.5f);
				s.remove_all();

				// ACT
				move_to(s, 108.3f, -15.1f);
				line_to(s, -5.0f, -15.1f);
				mocks::path::point points2[] = { vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), };

				// ASSERT
				mocks::path::point reference2[] = {
					moveto(108.3f, -15.85f), lineto(108.3f, -14.35f),
					lineto(-5.0f, -14.35f), lineto(-5.0f, -15.85f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, points2);
			}


			test( VerticalLineIsTransformedToARect )
			{
				// INIT
				stroke s;

				s.width(7.0f);
				s.set_cap(caps::butt());

				// ACT
				move_to(s, 1.3f, -5.31f);
				line_to(s, 1.3f, 1.8f);
				mocks::path::point points1[] = { vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), };

				// ASSERT
				mocks::path::point reference1[] = {
					moveto(-2.2f, -5.31f), lineto(4.8f, -5.31f),
					lineto(4.8f, 1.8f), lineto(-2.2f, 1.8f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, points1);

				// INIT
				s.width(4.0f);
				s.remove_all();

				// ACT
				move_to(s, -108.3f, 15.1f);
				line_to(s, -108.3f, -10.0f);
				mocks::path::point points2[] = { vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), };

				// ASSERT
				mocks::path::point reference2[] = {
					moveto(-106.3f, 15.1f), lineto(-110.3f, 15.1f),
					lineto(-110.3f, -10.0f), lineto(-106.3f, -10.0f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, points2);
			}


			test( InclinedLineIsTransformedToARect )
			{
				// INIT
				stroke s;

				s.width(2.0f);
				s.set_cap(caps::butt());

				// ACT
				move_to(s, 1.0f, -5.0f);
				line_to(s, 26.9808f, 10.0f);
				mocks::path::point points1[] = { vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), };

				// ASSERT
				mocks::path::point reference1[] = {
					moveto(0.5f, -4.13397f), lineto(1.5f, -5.86603f),
					lineto(27.4808f, 9.13397f), lineto(26.4808f, 10.8660f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, points1);

				// INIT
				s.width(4.0f / 0.707107f);
				s.remove_all();

				// ACT
				move_to(s, 10.0f, -10.0f);
				line_to(s, 0.0f, 0.0f);
				mocks::path::point points2[] = { vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), };

				// ASSERT
				mocks::path::point reference2[] = {
					moveto(8.0f, -12.0f), lineto(12.0f, -8.0f),
					lineto(2.0f, 2.0f), lineto(-2.0f, -2.0f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, points2);
			}


			test( StrokerGeneratesSinglePointSequenceForOpenSegment )
			{
				// INIT
				stroke s;

				move_to(s, 0.0f, 0.0f);
				line_to(s, 0.0f, 1.0f);

				s.set_cap(passthrough_cap(amount_alive));
				s.set_join(passthrough_join(amount_alive));
				s.width(4.0f);

				// ACT
				mocks::path::point points1[] = { vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), };

				// ASSERT
				mocks::path::point reference1[] = {
					moveto(2.0f, 1.0f), lineto(0.0f, 0.0f), lineto(0.0f, 1.0f),
					lineto(2.0f, 1.0f), lineto(0.0f, 1.0f), lineto(0.0f, 0.0f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, points1);

				// INIT
				s.remove_all();
				s.width(3.0f);
				move_to(s, 1.0f, 2.0f);
				line_to(s, 4.0f, 6.0f);

				// ACT
				mocks::path::point points2[] = { vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), vertex(s), };

				// ASSERT
				mocks::path::point reference2[] = {
					moveto(1.5f, 5.0f), lineto(1.0f, 2.0f), lineto(4.0f, 6.0f),
					lineto(1.5f, 5.0f), lineto(4.0f, 6.0f), lineto(1.0f, 2.0f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, points2);
			}


			test( StrokerGeneratesSinglePointSequenceForOpenPolyline )
			{
				// INIT
				stroke s;

				move_to(s, 1.0f, 1.0f);
				line_to(s, 4.0f, 5.0f);
				line_to(s, 4.0f, 15.0f);

				s.set_cap(passthrough_cap(amount_alive));
				s.set_join(passthrough_join(amount_alive));
				s.width(1.0f);

				// ACT
				mocks::path::point points1[] = {
					vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s),
				};

				// ASSERT
				mocks::path::point reference1[] = {
					moveto(0.5f, 5.0f), lineto(1.0f, 1.0f), lineto(4.0f, 5.0f),
					lineto(0.5f, 5.0f), lineto(1.0f, 1.0f), lineto(4.0f, 5.0f), lineto(4.0f, 15.0f), lineto(10.0f, 0.0f),
					lineto(0.5f, 10.0f), lineto(4.0f, 15.0f), lineto(4.0f, 5.0f),
					lineto(0.5f, 10.0f), lineto(4.0f, 15.0f), lineto(4.0f, 5.0f), lineto(1.0f, 1.0f), lineto(5.0f, 0.0f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, points1);

				// INIT
				s.remove_all();
				s.width(3.4f);
				move_to(s, 1.0f, 1.0f);
				line_to(s, 1.0f, 0.0f);
				line_to(s, 5.0f, 3.0f);
				line_to(s, 5.0f, 15.0f);

				// ACT
				mocks::path::point points2[] = {
					vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s),
				};

				// ASSERT
				mocks::path::point reference2[] = {
					moveto(1.7f, 1.0f), lineto(1.0f, 1.0f), lineto(1.0f, 0.0f),
					lineto(1.7f, 1.0f), lineto(1.0f, 1.0f), lineto(1.0f, 0.0f), lineto(5.0f, 3.0f), lineto(5.0f, 0.0f),
					lineto(1.7f, 5.0f), lineto(1.0f, 0.0f), lineto(5.0f, 3.0f), lineto(5.0f, 15.0f), lineto(12.0f, 0.0f),
					lineto(1.7f, 12.0f), lineto(5.0f, 15.0f), lineto(5.0f, 3.0f),
					lineto(1.7f, 12.0f), lineto(5.0f, 15.0f), lineto(5.0f, 3.0f), lineto(1.0f, 0.0f), lineto(5.0f, 0.0f),
					lineto(1.7f, 5.0f), lineto(5.0f, 3.0f), lineto(1.0f, 0.0f), lineto(1.0f, 1.0f), lineto(1.0f, 0.0f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, points2);
			}

			
			test( TwoOutlinesAreGeneratedForPolygons )
			{
				// INIT
				stroke s;

				move_to(s, 1.0f, 1.0f);
				line_to(s, 4.0f, 5.0f);
				line_to(s, 4.0f, 15.0f);
				end_poly(s, true);

				s.set_join(passthrough_join(amount_alive));
				s.width(2.0f);

				// ACT
				mocks::path::point points1[] = {
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s),
				};

				// ASSERT
				mocks::path::point reference1[] = {
					moveto(1.0f, sqrt(205.0f)), lineto(4.0f, 15.0f), lineto(1.0f, 1.0f), lineto(4.0f, 5.0f), lineto(5.0f, 0.0f),
					lineto(1.0f, 5.0f), lineto(1.0f, 1.0f), lineto(4.0f, 5.0f), lineto(4.0f, 15.0f), lineto(10.0f, 0.0f),
					lineto(1.0f, 10.0f), lineto(4.0f, 5.0f), lineto(4.0f, 15.0f), lineto(1.0f, 1.0f), lineto(sqrt(205.0f), 0.0f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					moveto(1.0f, 5.0f), lineto(4.0f, 5.0f), lineto(1.0f, 1.0f), lineto(4.0f, 15.0f), lineto(sqrt(205.0f), 0.0f),
					lineto(1.0f, sqrt(205.0f)), lineto(1.0f, 1.0f), lineto(4.0f, 15.0f), lineto(4.0f, 5.0f), lineto(10.0f, 0.0f),
					lineto(1.0f, 10.0f), lineto(4.0f, 15.0f), lineto(4.0f, 5.0f), lineto(1.0f, 1.0f), lineto(5.0f, 0.0f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference1, points1);

				// INIT
				s.remove_all();

				move_to(s, 1.0f, 1.0f);
				line_to(s, 5.0f, 2.0f);
				line_to(s, 4.0f, 6.0f);
				line_to(s, 0.0f, 5.0f);
				end_poly(s, true);

				// ACT
				mocks::path::point points2[] = {
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s), vertex(s), vertex(s), vertex(s),
					vertex(s), vertex(s),
				};

				// ASSERT
				real_t l = sqrt(17.0f);
				mocks::path::point reference2[] = {
					moveto(1.0f, l), lineto(0.0f, 5.0f), lineto(1.0f, 1.0f), lineto(5.0f, 2.0f), lineto(l, 0.0f),
					lineto(1.0f, l), lineto(1.0f, 1.0f), lineto(5.0f, 2.0f), lineto(4.0f, 6.0f), lineto(l, 0.0f),
					lineto(1.0f, l), lineto(5.0f, 2.0f), lineto(4.0f, 6.0f), lineto(0.0f, 5.0f), lineto(l, 0.0f),
					lineto(1.0f, l), lineto(4.0f, 6.0f), lineto(0.0f, 5.0f), lineto(1.0f, 1.0f), lineto(l, 0.0f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					moveto(1.0f, l), lineto(5.0f, 2.0f), lineto(1.0f, 1.0f), lineto(0.0f, 5.0f), lineto(l, 0.0f),
					lineto(1.0f, l), lineto(1.0f, 1.0f), lineto(0.0f, 5.0f), lineto(4.0f, 6.0f), lineto(l, 0.0f),
					lineto(1.0f, l), lineto(0.0f, 5.0f), lineto(4.0f, 6.0f), lineto(5.0f, 2.0f), lineto(l, 0.0f),
					lineto(1.0f, l), lineto(4.0f, 6.0f), lineto(5.0f, 2.0f), lineto(1.0f, 1.0f), lineto(l, 0.0f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference2, points2);
			}

			
			test( TwoOutlinesAreGeneratedForEmbeddedClose )
			{
				// INIT
				stroke s;

				move_to(s, 1.0f, 1.0f);
				line_to(s, 4.0f, 5.0f);
				line_to(s, 4.0f, 15.0f, true);

				s.set_join(partial_passthrough_join());

				// ACT
				mocks::path::point points[] = {
					vertex(s), vertex(s), vertex(s),
					vertex(s),
					vertex(s), vertex(s), vertex(s),
					vertex(s),
					vertex(s),
				};

				// ASSERT
				mocks::path::point reference[] = {
					moveto(1.0f, 1.0f), lineto(4.0f, 5.0f), lineto(4.0f, 15.0f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					 moveto(1.0f, 1.0f), lineto(4.0f, 15.0f), lineto(4.0f, 5.0f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
					{ 0.0f, 0.0f, path_command_stop },
				};

				assert_equal(reference, points);
			}


			test( EmptyStrokerStopsImmediately )
			{
				// INIT
				stroke s;

				// ACT / ASSERT
				assert_equal(path_command_stop, vertex(s).command);

				// INIT
				move_to(s, 1.0f, 2.0f);

				// ACT / ASSERT
				assert_equal(path_command_stop, vertex(s).command);

				// INIT
				line_to(s, 1.0f, 2.0f, true);

				// ACT / ASSERT
				assert_equal(path_command_stop, vertex(s).command);
			}


			test( ClearedStrokeIsEmpty )
			{
				// INIT
				stroke s;

				s.set_cap(passthrough_cap(amount_alive));

				move_to(s, 1.0f, 2.0f);
				line_to(s, 5.0f, 7.0f);
				line_to(s, 0.0f, 0.0f, true);

				// ACT
				s.remove_all();

				// ACT / ASSERT
				assert_equal(path_command_stop, vertex(s).command);

				// INIT ('ready' flag was cleared)
				move_to(s, 1.0f, 2.0f);

				// ACT / ASSERT
				assert_equal(path_command_stop, vertex(s).command);

				// INIT ('closed' flag was cleared)
				line_to(s, 1.5f, 7.1f);

				// ACT / ASSERT
				assert_equal(path_command_move_to, vertex(s).command);
			}


			test( StrokerStoresCopiesOfCapsAndJoins )
			{
				// INIT
				size_t joins1 = 0, caps1 = 0, joins2 = 0, caps2 = 0;
				stroke s;

				// ACT
				s.set_join(passthrough_join(joins1));
				s.set_cap(passthrough_cap(caps1));

				// ASSERT
				assert_equal(1u, joins1);
				assert_equal(1u, caps1);

				// ACT
				s.set_join(passthrough_join(joins2));

				// ASSERT
				assert_equal(0u, joins1);
				assert_equal(1u, joins2);
				assert_equal(1u, caps1);

				// ACT
				s.set_cap(passthrough_cap(caps2));

				// ASSERT
				assert_equal(0u, joins1);
				assert_equal(1u, joins2);
				assert_equal(0u, caps1);
				assert_equal(1u, caps2);
			}


			test( CapsAndJoinsStoredAreDeletedOnStrokerDestruction )
			{
				// INIT
				size_t joins = 0, caps = 0;
				auto_ptr<stroke> s(new stroke);

				s->set_join(passthrough_join(joins));
				s->set_cap(passthrough_cap(caps));

				// ACT
				s.reset();

				// ASSERT
				assert_equal(0u, joins);
				assert_equal(0u, caps);
			}


			test( CloselyLocatedVerticesAreIgnoredInOpenPath )
			{
				// INIT
				stroke s;

				move_to(s, 1.0f, 2.0f);

				// ACT
				line_to(s, 1.0f + 0.5f * distance_epsilon, 2.0f + 0.2f * distance_epsilon);

				// ACT / ASSERT
				assert_equal(path_command_stop, vertex(s).command);
			}


			test( LastPointIsRemovedIfCloseToTheFirstOnPathClosure )
			{
				// INIT
				stroke s;

				s.set_join(partial_passthrough_join());

				move_to(s, 2.0f, 5.0f);
				line_to(s, 0.3f, 19.2f);
				line_to(s, 8.2f, 10.0f);

				// ACT
				line_to(s, 2.0f + 0.3f * distance_epsilon, 5.0f + 0.4f * distance_epsilon);
				end_poly(s, true);

				// ASSERT
				mocks::path::point points[] = {
					vertex(s), vertex(s), vertex(s),
					vertex(s),
				};

				mocks::path::point reference[] = {
					moveto(2.0f, 5.0f), lineto(0.3f, 19.2f), lineto(8.2f, 10.0f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
				};

				assert_equal(reference, points);
			}


			test( PathIsNotCompleteIfLastPointRemovedWasTheThirdInThePath )
			{
				// INIT
				stroke s;

				move_to(s, 2.0f, 5.0f);
				line_to(s, 0.3f, 19.2f);

				// ACT
				line_to(s, 2.0f + 0.3f * distance_epsilon, 5.0f + 0.4f * distance_epsilon);
				end_poly(s, true);

				// ACT / ASSERT
				assert_equal(path_command_stop, vertex(s).command);
			}


			test( IncorrectClosureIsIgnored )
			{
				// INIT
				stroke s;

				// ACT
				end_poly(s, true);

				// ACT / ASSERT
				assert_equal(path_command_stop, vertex(s).command);

				// INIT
				s.set_cap(passthrough_cap(amount_alive));

				// ACT
				move_to(s, 1.0f, 0.4f);
				line_to(s, 0.0f, 0.0f);

				// ACT / ASSERT
				assert_equal(path_command_move_to, vertex(s).command);
			}


			test( RemnantsOfPreviousIteratorAreSkipped )
			{
				// INIT
				stroke s;

				s.set_cap(passthrough_cap(amount_alive));
				s.set_join(partial_passthrough_join());
				s.width(101.0f);

				move_to(s, 11.0f, 75.0f);
				line_to(s, 0.3f, 19.2f);

				vertex(s);

				// ACT
				s.remove_all();
				
				// ASSERT
				move_to(s, 2.0f, 5.0f);
				line_to(s, 0.3f, 19.2f);
				line_to(s, 8.2f, 10.0f, true);

				mocks::path::point points[] = {
					vertex(s), vertex(s), vertex(s),
					vertex(s),
				};

				mocks::path::point reference[] = {
					moveto(2.0f, 5.0f), lineto(0.3f, 19.2f), lineto(8.2f, 10.0f),
					{ 0.0f, 0.0f, path_command_end_poly | path_flag_close },
				};

				assert_equal(reference, points);
			}
		end_test_suite
	}
}
