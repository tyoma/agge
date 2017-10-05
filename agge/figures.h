#pragma once

#include "types.h"

namespace agge
{
	class rectangle
	{
	public:
		class iterator;

	public:
		rectangle(real_t x1, real_t y1, real_t x2, real_t y2);

		iterator iterate() const;

	private:
		rect<real_t> _rect;
	};

	class rectangle::iterator
	{
	public:
		iterator(const rect<real_t> &r);

		int vertex(real_t *x, real_t *y);

	private:
		rect<real_t> _rect;
		unsigned _step;
	};
}
