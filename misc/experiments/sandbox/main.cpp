#include "MainDialog.h"

#include <agge/scanline.h>
#include <agge/blenders_simd.h>

#include <aggx/rasterizer.h>
#include <aggx/blenders.h>

#include <aggx/win32_bitmap.h>

#include <aggx/agg_conv_stroke.h>
#include <aggx/agg_ellipse.h>

#include <agg_rasterizer_sl_clip.h>

#include <agg_ellipse.h>
#include <agg_pixfmt_rgba.h>
#include <agg_renderer_base.h>
#include <agg_scanline_u.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_scanline.h>

#include <time.h>
#include <windows.h>

using namespace agge;
using namespace aggx;
using namespace std;

const bool c_use_original_agg = false;
const int c_ellipses_number = 2000;
typedef simd::blender_solid_color blender_used;

namespace
{
	agge::simd::blender_solid_color::pixel make_pixel(rgba8 color)
	{
		agge::simd::blender_solid_color::pixel p = { color.b, color.g, color.r, 0 };
		return p;
	}

	template <typename BlenderT>
	class blender : public BlenderT
	{
	public:
		blender(rgba8 color)
			: BlenderT(make_pixel(color), color.a)
		{	}
	};

	typedef std::vector< std::pair<std::pair<aggx::real, aggx::real>, unsigned> > AggPath;

	class bitmap_rendering_buffer
	{
	public:
		typedef unsigned int pixel_type;
		typedef agg::const_row_info<pixel_type> row_data;

	public:
		bitmap_rendering_buffer(bitmap &target)
			: _target(target)
		{	}

		pixel_type *row_ptr(int, int y, int)
		{	return reinterpret_cast<pixel_type *>(_target.row_ptr(y));	}

		unsigned int width() const
		{	return _target.width();	}

		unsigned int height() const
		{	return _target.height();	}

	private:
		bitmap &_target;
	};

	double stopwatch(LARGE_INTEGER &counter)
	{
		double value;
		LARGE_INTEGER current, f;

		::QueryPerformanceCounter(&current);
		::QueryPerformanceFrequency(&f);
		value = 1000.0 * (current.QuadPart - counter.QuadPart) / f.QuadPart;
		counter = current;
		return value;
	}

	class agg_path_adaptor
	{
	public:
		agg_path_adaptor(const AggPath &path)
			: _path(path), _position(_path.begin())
		{
		}

		void rewind(unsigned)
		{
			_position = _path.begin();
		}

		unsigned vertex(float* x, float* y)
		{
			if (_position == _path.end())
				return path_cmd_stop;
			else
				return *x = _position->first.first, *y = _position->first.second, _position++->second;
		}

		unsigned vertex(double* x, double* y)
		{
			if (_position == _path.end())
				return path_cmd_stop;
			else
				return *x = _position->first.first, *y = _position->first.second, _position++->second;
		}

	private:
		const AggPath &_path;
		AggPath::const_iterator _position;
	};

	void pathStart(AggPath &/*path*/)
	{	}

	void pathMoveTo(AggPath &path, real x, real y)
	{	path.push_back(make_pair(make_pair(x, y), path_cmd_move_to));	}
	
	void pathLineTo(AggPath &path, real x, real y)
	{	path.push_back(make_pair(make_pair(x, y), path_cmd_line_to));	}

	void pathEnd(AggPath &path)
	{	path.push_back(make_pair(make_pair(0.0f, 0.0f), path_cmd_stop));	}


	template <typename TargetT>
	void spiral(TargetT &target, real x, real y, real r1, real r2, real step, real start_angle)
	{
		const float k = 4.0f;

		bool start = true;

		pathStart(target);
		for (real angle = start_angle, dr = k * step / 45.0f, da = k / 180.0f * pi; r1 < r2; r1 += dr, angle += da, start = false)
		{
			const real px = x + aggx::cos(angle) * r1, py = y + aggx::sin(angle) * r1;

			if (start)
				pathMoveTo(target, px, py);
			else
				pathLineTo(target, px, py);
		}
		pathEnd(target);
	}

	template <typename TargetT, typename PathT>
	void flatten(TargetT &destination, PathT &source)
	{
		unsigned cmd;
		real x, y;

		source.rewind(0);
		while (!is_stop(cmd = source.vertex(&x, &y)))
			destination.push_back(make_pair(make_pair(x, y), cmd));
	}

