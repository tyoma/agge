#include <agge/clipper.h>
#include <agge/dash.h>
#include <agge/filling_rules.h>
#include <agge/math.h>
#include <agge/rasterizer.h>
#include <agge/renderer.h>
#include <agge/stroke.h>
#include <agge/stroke_features.h>

#include <misc/experiments/common/paths.h>

#include <samples/common/shell.h>
#include <samples/common/timing.h>

using namespace agge;
using namespace std;
using namespace common;

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


	class Sandbox : public application
	{
	public:
		Sandbox()
		{
			_dash.add_dash(15.0f, 4.0f);
			_dash.add_dash(4.0f, 4.0f);
		}

	private:
		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			long long counter;
			const rect_i area = { 0, 0, surface.width(), surface.height() };

			_rasterizer.reset();

			stopwatch(counter);
				fill(surface, area, blender_solid_color(255, 255, 255));
			timings.clearing += stopwatch(counter);

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

			blender_solid_color brush(0, 154, 255);
			agg_path_adaptor spiral(_spiral_flattened);

			stopwatch(counter);
			add_path(_rasterizer, spiral);
			_rasterizer.sort();
			timings.rasterization += stopwatch(counter);
			_renderer(surface, 0, _rasterizer, brush, winding<>());
			timings.rendition += stopwatch(counter);
		}

		virtual void resize(int width, int height)
		{
			_spiral.clear();
			spiral(_spiral, width / 2.0f, height / 2.0f, 5, (std::min)(width, height) / 2.0f - 10.0f, 1.0f, 0.0f);
		}

	private:
		rasterizer< clipper<int> > _rasterizer;
		renderer _renderer;
		AggPath _spiral, _spiral_flattened;
		stroke _stroke1, _stroke2;
		dash _dash;
	};
}

application *agge_create_application()
{
	return new Sandbox;
}
