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

	inline void is_on_circle(double cx, double cy, double r, double x, double y, const LocationInfo &location)
	{
		const double tolerance = std::pow(r, -4);
		const double err = std::sqrt((x -= cx, x * x) + (y -= cy, y * y)) - r;

		if (err < -tolerance || tolerance < err)
			throw FailedAssertion("The point is not on the circle!", location);
	}
}

#define assert_equal_approx(_mp_lhs, _mp_rhs, _mp_order)  ut::are_equal_approx((_mp_lhs), (_mp_rhs), (_mp_order), ut::LocationInfo(__FILE__, __LINE__))
#define assert_on_circle(_mp_cx, _mp_cy, _mp_r, _mp_x, _mp_y)  ut::is_on_circle((_mp_cx), (_mp_cy), (_mp_r), (_mp_x), (_mp_y), ut::LocationInfo(__FILE__, __LINE__))

