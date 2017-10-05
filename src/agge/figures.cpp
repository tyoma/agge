#include <agge/figures.h>

#include <agge/path.h>

namespace agge
{
	rectangle::rectangle(real_t x1, real_t y1, real_t x2, real_t y2)
	{
		_rect.x1 = x1;
		_rect.y1 = y1;
		_rect.x2 = x2;
		_rect.y2 = y2;
	}

	rectangle::iterator rectangle::iterate() const
	{	return iterator(_rect);	}


	rectangle::iterator::iterator(const rect<real_t> &r)
		: _rect(r), _step(0)
	{	}

	int rectangle::iterator::vertex(real_t *x, real_t *y)
	{
		switch (_step++)
		{
		case 0:
			*x = _rect.x1, *y = _rect.y1;
			return path_command_move_to;

		case 1:
			*x = _rect.x2, *y = _rect.y1;
			return path_command_line_to;

		case 2:
			*x = _rect.x2, *y = _rect.y2;
			return path_command_line_to;

		case 3:
			*x = _rect.x1, *y = _rect.y2;
			return path_command_line_to;

		case 4:
			return path_command_end_poly | path_flag_close;

		default:
			return path_command_stop;
		}
	}
}
