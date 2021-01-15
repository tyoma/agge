#pragma once

#include <agge/types.h>
#include <math.h>
#include <ut/assert.h>

namespace ut
{
	const double agge_tests_tolerance = 0.001;

	inline void is_on_circle(double cx, double cy, double r, double x, double y, const LocationInfo &location)
	{	ut::are_approx_equal(sqrt((x -= cx, x * x) + (y -= cy, y * y)), r, agge_tests_tolerance, location);	}

	template <typename ContainerT>
	inline void are_on_circle(double cx, double cy, double r, const ContainerT &points, const LocationInfo &location)
	{
		for (typename ContainerT::const_iterator i = points.begin(); i != points.end(); ++i)
			is_on_circle(cx, cy, r, i->x, i->y, location);
	}
}

#define assert_point_on_circle(_mp_cx, _mp_cy, _mp_r, _mp_x, _mp_y)  ut::is_on_circle((_mp_cx), (_mp_cy), (_mp_r), (_mp_x), (_mp_y), ut::LocationInfo(__FILE__, __LINE__))
#define assert_points_on_circle(_mp_cx, _mp_cy, _mp_r, _mp_points)  ut::are_on_circle((_mp_cx), (_mp_cy), (_mp_r), (_mp_points), ut::LocationInfo(__FILE__, __LINE__))
