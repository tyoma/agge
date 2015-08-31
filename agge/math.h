#pragma once

#include "types.h"

namespace agge
{
	real_t sqrt(real_t x);
	
	inline real_t distance(real_t ax, real_t ay, real_t bx, real_t by)
	{
		bx -= ax;
		by -= ay;
		return sqrt(bx * bx + by * by);
	}
}
