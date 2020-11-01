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


	class rounded_rectangle
	{
	public:
		rounded_rectangle(real_t x1, real_t y1, real_t x2, real_t y2, real_t rx, real_t ry = 0.0f);

		void rewind(unsigned id);
		int vertex(real_t *x, real_t *y);

	private:
		enum { initial = 1, left = 0, right = 2, top = 0, bottom = 4, close = 8, complete = 16 };

	private:
		void setup_bezier(unsigned state);

	private:
		real_t _t, _step;
		real_t _xb, _yb, _xc1, _yc1, _xc2, _yc2, _xe, _ye;
		real_t _rx, _ry;
		point_r _points[2];
		unsigned _state;
	};
}
