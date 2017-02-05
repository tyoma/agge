#include "MainDialog.h"

#include "../common/bouncing.h"
#include "../common/paths.h"
#include "../common/timing.h"

#include <agge/blenders_simd.h>
#include <agge/clipper.h>
#include <agge/dash.h>
#include <agge/math.h>
#include <agge/rasterizer.h>
#include <agge/renderer_parallel.h>
#include <agge/stroke.h>
#include <agge/stroke_features.h>

#include <aggx/blenders.h>

#include <aggx/aggx_ellipse.h>

using namespace std;
using namespace demo;

const int c_thread_count = 1;
const int c_balls_number = 0;
typedef agge::simd::blender_solid_color blender_used;

namespace
{
	class unlimited_miter : public agge::stroke::join
	{
	public:
		virtual void calc(agge::points &output, agge::real_t w, const agge::point_r &v0, agge::real_t d01,
			const agge::point_r &v1, agge::real_t d12, const agge::point_r &v2) const
		{
			using namespace agge;

			d01 = w / d01;
			d12 = w / d12;

			const real_t dx1 = d01 * (v1.y - v0.y);
			const real_t dy1 = d01 * (v1.x - v0.x);
			const real_t dx2 = d12 * (v2.y - v1.y);
			const real_t dy2 = d12 * (v2.x - v1.x);

			real_t xi, yi;

			if (calc_intersection(v0.x + dx1, v0.y - dy1, v1.x + dx1, v1.y - dy1,
				v1.x + dx2, v1.y - dy2, v2.x + dx2, v2.y - dy2, &xi, &yi))
			{
				output.push_back(create_point(xi, yi));
			}
		}

	private:
		AGGE_INLINE static bool calc_intersection(agge::real_t ax, agge::real_t ay, agge::real_t bx, agge::real_t by,
			agge::real_t cx, agge::real_t cy, agge::real_t dx, agge::real_t dy, agge::real_t *x, agge::real_t *y)
		{
			using namespace agge;

			real_t num = (ay-cy) * (dx-cx) - (ax-cx) * (dy-cy);
			real_t den = (bx-ax) * (dy-cy) - (by-ay) * (dx-cx);
			if (fabs(den) < distance_epsilon)
				return false;	
			real_t r = num / den;
			*x = ax + r * (bx-ax);
			*y = ay + r * (by-ay);
			return true;
		}
	};

	agge::simd::blender_solid_color::pixel make_pixel(aggx::rgba8 color)
	{
		agge::simd::blender_solid_color::pixel p = { color.b, color.g, color.r, 0 };
		return p;
	}

	template <typename BlenderT>
	class blender : public BlenderT
	{
	public:
		blender(aggx::rgba8 color)
			: BlenderT(make_pixel(color), color.a)
		{	}
	};

	template <int precision>
	struct calculate_alpha
	{
		unsigned int operator ()(int area) const
		{
			area >>= precision + 1;
			if (area < 0)
				area = -area;
			if (area > 255)
				area = 255;
			return area;
		}
	};

	template <typename LinesSinkT, typename PathT>
	void add_path(LinesSinkT &sink, PathT &path)
	{
		using namespace agge;

		real_t x, y;

		path.rewind(0);
		for (int command; command = path.vertex(&x, &y), path_command_stop != command; )
		{
			if (path_command_line_to == (command & path_command_mask))
				sink.line_to(x, y);
			else if (path_command_move_to == (command & path_command_mask))
				sink.move_to(x, y);
			if (command & path_flag_close)
				sink.close_polygon();
		}
	}


	class agge_drawer : public Drawer
	{
	public:
		typedef blender<blender_used> solid_color_brush;

	public:
		agge_drawer()
			: _renderer(c_thread_count), _balls(c_balls)
		{
			_balls.resize(c_balls_number);
			_dash.add_dash(15.0f, 4.0f);
			_dash.add_dash(4.0f, 4.0f);
		}

	private:
		virtual void draw(::bitmap &surface, Timings &timings)
		{
			LARGE_INTEGER counter;
			const float dt = 0.3f * (float)stopwatch(_balls_timer);
			const agge::rect_i area = { 0, 0, surface.width(), surface.height() };

			_rasterizer.reset();

			stopwatch(counter);
				agge::fill(surface, area, solid_color_brush(aggx::rgba8(255, 255, 255)));
			timings.clearing += stopwatch(counter);

			if (_balls.empty())
			{
				stopwatch(counter);
					agg_path_adaptor p(_spiral);
					agge::path_generator_adapter<agg_path_adaptor, agge::stroke> path_stroke1(p, _stroke1);
					agge::path_generator_adapter<agge::path_generator_adapter<agg_path_adaptor, agge::stroke>, agge::stroke> path_stroke2(path_stroke1, _stroke2);

					agge::path_generator_adapter<agg_path_adaptor, agge::dash> path_stroke3(p, _dash);
					agge::path_generator_adapter<agge::path_generator_adapter<agg_path_adaptor, agge::dash>, agge::stroke> path_stroke4(path_stroke3, _stroke1);
					agge::path_generator_adapter<agge::path_generator_adapter<agge::path_generator_adapter<agg_path_adaptor, agge::dash>, agge::stroke>, agge::stroke> path_stroke5(path_stroke4, _stroke2);

					_stroke1.width(3.0f);
					_stroke1.set_cap(agge::caps::butt());
					_stroke1.set_join(unlimited_miter());

					_stroke2.width(2.0f);
					_stroke2.set_cap(agge::caps::butt());
					_stroke2.set_join(agge::joins::bevel());

					_spiral_flattened.clear();
					flatten<agge::real_t>(_spiral_flattened, path_stroke1);
				timings.stroking += stopwatch(counter);

				solid_color_brush brush(aggx::rgba8(0, 154, 255, 255));

				stopwatch(counter);
				add_path(_rasterizer, agg_path_adaptor(_spiral_flattened));
				_rasterizer.sort();
				timings.rasterization += stopwatch(counter);
				_renderer(surface, 0, _rasterizer, brush, calculate_alpha<agge::vector_rasterizer::_1_shift>());
				timings.rendition += stopwatch(counter);
			}

			for_each(_balls.begin(), _balls.end(), [&] (ball &b) {
				demo::move_and_bounce(b, dt, surface.width(), surface.height());
			});

			for_each(_balls.begin(), _balls.end(), [&] (ball &b) {
				aggx::ellipse e(b.x, b.y, b.radius, b.radius);

				_rasterizer.reset();

				stopwatch(counter);
				add_path(_rasterizer, e);
				_rasterizer.sort();
				timings.rasterization += stopwatch(counter);
				_renderer(surface, 0, _rasterizer, agge_drawer::solid_color_brush(b.color), calculate_alpha<agge::vector_rasterizer::_1_shift>());
				timings.rendition += stopwatch(counter);
			});
		}

		virtual void resize(int width, int height)
		{
			_spiral.clear();
			spiral(_spiral, width / 2, height / 2, 5, (std::min)(width, height) / 2 - 10, 1, 0);
		}

	private:
		agge::rasterizer< agge::clipper<int> > _rasterizer;
		__declspec(align(16)) agge::renderer_parallel _renderer;
		AggPath _spiral, _spiral_flattened;
		LARGE_INTEGER _balls_timer;
		vector<demo::ball> _balls;
		agge::stroke _stroke1, _stroke2;
		agge::dash _dash;
	};
}


int main()
{
	::SetProcessDPIAware();

	agge_drawer d;

	MainDialog dlg(d);

	MainDialog::PumpMessages();
}
