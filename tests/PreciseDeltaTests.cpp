#include <agge/precise_delta.h>

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

namespace agge
{
	namespace tests
	{
		begin_test_suite( APreciseDeltaTests )
			test( PositiveFractionalDeltaDoesNotOverflow )
			{
				// INIT
				precise_delta d1(98, 99 * 0x10000);
				int sum = 0;

				// ACT
				d1.multiply(256);

				// ACT / ASSERT
				for (int i = 99 * 0x10000; i; --i)
					sum += d1.next();

				// ASSERT
				assert_is_true(98 * 256 - 1 <= sum && sum <= 98 * 256);

				// INIT
				precise_delta d2(-98, 99 * 0x10000);

				sum = 0;

				// ACT
				d2.multiply(-256);

				// ACT / ASSERT
				for (int i = 99 * 0x10000; i; --i)
					sum += d2.next();

				// ASSERT
				assert_is_true(98 * 256 - 1 <= sum && sum <= 98 * 256);
			}


			test( NegativeFractionalDeltaDoesNotOverflow )
			{
				// INIT
				precise_delta d1(98, 99 * 0x10000);
				int sum = 0;

				// ACT
				d1.multiply(-256);

				// ACT / ASSERT
				for (int i = 99 * 0x10000; i; --i)
					sum += d1.next();

				// ASSERT
				assert_is_true(-98 * 256 <= sum && sum <= -98 * 256 + 1);

				// INIT
				precise_delta d2(-98, 99 * 0x10000);

				sum = 0;

				// ACT
				d2.multiply(256);

				// ACT / ASSERT
				for (int i = 99 * 0x10000; i; --i)
					sum += d2.next();

				// ASSERT
				assert_is_true(-98 * 256 <= sum && sum <= -98 * 256 + 1);

				// TODO: This test will pass when rounding mode is fixed.
				//// INIT
				//precise_delta d3(-1024, 5632 /*22 * 256*/);

				//sum = 0;

				//// ACT
				//d3.multiply(256);

				//// ACT / ASSERT
				//for (int i = 22; i; --i)
				//	sum += d3.next();

				//assert_is_true(-1024 <= sum && sum <= -1024 + 1);
			}


			test( MaxNoLossExpMultiplication )
			{
				// INIT
				precise_delta dp(0x400000, 1);

				// ACT
				dp.multiply(1);

				// ACT / ASSERT
				assert_equal(0x400000, dp.next());

				// ACT
				dp.multiply(-1);

				// ACT / ASSERT
				assert_equal(-0x400000, dp.next());

				// INIT
				precise_delta dn(-0x400000, 1);

				// ACT
				dn.multiply(1);

				// ACT / ASSERT
				assert_equal(-0x400000, dn.next());

				// ACT
				dn.multiply(-1);

				// ACT / ASSERT
				assert_equal(0x400000, dn.next());
			}


			test( MaxNoLossExpMultiplicationByMaxK )
			{
				// INIT
				precise_delta dp(0x400000, 1);

				// ACT
				dp.multiply(256);

				// ACT / ASSERT
				assert_equal(0x40000000, dp.next());

				// ACT
				dp.multiply(-256);

				// ACT / ASSERT
				assert_equal(-0x40000000, dp.next());

				// INIT
				precise_delta dn(-0x400000, 1);

				// ACT
				dn.multiply(256);

				// ACT / ASSERT
				assert_equal(-0x40000000, dn.next());

				// ACT
				dn.multiply(-256);

				// ACT / ASSERT
				assert_equal(0x40000000, dn.next());
			}


			test( MinNoLossExpMultiplication )
			{
				// INIT
				precise_delta dp(1, 256);

				// ACT
				dp.multiply(256);

				// ACT / ASSERT
				assert_equal(1, dp.next());

				// ACT
				dp.multiply(-256);

				// ACT / ASSERT
				assert_equal(-1, dp.next());

				// INIT
				precise_delta dn(-1, 256);

				// ACT
				dn.multiply(256);

				// ACT / ASSERT
				assert_equal(-1, dn.next());

				// ACT
				dn.multiply(-256);

				// ACT / ASSERT
				assert_equal(1, dn.next());
			}


			test( ExtremeMinRatioSupported1 )
			{
				// INIT
				precise_delta d(1, 0x1000000);
				int sum = 0;

				// ACT
				d.multiply(137);
				for (int i = 0x1000000; i; --i)
					sum += d.next();

				// ASSERT
				assert_equal(137, sum);

				// INIT
				sum = 0;

				// ACT
				d.multiply(-19);
				for (int i = 0x1000000; i; --i)
					sum += d.next();

				// ASSERT
				assert_equal(-19, sum);
			}


			test( ExtremeMinRatioSupported2 )
			{
				// INIT
				precise_delta d(1, 0x40000000);
				int sum = 0;

				// ACT
				d.multiply(256);
				for (int i = 0x400000; i; --i)
					sum += d.next();

				// ASSERT
				assert_equal(1, sum);

				// INIT
				sum = 0;

				// ACT
				d.multiply(-256);
				for (int i = 0x400000; i; --i)
					sum += d.next();

				// ASSERT
				assert_equal(-1, sum);
			}


			test( ExtremeMaxRatioSupported )
			{
				// INIT
				precise_delta d(0x1000000, 1);

				// ACT
				d.multiply(0x40);

				// ACT / ASSERT
				assert_equal(0x40000000, d.next());

				// ACT
				d.multiply(-0x40);

				// ACT / ASSERT
				assert_equal(-0x40000000, d.next());
			}

		end_test_suite
	}
}
