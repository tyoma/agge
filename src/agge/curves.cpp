#include <agge/curves.h>

#include <agge/math.h>
#include <agge/path.h>

namespace agge
{
	qbezier::qbezier(real_t xb, real_t yb, real_t xc, real_t yc, real_t xe, real_t ye, real_t step)
		: _xb(xb), _yb(yb), _xc(xc), _yc(yc), _xe(xe), _ye(ye), _t(0.0f), _step(step), _stage(path_command_move_to)
	{	}

	void qbezier::rewind(unsigned /*id*/)
	{
		_t = 0.0f;
		_stage = path_command_move_to;
	}

	int qbezier::vertex(real_t *x, real_t *y)
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
				const real_t _1_t = 1.0f - _t;
				const real_t c[] = { _1_t * _1_t, 2.0f * _1_t * _t, _t * _t, };

				*x = _xb * c[0] + _xc * c[1] + _xe * c[2];
				*y = _yb * c[0] + _yc * c[1] + _ye * c[2];
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


	cbezier::cbezier(real_t xb, real_t yb, real_t xc1, real_t yc1, real_t xc2, real_t yc2, real_t xe, real_t ye, real_t step)
		: _xb(xb), _yb(yb), _xc1(xc1), _yc1(yc1), _xc2(xc2), _yc2(yc2), _xe(xe), _ye(ye), _t(0.0f), _step(step),
			_stage(path_command_move_to)
	{	}

	void cbezier::rewind(unsigned /*id*/)
	{
		_t = 0.0f;
		_stage = path_command_move_to;
	}

	int cbezier::vertex(real_t *x, real_t *y)
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
				const real_t _1_t = 1.0f - _t;
				const real_t c[] = { _1_t * _1_t * _1_t, 3.0f * _1_t * _1_t * _t, 3.0f * _1_t * _t * _t, _t * _t * _t, };

				*x = _xb * c[0] + _xc1 * c[1] + _xc2 * c[2] + _xe * c[3];
				*y = _yb * c[0] + _yc1 * c[1] + _yc2 * c[2] + _ye * c[3];
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
