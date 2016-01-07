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
		vertex_sequence::const_iterator i;
		point_r m;

		switch (_state)
		{
		case initial:
			_j = begin();
			_dash = &_dashes;
			_t1 = 0.0f;
			_t2 = 0.0f;
			_state = seek;

		case seek:
			do
			{
				_t1 += _j->distance;
				if (++_j == end())
				{
					_state = complete;
					return path_command_stop;
				}
			}	while (_t1 < _t2);
			_state = move;

		case move:
			i = _j - 1;
			m = _j->point + ((_t2 - _t1) / i->distance) * (_j->point - i->point);
			*x = m.x, *y = m.y;
			_t2 += _dash->dash_length;
			_state = _t2 > _t1 ? emit_source : finish_dash;
			return path_command_move_to;

		case emit_source:
			*x = _j->point.x, *y = _j->point.y;
			_t1 += _j->distance;
			if (++_j == end())
				_state = complete;
			else if (_t1 > _t2)
				_state = finish_dash;
			return path_command_line_to;

		case finish_dash:
			i = _j - 1;
			m = _j->point + ((_t2 - _t1) / i->distance) * (_j->point - i->point);
			*x = m.x, *y = m.y;
			_t2 += _dash->gap_length;
			_state = _t2 >= _t1 ? seek : move;
			return path_command_line_to;

		case complete:
			return path_command_stop;
		}
		return path_command_stop;
	}
}
