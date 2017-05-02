#pragma once

#include "types.h"

namespace agge
{
	struct order_rgba { enum { R = 0, G = 1, B = 2, A = 3 }; };
	struct order_bgra { enum { B = 0, G = 1, R = 2, A = 3 }; };
	struct order_argb { enum { A = 0, R = 1, G = 2, B = 3 }; };
	struct order_abgr { enum { A = 0, B = 1, G = 2, R = 3 }; };

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
