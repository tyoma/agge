#include <agge/precise_delta.h>

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

namespace agge
{
	namespace tests
	{
		begin_test_suite( APreciseDeltaTests )
			test( MaxNoLossExpMultiplication )
			{
				// INIT
				precise_delta dp(0x400000, 1);

				// ACT
				dp.multiply(1);
				dp.next();

				// ASSERT
				assert_equal(0x400000, dp.get());

				// ACT
				dp.multiply(-1);
				dp.next();

				// ASSERT
				assert_equal(-0x400000, dp.get());

				// INIT
				precise_delta dn(-0x400000, 1);

				// ACT
				dn.multiply(1);
				dn.next();

				// ASSERT
				assert_equal(-0x400000, dn.get());

				// ACT
				dn.multiply(-1);
				dn.next();

				// ASSERT
				assert_equal(0x400000, dn.get());
			}


			test( MaxNoLossExpMultiplicationByMaxK )
			{
				// INIT
				precise_delta dp(0x400000, 1);

				// ACT
				dp.multiply(256);
				dp.next();

				// ASSERT
				assert_equal(0x40000000, dp.get());

				// ACT
				dp.multiply(-256);
				dp.next();

				// ASSERT
				assert_equal(-0x40000000, dp.get());

				// INIT
				precise_delta dn(-0x400000, 1);

				// ACT
				dn.multiply(256);
				dn.next();

				// ASSERT
				assert_equal(-0x40000000, dn.get());

				// ACT
				dn.multiply(-256);
				dn.next();

				// ASSERT
				assert_equal(0x40000000, dn.get());
			}


			test( MinNoLossExpMultiplication )
			{
				// INIT
				precise_delta dp(1, 256);

				// ACT
				dp.multiply(256);
				dp.next();

				// ASSERT
				assert_equal(1, dp.get());

				// ACT
				dp.multiply(-256);
				dp.next();

				// ASSERT
				assert_equal(-1, dp.get());

				// INIT
				precise_delta dn(-1, 256);

				// ACT
				dn.multiply(256);
				dn.next();

				// ASSERT
				assert_equal(-1, dn.get());

				// ACT
				dn.multiply(-256);
				dn.next();

				// ASSERT
				assert_equal(1, dn.get());
			}


			test( ExtremeMinRatioSupported1 )
			{
				// INIT
				precise_delta d(1, 0x1000000);
				int sum = 0;

				// ACT
				d.multiply(137);
				for (int i = 0x1000000; i; --i)
				{
					d.next();
					sum += d.get();
				}

				// ASSERT
				assert_equal(137, sum);

				// INIT
				sum = 0;

				// ACT
				d.multiply(-19);
				for (int i = 0x1000000; i; --i)
				{
					d.next();
					sum += d.get();
				}

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
				{
					d.next();
					sum += d.get();
				}

				// ASSERT
				assert_equal(1, sum);

				// INIT
				sum = 0;

				// ACT
				d.multiply(-256);
				for (int i = 0x400000; i; --i)
				{
					d.next();
					sum += d.get();
				}

				// ASSERT
				assert_equal(-1, sum);
			}


			test( ExtremeMaxRatioSupported )
			{
				// INIT
				precise_delta d(0x1000000, 1);

				// ACT
				d.multiply(0x40);
				d.next();

				// ACT / ASSERT
				assert_equal(0x40000000, d.get());

				// ACT
				d.multiply(-0x40);
				d.next();

				// ACT / ASSERT
				assert_equal(-0x40000000, d.get());
			}

		end_test_suite
	}
}
