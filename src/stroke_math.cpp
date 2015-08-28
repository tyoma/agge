#include <agge/stroke_math.h>

#include <math.h>

namespace agge
{
	real_t distance(real_t x1, real_t y1, real_t x2, real_t y2)
	{
		x2 -= x1;
		y2 -= y1;

		return sqrtf(x2 * x2 + y2 * y2);
	}
}
