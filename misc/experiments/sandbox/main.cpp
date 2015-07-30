#include "MainDialog.h"

#include "../common/bouncing.h"
#include "../common/paths.h"

#include <agge/renderer_parallel.h>
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
#include <iostream>

using namespace agge;
using namespace aggx;
using namespace std;
using namespace demo;

const int c_thread_count = 1;
const bool c_use_original_agg = false;
const int c_balls_number = 0;//2000;
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

	int random(unsigned __int64 upper_bound)
	{	return static_cast<unsigned>(upper_bound * rand() / RAND_MAX);}

	float frandom()
	{	return static_cast<float>(1.0 * rand() / RAND_MAX);	}
}

int main()
{
	//for (int count = 2000; count; --count)
	//{
	//	cout << fixed << "ball("
	//		<< 70.0f * frandom() << "f, "
	//		<< "rgba8(" << random(255) << ", " << random(255) << ", " << random(255) << ", " << 10 + random(245) << "), "
	//		<< frandom() << "f, " << frandom() << "f, "
	//		<< "0.0f, 0.0f)," << endl;
	//}

	typedef rasterizer_scanline_aa<agg::rasterizer_sl_no_clip> rasterizer_scanline;

	srand((unsigned)time(0));

	vector<demo::ball> balls(c_balls);

	balls.resize(c_balls_number);

	MSG msg;
	rasterizer_scanline ras;
	agge::renderer_parallel myrenderer(c_thread_count);

	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	AggPath spiral_line, spiral_flatten;
	LARGE_INTEGER timer;

	stopwatch(timer);

	MainDialog dlg([&](bitmap &target, double &clearing, double &rasterization, double &rendition) {
		typedef blender<blender_used> blenderx;

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

		const float dt = 0.3f * (float)stopwatch(timer);

		if (!c_use_original_agg)
		{
			if (!c_balls_number)
			{
				blenderx brush(rgba8(0, 154, 255, 230));

				stopwatch(counter);
				ras.add_path(agg_path_adaptor(spiral_flatten));
				ras.prepare();
				rasterization += stopwatch(counter);
				myrenderer(target, 0, ras.get_mask(), brush, calculate_alpha<8>());
				rendition += stopwatch(counter);
			}

			for_each(balls.begin(), balls.end(), [&] (ball &b) {
				demo::move_and_bounce(b, dt, target.width(), target.height());
			});

			for_each(balls.begin(), balls.end(), [&] (ball &b) {
				aggx::ellipse e(b.x, b.y, b.radius, b.radius);

				ras.reset();

				stopwatch(counter);
				ras.add_path(e);
				ras.prepare();
				rasterization += stopwatch(counter);
				myrenderer(target, 0, ras.get_mask(), blenderx(b.color), calculate_alpha<8>());
				rendition += stopwatch(counter);
			});
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

			if (!c_balls_number)
			{
				stopwatch(counter);
				ras_aa.add_path(agg_path_adaptor(spiral_flatten));
				ras_aa.sort();
				rasterization += stopwatch(counter);
				ren_aa.color(agg::rgba8(0, 154, 255, 255));
				agg::render_scanlines(ras_aa, sl, ren_aa);
				rendition += stopwatch(counter);
			}

			for_each(balls.begin(), balls.end(), [&] (ball &b) {
				demo::move_and_bounce(b, dt, target.width(), target.height());
			});

			for_each(balls.begin(), balls.end(), [&] (ball &b) {
				agg::ellipse e(b.x, b.y, b.radius, b.radius);

				ras_aa.reset();

				stopwatch(counter);
				ras_aa.add_path(e);
				ras_aa.sort();
				rasterization += stopwatch(counter);
				ren_aa.color(agg::rgba8(b.color.r, b.color.g, b.color.b, b.color.a));
				agg::render_scanlines(ras_aa, sl, ren_aa);
				rendition += stopwatch(counter);
			});
		}
	});

	while (::GetMessage(&msg, NULL, 0, 0))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}
