#pragma once

#include <agge/platform/win32/bitmap.h>

namespace aggx
{
	class bitmap : public agge::platform::raw_bitmap
	{
	public:
		typedef agge::pixel32 pixel;

	public:
		bitmap(unsigned w, unsigned h)
			: raw_bitmap(w, h, agge::bpp32)
		{	}

		pixel *row_ptr(unsigned y)
		{	return static_cast<pixel *>(raw_bitmap::row_ptr(y));	}
	};
}
