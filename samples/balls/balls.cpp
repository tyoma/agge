#include "balls_data.h"

#include <agge/clipper.h>
#include <agge/filling_rules.h>
#include <agge/rasterizer.h>
#include <agge/renderer_parallel.h>

#include <misc/experiments/common/ellipse.h>

#include <samples/common/shell.h>
#include <samples/common/timing.h>

using namespace agge;
using namespace std;
using namespace common;

const int c_thread_count = 4;
const int c_balls_number = 2000;

namespace
{
	class blender_solid_color : public platform_blender_solid_color
	{
	public:
		blender_solid_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
			: platform_blender_solid_color(make_pixel(r, g, b, a), a)
		{	}

	private:
		pixel make_pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		{
			pixel p = { b, g, r, a };
			return p;
		}
	};

	class Balls : public application
	{
	public:
		Balls()
			: _renderer(c_thread_count), _balls(c_balls)
		{	_balls.resize(c_balls_number);	}

	private:
		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			long long counter;
			const float dt = 0.3f * (float)stopwatch(_balls_timer);
			const rect_i area = { 0, 0, surface.width(), surface.height() };

			_rasterizer.reset();

			stopwatch(counter);
				fill(surface, area, blender_solid_color(255, 255, 255));
			timings.clearing += stopwatch(counter);

			for (vector<ball>::iterator i = _balls.begin(); i != _balls.end(); ++i)
				move_and_bounce(*i, dt, static_cast<real_t>(surface.width()), static_cast<real_t>(surface.height()));

			for (vector<ball>::iterator i = _balls.begin(); i != _balls.end(); ++i)
			{
				ellipse e(i->x, i->y, i->radius, i->radius);
				blender_solid_color brush(i->color.r, i->color.g, i->color.b, i->color.a);

				_rasterizer.reset();

				stopwatch(counter);
				add_path(_rasterizer, e);
				_rasterizer.sort();
				timings.rasterization += stopwatch(counter);
				_renderer(surface, 0, _rasterizer, brush, winding<>());
				timings.rendition += stopwatch(counter);
			}
		}

	private:
		rasterizer< clipper<int> > _rasterizer;
		renderer_parallel _renderer;
		long long _balls_timer;
		vector<ball> _balls;
	};
}

application *agge_create_application()
{
	return new Balls;
}
