#pragma once

#include "shared_ptr.h"

namespace agge
{
	class font;
	struct font_descriptor;

	struct font_factory
	{
		virtual ptr<font> create_font(const font_descriptor &descriptor) = 0;
	};
}
