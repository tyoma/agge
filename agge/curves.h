#pragma once

#include "types.h"

namespace agge
{
	class qbezier
	{
	public:
		qbezier(real_t xb, real_t yb, real_t xc, real_t yc, real_t xe, real_t ye, real_t step);

		void rewind(unsigned id);
		int vertex(real_t *x, real_t *y);

	private:
		real_t _xb, _yb, _xc, _yc, _xe, _ye;
		real_t _t, _step;
		int _stage;
	};


	class cbezier
	{
	public:
		cbezier(real_t xb, real_t yb, real_t xc1, real_t yc1, real_t xc2, real_t yc2, real_t xe, real_t ye, real_t step);

		void rewind(unsigned id);
		int vertex(real_t *x, real_t *y);

		static void calculate(real_t *x, real_t *y, real_t xb, real_t yb, real_t xc1, real_t yc1, real_t xc2, real_t yc2,
			real_t xe, real_t ye, real_t t);

	private:
		real_t _xb, _yb, _xc1, _yc1, _xc2, _yc2, _xe, _ye;
		real_t _t, _step;
		int _stage;
	};


	class arc
	{
	public:
		arc(real_t cx, real_t cy, real_t r, real_t start, real_t end, real_t da = 0.05f);

		void rewind(unsigned id);
		int vertex(real_t *x, real_t *y);

	private:
		real_t _a;
		int _stage;
		real_t _cx, _cy, _r, _start, _end, _step;
	};
}
