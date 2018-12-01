#pragma once

#include <cmath>
#include <ut/assert.h>

namespace ut
{
	inline void is_on_circle(double cx, double cy, double r, double x, double y, const LocationInfo &location)
	{	ut::are_approx_equal(std::sqrt((x -= cx, x * x) + (y -= cy, y * y)), r, 0.0001, location);	}
}

#define assert_on_circle(_mp_cx, _mp_cy, _mp_r, _mp_x, _mp_y)  ut::is_on_circle((_mp_cx), (_mp_cy), (_mp_r), (_mp_x), (_mp_y), ut::LocationInfo(__FILE__, __LINE__))
