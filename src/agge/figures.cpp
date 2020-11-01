#include <agge/figures.h>

#include <agge/curves.h>
#include <agge/math.h>
#include <agge/path.h>

namespace agge
{
	line::line(real_t x1, real_t y1, real_t x2, real_t y2)
		: _step(0)
	{
		_points[0].x = x1, _points[0].y = y1;
		_points[1].x = x2, _points[1].y = y2;
	}

	void line::rewind(unsigned /*id*/)
	{	_step = 0;	}

	int line::vertex(real_t *x, real_t *y)
	{
		int step = _step;

		switch (_step)
		{
		default:
			*x = _points[step].x, *y = _points[step].y;
			return _step = step + 1, step == 0 ? path_command_move_to : path_command_line_to;

		case 2:
			return _step = 3, path_command_end_poly;

		case 3:
			return path_command_stop;
		}
	}


	rectangle::rectangle(real_t x1, real_t y1, real_t x2, real_t y2)
		: _step(0)
	{
		_points[0].x = x1, _points[0].y = y1;
		_points[1].x = x2, _points[1].y = y2;
	}

	void rectangle::rewind(unsigned /*id*/)
	{	_step = 0;	}

	int rectangle::vertex(real_t *x, real_t *y)
	{
		int step = _step;

		switch (step)
		{
		default:
			*x = _points[(step + 1) / 2 & 1].x, *y = _points[step / 2].y;
			return _step = step + 1, step == 0 ? path_command_move_to : path_command_line_to;

		case 4:
			return _step = 5, path_command_end_poly | path_flag_close;

		case 5:
			return path_command_stop;
		}
	}


	rounded_rectangle::rounded_rectangle(real_t x1, real_t y1, real_t x2, real_t y2, real_t rx, real_t /*ry*/)
		: _step(4.0f * optimal_circle_stepping(rx)), _rx(rx), _ry(rx), _state(initial)
	{
		_points[0].x = x1, _points[0].y = y1;
		_points[1].x = x2, _points[1].y = y2;
	}

	void rounded_rectangle::rewind(unsigned /*id*/)
	{	_state = initial;	}

	int rounded_rectangle::vertex(real_t *x, real_t *y)
	{
		int cmd = path_command_line_to;

		switch (_state)
		{
		case close:
			_state = complete;
			return path_command_end_poly | path_flag_close;

		case complete:
			return path_command_stop;

		case initial:
			setup_bezier(_state = left | top), cmd = path_command_move_to;
		}

		cbezier::calculate(x, y, _xb, _yb, _xc1, _yc1, _xc2, _yc2, _xe, _ye, _t);
		_t += _step;
		if (_t > 1.0f)
		{
			switch (_state)
			{
			case left | top:
				_state = right | top;
				break;

			case right | top:
				_state = right | bottom;
				break;

			case right | bottom:
				_state = left | bottom;
				break;

			case left | bottom:
				_state = close;
				break;
			}
			setup_bezier(_state);
		}
		return cmd;
	}

	void rounded_rectangle::setup_bezier(unsigned state)
	{
		const real_t k_ = 1.0f - c_qarc_bezier_k;

		switch (state)
		{
		case left | top:
			_xb = _points[0].x, _yb = _points[0].y + _ry;
			_xc1 = _points[0].x, _yc1 = _points[0].y + k_ * _ry;
			_xc2 = _points[0].x + k_ * _rx, _yc2 = _points[0].y;
			_xe = _points[0].x + _rx, _ye = _points[0].y;
			break;

		case right | top:
			_xb = _points[1].x - _rx, _yb = _points[0].y;
			_xc1 = _points[1].x - k_ * _rx, _yc1 = _points[0].y;
			_xc2 = _points[1].x, _yc2 = _points[0].y + k_ * _ry;
			_xe = _points[1].x, _ye = _points[0].y + _rx;
			break;

		case right | bottom:
			_xb = _points[1].x, _yb = _points[1].y - _ry;
			_xc1 = _points[1].x, _yc1 = _points[1].y - k_ * _ry;
			_xc2 = _points[1].x - k_ * _rx, _yc2 = _points[1].y;
			_xe = _points[1].x - _rx, _ye = _points[1].y;
			break;

		case left | bottom:
			_xb = _points[0].x + _rx, _yb = _points[1].y;
			_xc1 = _points[0].x + k_ * _rx, _yc1 = _points[1].y;
			_xc2 = _points[0].x, _yc2 = _points[1].y - k_ * _ry;
			_xe = _points[0].x, _ye = _points[1].y - _ry;
			break;
		}
		_t = 0.0f;
	}
}
