#include <agge/dash.h>

#include <agge/path.h>

namespace agge
{
	dash::dash()
		: _state(initial)
	{
	}

	void dash::reset()
	{	}

	void dash::add_dash(real_t dash_length, real_t gap_length)
	{	_dashes.dash_length = dash_length, _dashes.gap_length = gap_length;	}

	void dash::remove_all()
	{
		vertex_sequence::clear();
		_state = initial;
	}

	int dash::vertex(real_t *x, real_t *y)
	{
		switch (_state)
		{
		case initial:
			_i = begin();
			_state = move;

		case move:
			_remainder = _dashes.dash_length;
			_previous = _i->point;
			*x = _previous.x, *y = _previous.y;
			++_i;
			_state = generate;
			return path_command_move_to;

		case generate:
			_remainder -= (_i - 1)->distance;
			if (_remainder < 0.0f)
			{
				point_r limit = _i->point + (_remainder / (_i - 1)->distance) * (_i->point - _previous);
				*x = limit.x, *y = limit.y;
			}
			else
			{
				*x = _i->point.x, *y = _i->point.y;
			}
			if (++_i == end())
				_state = complete;
			return path_command_line_to;

		case complete:
			return path_command_stop;
		}
		return path_command_stop;
	}
}
