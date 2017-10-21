#pragma once

#include "types.h"

namespace agge
{
	class qbezier
	{
	public:
		class iterator;

	public:
		qbezier(real_t xb, real_t yb, real_t xc, real_t yc, real_t xe, real_t ye, real_t approximation = 0.1f);

		iterator iterate() const;

		real_t approximate_length() const;

	private:
		real_t _xb, _yb, _xc, _yc, _xe, _ye/*, _approximation*/;
	};

	class qbezier::iterator
	{
	public:
		iterator(real_t xb, real_t yb, real_t xc, real_t yc, real_t xe, real_t ye, real_t step);

		int vertex(real_t *x, real_t *y);

	private:
		real_t _xb, _yb, _xc, _yc, _xe, _ye;
		real_t _t, _step;
		int _stage;
	};


	class cbezier
	{
	public:
		class iterator;
	};

	class cbezier::iterator
	{
	public:
		iterator(real_t xb, real_t yb, real_t xc1, real_t yc1, real_t xc2, real_t yc2, real_t xe, real_t ye, real_t step);

		int vertex(real_t *x, real_t *y);

	private:
		real_t _xb, _yb, _xc1, _yc1, _xc2, _yc2, _xe, _ye;
		real_t _t, _step;
		int _stage;
	};
}
