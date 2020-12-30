#pragma once

#include "annotated_string.h"
#include "types.h"

namespace agge
{
	struct style_annotation
	{
		std::wstring font_families;
		int height : 20;
		font_weight weight : 5;
		unsigned italic : 1;
	};

	typedef annotated_string<wchar_t, style_annotation> richtext_t;
}
