#pragma once

#include "types.h"

namespace agge
{
	class line
	{
	public:
		line(real_t x1, real_t y1, real_t x2, real_t y2);

		void rewind(unsigned id);
		int vertex(real_t *x, real_t *y);

	private:
		point_r _points[2];
		unsigned _step;
	};


	class rectangle
	{
	public:
		rectangle(real_t x1, real_t y1, real_t x2, real_t y2);

		void rewind(unsigned id);
		int vertex(real_t *x, real_t *y);

	private:
		point_r _points[2];
		unsigned _step;
	};
}
