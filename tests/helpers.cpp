#include "helpers.h"

#include <math.h>

namespace agge
{
	namespace tests
	{
		template <>
		bool equal(const real_t &lhs, const real_t &rhs)
		{	return lhs == rhs || fabs((lhs - rhs) / (lhs + rhs)) <= 2e-6f;	}
	}
}
