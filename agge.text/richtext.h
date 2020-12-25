#pragma once

#include "annotated_string.h"

namespace agge
{
	struct range_modifier
	{
	};

	typedef annotated_string<wchar_t, range_modifier> richtext_t;
}
