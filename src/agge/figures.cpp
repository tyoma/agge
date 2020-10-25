#include <agge/figures.h>

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
}
