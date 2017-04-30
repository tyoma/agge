#pragma once

#include "types.h"

namespace agge
{
	enum bits_per_pixel { bpp32 = 32, bpp24 = 24, bpp16 = 16, bpp8 = 8 };

#pragma pack(push, 1)
	struct pixel32
	{
		uint8_t components[4];
	};

	struct pixel24
	{
		uint8_t components[3];
	};

	struct pixel16
	{
		uint16_t packed_components;
	};
#pragma pack(pop)
}
