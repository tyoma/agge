#include <agge/stroker.h>

#include <agge/math.h>
#include <agge/stroke_features.h>

namespace agge
{
	struct stroke::point_ref
	{
		point_r point;
		real_t distance;
	};

	stroke::stroke()
		: _cap(new caps::butt), _output_iterator(_output.end()), _state(0)
	{	}

	stroke::~stroke()
	{	delete _cap;	}

	void stroke::remove_all()
	{
		_input.clear();
		_output.clear();
		_output_iterator = _output.end();
		_state = start;
	}

	void stroke::add_vertex(real_t x, real_t y, int command)
	{
		const bool close = command == path_command_end_poly;

		if (!_input.empty())
		{
			point_ref &last = *(_input.end() - 1);

			last.distance = close ? distance(last.point.x, last.point.y, _input.begin()->point.x, _input.begin()->point.y)
				: distance(last.point.x, last.point.y, x, y);
		}

		if (!close)
		{
			point_ref p = { create_point(x, y), 0 };

			_input.push_back(p);
		}
		else
			_state |= closed;
	}
		
	int stroke::vertex(real_t *x, real_t *y)
	{
		for (; _input.size() > (is_closed() ? 2u : 1u); )
		{
			if (_output_iterator != _output.end())
			{
				*x = _output_iterator->x;
				*y = _output_iterator->y;
				++_output_iterator;
				int command = _state & moveto ? path_command_move_to : path_command_line_to;
				_state &= ~moveto;
				return command;
			}

			_output.clear();

			const input_vertices::const_iterator first = _input.begin();
			const input_vertices::const_iterator last = _input.end() - 1;
			const input_vertices::const_iterator prev = _i == first ? last : _i - 1;
			const input_vertices::const_iterator next = _i == last ? first : _i + 1;

			switch (_state & stage_mask)
			{
			case start:
				_i = first;
				_state |= moveto;
				set_stage(is_closed() ? outline_forward : start_cap);
				break;

			case start_cap:
				_cap->calc(_output, _width, _i->point, _i->distance, next->point);
				_i = next;
				set_stage(_i == last ? end_cap : outline_forward);
				break;

			case outline_forward:
				_join->calc(_output, _width, prev->point, prev->distance, _i->point, _i->distance, next->point);
				_i = next;
				if (is_closed() && _i == first)
					set_stage(end_poly1);
				else if (!is_closed() && _i == last)
					set_stage(end_cap);
				break;

			case end_poly1:
				_output_iterator = _output.end();
				_state |= moveto;
				set_stage(outline_backward);
				return path_command_end_poly;

			case end_cap:
				_cap->calc(_output, _width, _i->point, prev->distance, prev->point);
				_i = prev;
				set_stage(_i == first ? end_poly : outline_backward);
				break;

			case outline_backward:
				_join->calc(_output, _width, next->point, _i->distance, _i->point, prev->distance, prev->point);
				_i = prev;
				if (_i == first)
					set_stage(end_poly);
				break;

			case end_poly:
				_output_iterator = _output.end();
				set_stage(stop);
				return path_command_end_poly;

			case stop:
				return path_command_stop;
			}

			_output_iterator = _output.begin();
		}
		return path_command_stop;
	}
		
	void stroke::width(real_t w)
	{	_width = 0.5f * w;	}

	bool stroke::is_closed() const
	{	return 0 != (_state & closed);	}

	void stroke::set_stage(state stage)
	{	_state = (_state & ~stage_mask) | stage;	}
}
