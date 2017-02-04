#include <agge/precise_delta.h>

#include <ut/assert.h>
#include <ut/test.h>

namespace ut
{
	inline void are_equal(int reference, int actual, const LocationInfo &location)
	{
		if ((reference < 0 && (actual < reference || reference + 1 < actual))
			|| (reference > 0 && (actual < reference - 1 || reference < actual)))
		{
			throw FailedAssertion("Values are not equal!", location);
		}
	}
}

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
			}


			test( DivisionResultIsRoundedByChopping )
			{
				// INIT
				precise_delta d1(-1024, 5632 /*22 * 256*/);
				int sum = 0;

				// ACT
				d1.multiply(256);

				// ACT / ASSERT
				for (int i = 2200; i; --i)
					sum += d1.next();

				assert_is_true(-102400 <= sum);

				// INIT
				precise_delta d2(1024, 5632 /*22 * 256*/);

				sum = 0;

				// ACT
				d2.multiply(256);

				// ACT / ASSERT
				for (int i = 2200; i; --i)
					sum += d2.next();

				assert_is_true(102400 >= sum);
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
				assert_equal(0x400000, dp.next() >> 8);

				// ACT
				dp.multiply(-256);

				// ACT / ASSERT
				assert_equal(-0x400000, dp.next() >> 8);

				// INIT
				precise_delta dn(-0x400000, 1);

				// ACT
				dn.multiply(256);

				// ACT / ASSERT
				assert_equal(-0x400000, dn.next() >> 8);

				// ACT
				dn.multiply(-256);

				// ACT / ASSERT
				assert_equal(0x400000, dn.next() >> 8);
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
				precise_delta d(1, 0xFFFFFF);
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
				assert_equal(0x800000, d.next() >> 7);

				// ACT
				d.multiply(-0x40);

				// ACT / ASSERT
				assert_equal(-0x800000, d.next() >> 7);
			}

		end_test_suite
	}
}
