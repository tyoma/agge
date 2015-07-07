#pragma once

namespace agge
{
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;

#pragma pack(push, 1)
	struct pixel32
	{
		uint8_t c0, c1, c2, c3;
	};
#pragma pack(pop)
}
