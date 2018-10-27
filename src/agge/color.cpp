#include <agge/color.h>

namespace agge
{
	color color::make(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		color c = { r, g, b, a };
		return c;
	}
}
