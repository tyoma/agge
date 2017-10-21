#include <agge/curves.h>

#include <agge/math.h>
#include <agge/path.h>

namespace agge
{
	qbezier::qbezier(real_t xb, real_t yb, real_t xc, real_t yc, real_t xe, real_t ye, real_t /*approximation*/)
		: _xb(xb), _yb(yb), _xc(xc), _yc(yc), _xe(xe), _ye(ye)
	{	}

	qbezier::iterator qbezier::iterate() const
	{
		return iterator(_xb, _yb, 0.0f, 0.0f, _xe, _ye, 1.0f);
	}

	real_t qbezier::approximate_length() const
	{
		return 0.5f * (distance(_xb, _yb, _xe, _ye) + distance(_xb, _yb, _xc, _yc) + distance(_xc, _yc, _xe, _ye));
	}

	qbezier::iterator::iterator(real_t xb, real_t yb, real_t xc, real_t yc, real_t xe, real_t ye, real_t step)
		: _xb(xb), _yb(yb), _xc(xc), _yc(yc), _xe(xe), _ye(ye), _t(0.0f), _stage(path_command_move_to), _step(step)
	{	}

	int qbezier::iterator::vertex(real_t *x, real_t *y)
	{
		int stage = _stage;

		switch (stage)
		{
		case path_command_move_to:
			*x = _xb, *y = _yb;
			_stage = path_command_line_to;
			_t = _step;
			break;

		case path_command_line_to:
			if (_t < 1.0f)
			{
				const real_t _1_t = 1 - _t;

				*x = _xb * _1_t * _1_t + 2 * _1_t * _t * _xc + _xe * _t * _t;
				*y = _yb * _1_t * _1_t + 2 * _1_t * _t * _yc + _ye * _t * _t;
				_t += _step;
			}
			else
			{
				*x = _xe, *y = _ye;
				_stage = path_command_stop;
			}
			break;
		}
		return stage;
	}


	cbezier::iterator::iterator(real_t xb, real_t yb, real_t xc1, real_t yc1, real_t xc2, real_t yc2, real_t xe, real_t ye, real_t step)
		: _xb(xb), _yb(yb), _xc1(xc1), _yc1(yc1), _xc2(xc2), _yc2(yc2), _xe(xe), _ye(ye), _t(0.0f), _stage(path_command_move_to), _step(step)
	{	}

	int cbezier::iterator::vertex(real_t *x, real_t *y)
	{
		int stage = _stage;

		switch (stage)
		{
		case path_command_move_to:
			*x = _xb, *y = _yb;
			_stage = path_command_line_to;
			_t = _step;
			break;

		case path_command_line_to:
			if (_t < 1.0f)
			{
				const real_t _1_t = 1 - _t;

				*x = _xb * _1_t * _1_t * _1_t + 3 * _1_t * _1_t * _t * _xc1 + 3 * _1_t * _t * _t * _xc2 + _xe * _t * _t * _t;
				*y = _yb * _1_t * _1_t * _1_t + 3 * _1_t * _1_t * _t * _yc1 + 3 * _1_t * _t * _t * _yc2 + _ye * _t * _t * _t;
				_t += _step;
			}
			else
			{
				*x = _xe, *y = _ye;
				_stage = path_command_stop;
			}
			break;
		}
		return stage;
	}
}
