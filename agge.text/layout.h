#pragma once

#include <agge/types.h>

namespace agge
{
	class font;
	class vector_rasterizer;

	class layout
	{
	public:
		layout(const char *text, const font *f);

		void limit_width(real_t width);

		void render(vector_rasterizer &rasterizer);
	};
}
