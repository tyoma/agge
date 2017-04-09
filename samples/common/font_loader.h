#pragma once

#include <agge.text/font_engine.h>

class font_loader : public agge::font_engine_base::loader
{
	virtual agge::font::accessor_ptr load(const wchar_t *typeface, int height, bool bold, bool italic,
		agge::font_engine_base::grid_fit grid_fit);
};
