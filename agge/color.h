#pragma once

#include "types.h"

namespace agge
{
	struct color
	{
		uint8_t r, g, b, a;

		static color make(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF);
	};
}