	int random(unsigned __int64 upper_bound)
	{	return static_cast<unsigned>(upper_bound * rand() / RAND_MAX);}
}

int main()
{
	typedef rasterizer_scanline_aa<agg::rasterizer_sl_no_clip> rasterizer_scanline;

	srand((unsigned)time(0));

	vector< pair<rect_r, rgba8> > ellipses;

	const int max_radius = 50;

	for (int n = c_ellipses_number; n; --n)
	{
		rect_r r(random(1920 - 2 * max_radius) + max_radius, random(1080 - 2 * max_radius) + max_radius, 0, 0);
		
		r.x2 = random(max_radius) + 1, r.y2 = random(max_radius) + 1;
		
		rgba8 c(random(255), random(255), random(255), 200);

		ellipses.push_back(make_pair(r, c));
	}

	MSG msg;
	rasterizer_scanline ras;

	AggPath spiral_line, spiral_flatten;

	MainDialog dlg([&](bitmap &target, double &clearing, double &rasterization, double &rendition) {
		typedef blender<blender_used> blenderx;
		typedef renderer<bitmap, blenderx> renderer;
		typedef scanline_adapter<renderer> scanline;

		LARGE_INTEGER counter;

		stopwatch(counter);
		fill(target, blenderx(rgba8(255, 255, 255)));
		clearing += stopwatch(counter);

		spiral_line.clear();
		spiral(spiral_line, target.width() / 2, target.height() / 2, 5, (std::min)(target.width(), target.height()) / 2 - 10, 1, 0);

		agg_path_adaptor p(spiral_line);
		conv_stroke<agg_path_adaptor> stroke(p);

		stroke.width(3);

		spiral_flatten.clear();
		flatten(spiral_flatten, stroke);

		rasterization = 0;
		rendition = 0;

		if (!c_use_original_agg)
		{
			if (!c_ellipses_number)
			{
				stopwatch(counter);
				ras.add_path(agg_path_adaptor(spiral_flatten));
				ras.prepare();
				rasterization += stopwatch(counter);
				blenderx b(rgba8(0, 154, 255, 255));
				ras.render<scanline>(renderer(target, b));
				rendition += stopwatch(counter);
			}

			for (auto i = ellipses.begin(); i != ellipses.end(); ++i)
			{
				aggx::ellipse e(i->first.x1, i->first.y1, i->first.x2, i->first.y2);

				ras.reset();

				stopwatch(counter);
				ras.add_path(e);
				ras.prepare();
				rasterization += stopwatch(counter);
				blenderx b(i->second);
				ras.render<scanline>(renderer(target, b));
				rendition += stopwatch(counter);
			}
		}
		else
		{
			typedef agg::pixfmt_alpha_blend_rgba<agg::blender_bgra32, bitmap_rendering_buffer> pixfmt;
			typedef agg::rgba8 color_type;
			typedef agg::order_bgra component_order;
			typedef agg::renderer_base<pixfmt> renderer_base;
			typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_aa;
			typedef agg::rasterizer_scanline_aa<agg::rasterizer_sl_no_clip> rasterizer_scanline;
			typedef agg::scanline_u8 scanline;

			bitmap_rendering_buffer rbuf(target);
			pixfmt pixf(rbuf);
			renderer_base rb(pixf);
			renderer_aa ren_aa(rb);
			rasterizer_scanline ras_aa;
			scanline sl;

			if (!c_ellipses_number)
			{
				stopwatch(counter);
				ras_aa.add_path(agg_path_adaptor(spiral_flatten));
				ras_aa.sort();
				rasterization += stopwatch(counter);
				ren_aa.color(agg::rgba8(0, 154, 255, 255));
				agg::render_scanlines(ras_aa, sl, ren_aa);
				rendition += stopwatch(counter);
			}

			for (auto i = ellipses.begin(); i != ellipses.end(); ++i)
			{
				agg::ellipse e(i->first.x1, i->first.y1, i->first.x2, i->first.y2);

				ras_aa.reset();

				stopwatch(counter);
				ras_aa.add_path(e);
				ras_aa.sort();
				rasterization += stopwatch(counter);
				ren_aa.color(agg::rgba8(i->second.r, i->second.g, i->second.b, i->second.a));
				agg::render_scanlines(ras_aa, sl, ren_aa);
				rendition += stopwatch(counter);
			}
		}
	});

	while (::GetMessage(&msg, NULL, 0, 0))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}
