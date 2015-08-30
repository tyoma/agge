#include <agge/stroker.h>

#include <agge/stroke_features.h>
#include <agge/stroke_math.h>

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
		_initial = true;
		_state = start_cap;
	}

	void stroke::add_vertex(real_t x, real_t y, int /*command*/)
	{
		if (!_input.empty())
		{
			point_ref &last = *(_input.end() - 1);

			last.distance = distance(last.point.x, last.point.y, x, y);
		}

		point_ref p = { create_point(x, y), 0 };

		_input.push_back(p);
		_input_iterator = _input.begin();
	}
		
	int stroke::vertex(real_t *x, real_t *y)
	{
		if (_output_iterator != _output.end())
		{
			*x = _output_iterator->x;
			*y = _output_iterator->y;
			++_output_iterator;
			bool initial = _initial;
			_initial = false;
			return initial ? path_command_move_to : path_command_line_to;
		}

		_output.clear();

		switch (_state)
		{
		case start_cap:
			_cap->calc(_output, _width, _input_iterator->point, _input_iterator->distance, (_input_iterator + 1)->point);
			++_input_iterator;
			_state =  _input_iterator + 1 == _input.end() ? end_cap : outline_forward;
			break;

		case outline_forward:
			_join->calc(_output, _width, (_input_iterator - 1)->point, (_input_iterator - 1)->distance, _input_iterator->point,
				_input_iterator->distance, (_input_iterator + 1)->point);
			++_input_iterator;
			if (_input_iterator + 1 == _input.end())
				_state = end_cap;
			break;

		case end_cap:
			_cap->calc(_output, _width, _input_iterator->point, (_input_iterator - 1)->distance, (_input_iterator - 1)->point);
			--_input_iterator;
			_state =  _input_iterator == _input.begin() ? end_poly : outline_backward;
			break;

		case outline_backward:
			_join->calc(_output, _width, (_input_iterator + 1)->point, _input_iterator->distance, _input_iterator->point,
				(_input_iterator - 1)->distance, (_input_iterator - 1)->point);
			--_input_iterator;
			if (_input_iterator == _input.begin())
				_state = end_poly;
			break;

		case end_poly:
			_output_iterator = _output.end();
			_state = stop;
			return path_command_end_poly;

		case stop:
			return path_command_stop;
		}

		_output_iterator = _output.begin();
		return vertex(x, y);
	}
		
	void stroke::width(real_t w)
	{	_width = 0.5f * w;	}

	void stroke::generate()
	{
		input_vertices::const_iterator i = _input.begin();

		_cap->calc(_output, _width, i->point, i->distance, (i + 1)->point);
		++i;
		_cap->calc(_output, _width, i->point, (i - 1)->distance, (i - 1)->point);

		_output_iterator = _output.begin();
	}
}
