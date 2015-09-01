#pragma once

#include "types.h"

namespace agge
{
	template <typename T>
	struct limits
	{
		static T resolution();
	};

	real_t sqrt(real_t x);
	
	inline real_t distance(real_t ax, real_t ay, real_t bx, real_t by)
	{
		bx -= ax;
		by -= ay;
		return sqrt(bx * bx + by * by);
	}

	template <typename CoordT>
	inline CoordT distance(const point<CoordT> &a, const point<CoordT> &b)
	{	return distance(a.x, a.y, b.x, b.y);	}


	template <>
	inline float limits<float>::resolution()
	{	return 1e-6f;	}

	template <>
	inline double limits<double>::resolution()
	{	return 1e-15;	}


	const real_t distance_epsilon = limits<real_t>::resolution();
}
