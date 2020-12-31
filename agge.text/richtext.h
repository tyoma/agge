#pragma once

#include "annotated_string.h"
#include "types.h"

namespace agge
{
	struct font_style_annotation
	{
		font_descriptor basic;
	};

	typedef annotated_string<wchar_t, font_style_annotation> richtext_t;
}
