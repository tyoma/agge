#include "../common/blenders.h"
#include "../common/bouncing.h"
#include "../common/ellipse.h"
#include "../common/MainDialog.h"
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

using namespace agge;
using namespace std;
using namespace common;

const int c_thread_count = 1;
const int c_balls_number = 0;
typedef simd::blender_solid_color blender_used;

namespace
{
	class unlimited_miter : public stroke::join
	{
	public:
		virtual void calc(points &output, real_t w, const point_r &v0, real_t d01,
			const point_r &v1, real_t d12, const point_r &v2) const
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
		AGGE_INLINE static bool calc_intersection(real_t ax, real_t ay, real_t bx, real_t by,
			real_t cx, real_t cy, real_t dx, real_t dy, real_t *x, real_t *y)
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

	simd::blender_solid_color::pixel make_pixel(rgba8 color)
	{
		simd::blender_solid_color::pixel p = { color.b, color.g, color.r, 0 };
		return p;
	}

	template <typename BlenderT>
	class blender2 : public BlenderT
	{
	public:
		blender2(rgba8 color)
			: BlenderT(make_pixel(color), color.a)
		{	}
	};

	template <int precision>
	struct calculate_alpha
	{
		uint8_t operator ()(int area) const
		{
			area >>= precision + 1;
			if (area < 0)
				area = -area;
			if (area > 255)
				area = 255;
			return static_cast<uint8_t>(area);
		}
	};

	template <typename LinesSinkT, typename PathT>
	void add_path(LinesSinkT &sink, PathT &path)
	{
		using namespace agge;

		real_t x, y;

		path.rewind(0);
		for (int command; command = path.vertex(&x, &y), path_command_stop != command; )
			add_polyline_vertex(sink, x, y, command);
	}


	class agge_drawer : public Drawer
	{
	public:
		typedef blender2<blender_used> solid_color_brush;

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
			long long counter;
			const float dt = 0.3f * (float)stopwatch(_balls_timer);
			const rect_i area = { 0, 0, surface.width(), surface.height() };

			_rasterizer.reset();

			stopwatch(counter);
				fill(surface, area, solid_color_brush(rgba8(255, 255, 255)));
			timings.clearing += stopwatch(counter);

			if (_balls.empty())
			{
				stopwatch(counter);
					agg_path_adaptor p(_spiral);
					path_generator_adapter<agg_path_adaptor, stroke> path_stroke1(p, _stroke1);
					path_generator_adapter<path_generator_adapter<agg_path_adaptor, stroke>, stroke> path_stroke2(path_stroke1, _stroke2);

					path_generator_adapter<agg_path_adaptor, dash> path_stroke3(p, _dash);
					path_generator_adapter<path_generator_adapter<agg_path_adaptor, dash>, stroke> path_stroke4(path_stroke3, _stroke1);
					path_generator_adapter<path_generator_adapter<path_generator_adapter<agg_path_adaptor, dash>, stroke>, stroke> path_stroke5(path_stroke4, _stroke2);

					_stroke1.width(3.0f);
					_stroke1.set_cap(caps::butt());
					_stroke1.set_join(unlimited_miter());

					_stroke2.width(2.0f);
					_stroke2.set_cap(caps::butt());
					_stroke2.set_join(joins::bevel());

					_spiral_flattened.clear();
					flatten<real_t>(_spiral_flattened, path_stroke1);
				timings.stroking += stopwatch(counter);

				solid_color_brush brush(rgba8(0, 154, 255, 255));
				agg_path_adaptor spiral(_spiral_flattened);

				stopwatch(counter);
				add_path(_rasterizer, spiral);
				_rasterizer.sort();
				timings.rasterization += stopwatch(counter);
				_renderer(surface, 0, _rasterizer, brush, calculate_alpha<vector_rasterizer::_1_shift>());
				timings.rendition += stopwatch(counter);
			}

			for (vector<ball>::iterator i = _balls.begin(); i != _balls.end(); ++i)
				move_and_bounce(*i, dt, static_cast<real_t>(surface.width()), static_cast<real_t>(surface.height()));

			for (vector<ball>::iterator i = _balls.begin(); i != _balls.end(); ++i)
			{
				ellipse e(i->x, i->y, i->radius, i->radius);

				_rasterizer.reset();

				stopwatch(counter);
				add_path(_rasterizer, e);
				_rasterizer.sort();
				timings.rasterization += stopwatch(counter);
				_renderer(surface, 0, _rasterizer, agge_drawer::solid_color_brush(i->color), calculate_alpha<vector_rasterizer::_1_shift>());
				timings.rendition += stopwatch(counter);
			}
		}

		virtual void resize(int width, int height)
		{
			_spiral.clear();
			spiral(_spiral, width / 2.0f, height / 2.0f, 5, (std::min)(width, height) / 2.0f - 10.0f, 1.0f, 0.0f);
		}

	private:
		rasterizer< clipper<int> > _rasterizer;
		__declspec(align(16)) renderer_parallel _renderer;
		AggPath _spiral, _spiral_flattened;
		long long _balls_timer;
		vector<ball> _balls;
		stroke _stroke1, _stroke2;
		dash _dash;
	};
}


int main()
{
	agge_drawer d;

	MainDialog dlg(d);

	MainDialog::PumpMessages();
}
