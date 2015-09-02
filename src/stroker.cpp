#include <agge/stroker.h>

#include <agge/math.h>
#include <agge/stroke_features.h>

namespace agge
{
	struct stroke::point_ref
	{
		bool set_distance_to(const point_r &next)
		{
			distance = agge::distance(point, next);
			return distance > distance_epsilon;
		}

		point_r point;
		real_t distance;
	};

	stroke::stroke()
		: _cap(0), _join(0), _o(_output.end()), _state(0)
	{	}

	stroke::~stroke()
	{
		delete _cap;
		delete _join;
	}

	void stroke::remove_all()
	{
		_input.clear();
		_state = 0;
	}

	void stroke::add_vertex(real_t x, real_t y, int command)
	{
		if (is_vertex(command))
		{
			point_ref p = { { x, y } };

			if (_input.empty())
			{
				_input.push_back(p);
			}
			else
			{
				point_ref &last = *(_input.end() - 1);

				if (last.set_distance_to(p.point))
					_input.push_back(p);
			}
		}

		if (is_close(command))
			close();
	}
		
	int stroke::vertex(real_t *x, real_t *y)
	{
		for ( ; prepare(); )
		{
			if (_o != _output.end())
			{
				*x = _o->x;
				*y = _o->y;
				++_o;
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
			case start_cap:
				_cap->calc(_output, _width, _i->point, _i->distance, next->point);
				_i = next;
				set_state(_i == last ? end_cap : outline_forward);
				break;

			case outline_forward:
				_join->calc(_output, _width, prev->point, prev->distance, _i->point, _i->distance, next->point);
				_i = next;
				if (_i == first && is_closed())
					set_state(end_poly1);
				else if (_i == last && !is_closed())
					set_state(end_cap);
				break;

			case end_poly1:
				_o = _output.end();
				set_state(outline_backward | moveto);
				return path_command_end_poly | path_flag_close;

			case end_cap:
				_cap->calc(_output, _width, _i->point, prev->distance, prev->point);
				_i = prev;
				set_state(_i == first ? end_poly : outline_backward);
				break;

			case outline_backward:
				_join->calc(_output, _width, next->point, _i->distance, _i->point, prev->distance, prev->point);
				_i = prev;
				if (_i == first)
					set_state(end_poly);
				break;

			case end_poly:
				_o = _output.end();
				set_state(stop);
				return path_command_end_poly | path_flag_close;

			case stop:
				return path_command_stop;
			}

			_o = _output.begin();
		}
		return path_command_stop;
	}
		
	void stroke::width(real_t w)
	{	_width = 0.5f * w;	}

	bool stroke::prepare()
	{
		if (_state & ready)
			return true;

		if (_input.size() <= (is_closed() ? 2u : 1u))
			return false;

		_i = _input.begin();
		_o = _output.end();
		set_state((is_closed() ? outline_forward : start_cap) | moveto | ready);
		return true;
	}

	bool stroke::is_closed() const
	{	return 0 != (_state & closed);	}

	void stroke::set_state(int stage_and_flags)
	{	_state = (_state & ~stage_mask) | stage_and_flags;	}

	void stroke::close()
	{
		if (!_input.empty())
		{
			const point_ref &first = *_input.begin();
			point_ref &last = *(_input.end() - 1);
		
			if (!last.set_distance_to(first.point))
				_input.pop_back();
			_state |= closed;
		}
	}
}
