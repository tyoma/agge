#include <agge/stroke_features.h>

#include "helpers.h"

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

namespace agge
{
	namespace joins
	{
		namespace tests
		{
			namespace
			{
				inline real_t distance(const point_r &lhs, const point_r &rhs)
				{	return agge::distance(lhs.x, lhs.y, rhs.x, rhs.y);	}
			}

			begin_test_suite( BevelJoinTests )
				test( OuterJoinIsBeveled )
				{
					// INIT
					const point_r seq1[] = {
						{ 3.1f, 3.1f },
						{ 7.1f, 2.3f },
						{ 17.2f, 3.1f },
					};
					const point_r seq2[] = {
						{ 3.1f, 3.1f },
						{ 7.1f, 9.3f },
						{ 3.7f, 16.5f },
					};
					points output;
					bevel j;

					// ACT
					j.calc(output, 1.0f, seq1[0], distance(seq1[0], seq1[1]), seq1[1], distance(seq1[1], seq1[2]), seq1[2]);

					// ASSERT
					point_r reference1[] = {
						{ 6.90388f, 1.31942f },
						{ 7.17896f, 1.30312f },
					};

					assert_equal(reference1, output);

					// ACT
					j.calc(output, 1.75f, seq1[0], distance(seq1[0], seq1[1]), seq1[1], distance(seq1[1], seq1[2]), seq1[2]);

					// ASSERT
					point_r reference2[] = {
						{ 6.90388f, 1.31942f },
						{ 7.17896f, 1.30312f },
						{ 6.75680f, 0.583984f },
						{ 7.23818f, 0.555464f },
					};

					assert_equal(reference2, output);

					// INIT
					output.clear();

					// ACT
					j.calc(output, 1.75f, seq2[0], distance(seq2[0], seq2[1]), seq2[1], distance(seq2[1], seq2[2]), seq2[2]);

					// ASSERT
					point_r reference3[] = {
						{ 8.57052f, 8.35128f },
						{ 8.68244f, 10.0473f },
					};

					assert_equal(reference3, output);
				}


				test( InnerJoinIsBeveled )
				{
					// INIT
					const point_r seq[] = {
						{ 3.1f, 3.3f },
						{ 7.1f, 6.3f },
						{ 17.2f, 3.0f },
					};
					points output;
					bevel j;

					// ACT
					j.calc(output, 1.5f, seq[0], distance(seq[0], seq[1]), seq[1], distance(seq[1], seq[2]), seq[2]);

					// ASSERT
					point_r reference[] = {
						{ 8.00000f, 5.10000f },
						{ 6.63414f, 4.87418f },
					};

					assert_equal(reference, output);
				}
			end_test_suite


			begin_test_suite( MiterJoinTests )
				test( PointsSharingTheSameLineDoNotProduceOutput )
				{
					// INIT
					const point_r seq1[] = {
						{ 3.1f, 2.9f },
						{ 7.1f, 2.9f },
						{ 7.2f, 2.9f },
					};
					const point_r seq2[] = {
						{ 3.7f, 2.3f },
						{ 3.7f, 2.5f },
						{ 3.7f, 5.3f },
					};
					const point_r seq3[] = {
						{ 5.6f, 3.7f },
						{ 2.0f, 1.0f },
						{ 0.8f, 0.1f },
					};
					points output;
					miter j;

					// ACT
					j.calc(output, 1.5f, seq1[0], 4.0f, seq1[1], 0.1f, seq1[2]);

					j.calc(output, 6.0f, seq1[0], 4.0f, seq1[1], 0.1f, seq1[2]);
					j.calc(output, 6.0f, seq1[2], 0.1f, seq1[1], 4.0f, seq1[0]);
					j.calc(output, 6.0f, seq2[0], 0.1f, seq2[1], 5.0f, seq2[2]);
					j.calc(output, 6.0f, seq3[0], 0.1f, seq3[1], 1.0f, seq3[2]);

					// ASSERT
					assert_is_empty(output);
				}
			end_test_suite
		}
	}
}
