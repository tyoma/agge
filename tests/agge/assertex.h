#pragma once

#include <cmath>
#include <ut/assert.h>

namespace ut
{
	inline void are_equal_approx(double lhs, double rhs, int order, const LocationInfo &location)
	{
		const double tolerance = std::pow(0.1, order);
		const double err = 2 * (lhs - rhs) / (lhs + rhs);

		if (err < -tolerance || tolerance < err)
			throw FailedAssertion("Values are not equal approximately!", location);
	}
}

#define assert_equal_approx(_mp_lhs, _mp_rhs, _mp_order)  ut::are_equal_approx((_mp_lhs), (_mp_rhs), (_mp_order), ut::LocationInfo(__FILE__, __LINE__))
