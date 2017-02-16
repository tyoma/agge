#pragma once

#include <agge/types.h>

namespace common
{
	struct rgba8
	{
		typedef agge::uint8_t value_type;

		value_type r, g, b, a;

		rgba8(unsigned r_, unsigned g_, unsigned b_, unsigned a_ = static_cast<value_type>(-1))
			: r(value_type(r_)),  g(value_type(g_)),  b(value_type(b_)),  a(value_type(a_))
		{	}
	};
}
