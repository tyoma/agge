#pragma once

#include <aggx/basics.h>
#include <vector>

namespace demo
{
	struct ball
	{
		ball()
			: radius(1.0f), color(128, 128, 128, 10), vx(1.0f), vy(1.0f), x(1.0f), y(1.0f)
		{	}

		ball(aggx::real radius_, aggx::rgba8 color_, aggx::real vx_, aggx::real vy_, aggx::real x_, aggx::real y_)
			: radius(radius_), color(color_), vx(vx_), vy(vy_), x(x_), y(y_)
		{	}

		aggx::real radius;
		aggx::rgba8 color;
		aggx::real vx, vy;
		aggx::real x, y;
	};

	inline void move_and_bounce(ball &ball, aggx::real dt, aggx::real w, aggx::real h)
	{
		const aggx::real radius = ball.radius;
		const aggx::real l = radius, r = w - radius, t = radius, b = h - radius;
		aggx::real &x = ball.x, &y = ball.y;
		aggx::real &vx = ball.vx, &vy = ball.vy;
		
		x += dt * vx, y += dt * vy;

		if (x < l)
			x += l - x, vx = -vx;
		else if (x > r)
			x -= x - r, vx = -vx;

		if (y < t)
			y += t - y, vy = -vy;
		else if (y > b)
			y -= y - b, vy = -vy;
	}

	extern const std::vector<ball> c_balls;
}
