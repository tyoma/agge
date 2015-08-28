#pragma once

#include "types.h"

namespace agge
{
	real_t distance(real_t x1, real_t y1, real_t x2, real_t y2);
	
	inline real_t distance(const point_r &lhs, const point_r &rhs)
	{	return distance(lhs.x, lhs.y, rhs.x, rhs.y);	}
}
