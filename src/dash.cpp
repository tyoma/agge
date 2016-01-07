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
		vertex_sequence::const_iterator previous;
		point_r m;

		switch (_state)
		{
		case complete:
			return path_command_stop;

		case initial:
			_i = begin();
			_state = move;

		case move:
			_dash = &_dashes;
			_remainder = _dash->dash_length;
			*x = _i->point.x, *y = _i->point.y;
			if (_i->distance > _dash->dash_length)
			{
				_remainder = _i->distance;
				_state = emit_dash;
			}
			else
			{
				_remainder = _dash->dash_length;
				_state = emit_source;
			}
			++_i;
			return path_command_move_to;

		case emit_source:
			*x = _i->point.x, *y = _i->point.y;
//			_state = _i->distance > _remainder ? emit_dashes : emit_source;
			if (++_i == end())
				_state = complete;
			return path_command_line_to;

		case move_dash:
			previous = _i - 1;
			m = previous->point + ((previous->distance - _remainder + _dash->gap_length) / previous->distance) * (_i->point - previous->point);

			*x = m.x, *y = m.y;
			_remainder -= _dash->gap_length;
			if (_remainder > _dash->dash_length)
				_state = emit_dash;
			else if (++_i == end())
				_state = complete;
			return path_command_move_to;

		case emit_dash:
			previous = _i - 1;
			m = previous->point + ((previous->distance - _remainder + _dash->dash_length) / previous->distance) * (_i->point - previous->point);

			*x = m.x, *y = m.y;
			_remainder -= _dash->dash_length;
			if (_remainder > _dash->gap_length)
				_state = move_dash;
			else if (++_i == end())
				_state = complete;
			return path_command_line_to;
		}
		return path_command_stop;
	}
}
