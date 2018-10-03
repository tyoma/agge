#include <agge/stroke_features.h>

#include "helpers.h"

#include <tests/common/helpers.h>
#include <ut/assert.h>
#include <ut/test.h>

namespace agge
{
	namespace joins
	{
		namespace tests
		{
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
		}
	}

	namespace caps
	{
		namespace tests
		{
			begin_test_suite( TriangleCapTests )
				test( ThreePointsAreProducedByCap )
				{
					// INIT
					const point_r seq1[] = { { 3.1f, 3.3f }, { 7.1f, 6.3f }, };
					const point_r seq2[] = { { 13.1f, -1.3f }, { 2.1f, 1.3f }, };
					points output;
					triangle c;

					// ACT
					c.calc(output, 1.5f, seq1[0], distance(seq1[0], seq1[1]), seq1[1]);

					// ASSERT
					assert_equal(3u, output.size());

					// ACT
					c.calc(output, 2.5f, seq2[0], distance(seq2[0], seq2[1]), seq2[1]);

					// ASSERT
					assert_equal(6u, output.size());
				}


				test( PointsAreCalculatedAccordinglyToWidth )
				{
					// INIT
					const point_r seq[] = { { 3.1f, 3.3f }, { 7.1f, 6.3f }, };
					points output;
					triangle c;

					// ACT
					c.calc(output, 2.0f, seq[0], distance(seq[0], seq[1]), seq[1]);

					// ASSERT
					point_r reference1[] = {
						{ 1.90000f, 4.90000f },
						{ 1.50000f, 2.10000f },
						{ 4.30000f, 1.70000f },
					};

					assert_equal(reference1, output);

					// INIT
					output.clear();

					// ACT
					c.calc(output, 3.0f, seq[0], distance(seq[0], seq[1]), seq[1]);

					// ASSERT
					point_r reference2[] = {
						{ 1.30000f, 5.70000f },
						{ 0.70000f, 1.50000f },
						{ 4.90000f, 0.90000f },
					};

					assert_equal(reference2, output);
				}


				test( TipIsProtrudedAccordinglyToParameter )
				{
					// INIT
					const point_r seq[] = { { 3.1f, 3.3f }, { 7.1f, 6.3f }, };
					points output;
					triangle c(2.5f);

					// ACT
					c.calc(output, 2.0f, seq[0], distance(seq[0], seq[1]), seq[1]);

					// ASSERT
					point_r reference[] = {
						{ 1.90000f, 4.90000f },
						{ -0.90000f, 0.30000f },
						{ 4.30000f, 1.70000f },
					};

					assert_equal(reference, output);
				}
			end_test_suite
		}
	}
}
