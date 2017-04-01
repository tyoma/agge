#include <agge/filling_rules.h>

#include <ut/assert.h>
#include <ut/test.h>

namespace agge
{
	namespace tests
	{
		begin_test_suite( FillingRulesTests )
			test( WindingRuleReturnInRangeValuesAsIs )
			{
				// INIT
				winding<0> r1;
				winding<3> r2;

				// ACT / ASSERT
				assert_equal(0, r1(0 * 2));
				assert_equal(1, r1(1 * 2));
				assert_equal(13, r1(13 * 2));
				assert_equal(255, r1(255 * 2));
				assert_equal(0, r2(0 * 2 * 8));
				assert_equal(1, r2(1 * 2 * 8));
				assert_equal(17, r2(17 * 2 * 8));
				assert_equal(254, r2(254 * 2 * 8));
			}


			test( WindingRuleDefaultedToVectorRasterizerShift )
			{
				// INIT
				winding<> r;

				// ACT / ASSERT
				assert_equal(1, r(1 << (1 + vector_rasterizer::_1_shift)));
				assert_equal(13, r(13 << (1 + vector_rasterizer::_1_shift)));
				assert_equal(255, r(255 << (1 + vector_rasterizer::_1_shift)));
			}


			test( WindingRuleClamps )
			{
				// INIT
				winding<0> r1;
				winding<4> r2;

				// ACT / ASSERT
				assert_equal(255, r1(256 << 1));
				assert_equal(255, r1(51123 << 1));
				assert_equal(255, r2(256 << 5));
				assert_equal(255, r2(1313 << 5));
			}


			test( NegativeValuesAreInverted )
			{
				// INIT
				winding<> r;

				// ACT / ASSERT
				assert_equal(25, r(-25 << (1 + vector_rasterizer::_1_shift)));
				assert_equal(255, r(-255 << (1 + vector_rasterizer::_1_shift)));
			}


			test( NegativeValuesAreClamped )
			{
				// INIT
				winding<1> r1;
				winding<5> r2;

				// ACT / ASSERT
				assert_equal(255, r1(-2555 << (1 + 1)));
				assert_equal(255, r1(-256 << (1 + 1)));
				assert_equal(255, r2(-2555 << (1 + 5)));
				assert_equal(255, r2(-256 << (1 + 5)));
			}
		end_test_suite
	}
}
