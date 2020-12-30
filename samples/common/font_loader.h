#pragma once

#include "services.h"

#include <agge.text/text_engine.h>

class font_loader : public agge::text_engine_base::loader, agge::noncopyable
{
public:
	font_loader(services &s);

	virtual agge::font::accessor_ptr load(const agge::font_descriptor &descriptor);

private:
	services &_services;
};

class native_font_loader : public agge::text_engine_base::loader
{
	virtual agge::font::accessor_ptr load(const agge::font_descriptor &descriptor);
};
