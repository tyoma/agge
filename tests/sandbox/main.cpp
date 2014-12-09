#include "MainDialog.h"

#include <aggx/scanline.h>
#include <aggx/scanline_adapter.h>
#include <aggx/rasterizer.h>
#include <aggx/renderer.h>
#include <aggx/rendition_adapter.h>
#include <aggx/blenders.h>
#include <aggx/blenders_intel.h>

#include <aggx/win32_bitmap.h>

#include <aggx/agg_conv_stroke.h>
#include <aggx/agg_ellipse.h>

#include <agg/include/agg_rasterizer_sl_clip.h>

#include <time.h>
#include <windows.h>

using namespace aggx;
using namespace std;

namespace
{
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

	class spiral
	{
	public:
		spiral(real x, real y, real r1, real r2, real step, real start_angle=0) :
		  m_x(x), 
			  m_y(y), 
			  m_r1(r1), 
			  m_r2(r2), 
			  m_step(step), 
			  m_start_angle(start_angle),
			  m_angle(start_angle),
			  m_da(deg2rad(1.0)),
			  m_dr(m_step / 45.0f)
		  {
		  }

		  void rewind(unsigned) 
		  { 
			  m_angle = m_start_angle; 
			  m_curr_r = m_r1; 
			  m_start = true; 
		  }

		  unsigned vertex(real* x, real* y)
		  {
			  if(m_curr_r > m_r2) return path_cmd_stop;

			  *x = m_x + aggx::cos(m_angle) * m_curr_r;
			  *y = m_y + aggx::sin(m_angle) * m_curr_r;
			  m_curr_r += m_dr;
			  m_angle += m_da;
			  if(m_start) 
			  {
				  m_start = false;
				  return path_cmd_move_to;
			  }
			  return path_cmd_line_to;
		  }

	private:
		real m_x;
		real m_y;
		real m_r1;
		real m_r2;
		real m_step;
		real m_start_angle;

		real m_angle;
		real m_curr_r;
		real m_da;
		real m_dr;
		bool   m_start;
	};

	int random(unsigned __int64 upper_bound)
	{	return static_cast<unsigned>(upper_bound * rand() / RAND_MAX);}
}

int main()
{
	typedef rasterizer_scanline_aa<agg::rasterizer_sl_no_clip/*agg::rasterizer_sl_clip_int*/> rasterizer_scanline;

	srand((unsigned)time(0));

	vector< pair<rect_r, rgba8> > ellipses;

	for (int n = 400; n; --n)
	{
		rect_r r(random(1620), random(1080), 0, 0);
		
		r.x2 = r.x1 + random(300), r.y2 = r.y1 + random(200);
		
		rgba8 c(random(255), random(255), random(255), 255);

		ellipses.push_back(make_pair(r, c));
	}

	//ellipses.push_back(make_pair(rect_r(10, 10, 999, 850), rgba8(23, 190, 250, 224)));
	//ellipses.push_back(make_pair(rect_r(10, 10, 1900, 1000), rgba8(23, 23, 250, 100)));
	//ellipses.push_back(make_pair(rect_r(600, 400, 1900, 1000), rgba8(255, 30, 10, 224)));
	//ellipses.push_back(make_pair(rect_r(600, 400, 1900, 1000), rgba8(255, 30, 10, 224)));
	//ellipses.push_back(make_pair(rect_r(600, 400, 1900, 1000), rgba8(255, 30, 10, 224)));
	//ellipses.push_back(make_pair(rect_r(600, 400, 1900, 1000), rgba8(255, 30, 10, 224)));

	MSG msg;
	rasterizer_scanline ras;

	MainDialog dlg([&](bitmap &target, double &rasterization, double &rendition) {

//		typedef blender_solid_color<bitmap::pixel> blenderx;
		typedef intel::blender_solid_color blenderx;
		typedef rendition_adapter<bitmap, blenderx> renderer;
		typedef scanline_adapter<renderer> scanline;

		renderer(target, blenderx(rgba8(255, 255, 255))).clear();

		LARGE_INTEGER counter;
		rasterization = 0;
		rendition = 0;

		//for_each(ellipses.begin(), ellipses.end(), [&] (const pair<rect_r, rgba8> &e) {
		//	aggx::ellipse ellipse(0.5 * (e.first.x1 + e.first.x2), 0.5 * (e.first.y1 + e.first.y2),
		//		0.5 * (e.first.x2 - e.first.x1), 0.5 * (e.first.y2 - e.first.y1));

		//	stopwatch(counter);

		//	ras.add_path(ellipse);
		//	ras.prepare();

		//	rasterization += stopwatch(counter);

		//	renderer r(target, blenderx(e.second));

		//	ras.render<scanline>(r);
		//	
		//	rendition += stopwatch(counter);
		//});

		stopwatch(counter);

		spiral s4(target.width() / 2, target.height() / 2, 5, (std::min)(target.width(), target.height()) / 2 - 10, 1, 0);
		conv_stroke<spiral> stroke(s4);

		stroke.width(3);
		stroke.line_cap(round_cap);
		ras.add_path(stroke);

		ras.move_to(100 * 0x100, 100 * 0x100);
		ras.line_to(100 * 0x100, 101 * 0x100);
		ras.line_to(101 * 0x100, 100 * 0x100);
		ras.line_to(100 * 0x100, 100 * 0x100);

		ras.prepare();

		rasterization += stopwatch(counter);

		renderer r(target, blenderx(rgba8(0, 154, 255, 255)));
		ras.render<scanline>(r);

		rendition += stopwatch(counter);
	});

	while (::GetMessage(&msg, NULL, 0, 0))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}
