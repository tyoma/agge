#pragma once

#include <memory>

class BarsModel;

class BarView
{
	std::shared_ptr<BarsModel> _model;

public:
	BarView();
	~BarView();

	void SetModel(std::shared_ptr<BarsModel> model);
	void Render(CDC &dc, const CSize &size);
};
