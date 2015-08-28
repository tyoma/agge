#include "helpers.h"

#include <math.h>

namespace agge
{
	namespace tests
	{
		template <>
		bool equal(const float &lhs, const float &rhs)
		{	return lhs == rhs || fabsf((lhs - rhs) / (lhs + rhs)) <= 2e-6f;	}
	}
}
