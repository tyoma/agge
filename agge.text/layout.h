#pragma once

#include "font.h"

#include <agge/types.h>
#include <string>

namespace agge
{
	class vector_rasterizer;

	class layout : noncopyable
	{
	public:
		layout(const wchar_t *text, font::ptr font_);

		void limit_width(real_t width);
		box_r get_box();

		void render(vector_rasterizer &rasterizer);

	private:
		std::wstring _text;
		font::ptr _font;
	};
}
