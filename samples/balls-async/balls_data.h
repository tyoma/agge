#pragma once

#include <agge/types.h>
#include <vector>

struct rgba8
{
	typedef agge::uint8_t value_type;

	value_type r, g, b, a;

	rgba8(unsigned r_, unsigned g_, unsigned b_, unsigned a_ = static_cast<value_type>(-1))
		: r(value_type(r_)),  g(value_type(g_)),  b(value_type(b_)),  a(value_type(a_))
	{	}
};

struct ball
{
	ball()
		: radius(1.0f), color(128, 128, 128, 10), vx(1.0f), vy(1.0f), x(1.0f), y(1.0f)
	{	}

	ball(agge::real_t radius_, rgba8 color_, agge::real_t vx_, agge::real_t vy_, agge::real_t x_, agge::real_t y_)
		: radius(radius_), color(color_), vx(vx_), vy(vy_), x(x_), y(y_)
	{	}

	agge::real_t radius;
	rgba8 color;
	agge::real_t vx, vy;
	agge::real_t x, y;
};

inline void move_and_bounce(ball &ball_, agge::real_t dt, agge::real_t w, agge::real_t h)
{
	const agge::real_t radius = ball_.radius;
	const agge::real_t l = radius, r = w - radius, t = radius, b = h - radius;
	agge::real_t &x = ball_.x, &y = ball_.y;
	agge::real_t &vx = ball_.vx, &vy = ball_.vy;

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
