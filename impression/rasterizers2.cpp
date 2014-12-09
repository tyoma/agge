#include "pixmap.h"

#include "renderer.h"
#include "scanline.h"
#include "aggx/rasterizer.h"

#include "ctrl/agg_slider_ctrl.h"
#include "ctrl/agg_cbox_ctrl.h"
#include "platform/agg_platform_support.h"

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>
#include "agg_rasterizer_sl_clip.h"
//#include "agg_scanline_u.h"
//#include "agg_rendering_buffer.h"
//#include "agg_renderer_scanline.h"
//#include "agg_rasterizer_scanline_aa.h"

//#define AGG_BGR24 
//#define AGG_RGB24
#define AGG_BGRA32 
//#define AGG_RGBA32 
//#define AGG_ARGB32 
//#define AGG_ABGR32
//#define AGG_RGB565
//#define AGG_RGB555
#include "pixel_formats.h"

template<class ColorT, class Order>
struct blender_rgb
{
	typedef ColorT color_type;
	typedef Order order_type;
	typedef typename color_type::value_type value_type;
	typedef typename color_type::calc_type calc_type;
	enum base_scale_e { base_shift = color_type::base_shift };

	static void blend_pix(value_type* p, unsigned cr, unsigned cg, unsigned cb, unsigned alpha, unsigned cover)
	{
//		__m64 malpha = _mm_set_pi16(alpha, alpha, alpha, 0);
		//__m64 new_pixel = _mm_set_pi16(cr, cg, cb, 0);
		//__m64 current_pixel = _mm_set_pi16(p[Order::R], p[Order::G], p[Order::B], 0);

		//new_pixel = _mm_subs_pi16(new_pixel, current_pixel);
		//new_pixel = _mm_sll_pi16(new_pixel, _mm_set_pi16(8, 8, 8, 0));
		//new_pixel = _mm_mulhi_pi16(new_pixel, _mm_sll_pi16(_mm_set_pi16(alpha, alpha, alpha, 0), _mm_set_pi16(8, 8, 8, 0)));
		//new_pixel = _mm_srl_pi16(new_pixel, _mm_set_pi16(8, 8, 8, 0));
		//current_pixel = _mm_adds_pu16(current_pixel, new_pixel);
		//p[Order::R] = current_pixel.m64_u16[0];
		//p[Order::G] = current_pixel.m64_u16[1];
		//p[Order::B] = current_pixel.m64_u16[2];

		p[Order::R] += (value_type)(((cr - p[Order::R]) * alpha) >> base_shift);
		p[Order::G] += (value_type)(((cg - p[Order::G]) * alpha) >> base_shift);
		p[Order::B] += (value_type)(((cb - p[Order::B]) * alpha) >> base_shift);
	}
};


class spiral
{
public:
    spiral(double x, double y, double r1, double r2, double step, double start_angle=0) :
        m_x(x), 
        m_y(y), 
        m_r1(r1), 
        m_r2(r2), 
        m_step(step), 
        m_start_angle(start_angle),
        m_angle(start_angle),
        m_da(agg::deg2rad(1.0)),
        m_dr(m_step / 45.0)
    {
    }

    void rewind(unsigned) 
    { 
        m_angle = m_start_angle; 
        m_curr_r = m_r1; 
        m_start = true; 
    }

    unsigned vertex(double* x, double* y)
    {
        if(m_curr_r > m_r2) return agg::path_cmd_stop;

        *x = m_x + cos(m_angle) * m_curr_r;
        *y = m_y + sin(m_angle) * m_curr_r;
        m_curr_r += m_dr;
        m_angle += m_da;
        if(m_start) 
        {
            m_start = false;
            return agg::path_cmd_move_to;
        }
        return agg::path_cmd_line_to;
    }

private:
    double m_x;
    double m_y;
    double m_r1;
    double m_r2;
    double m_step;
    double m_start_angle;

    double m_angle;
    double m_curr_r;
    double m_da;
    double m_dr;
    bool   m_start;
};


class the_application : public agg::platform_support
{
    agg::slider_ctrl<agg::rgba8> m_step;
    agg::slider_ctrl<agg::rgba8> m_width;
    agg::cbox_ctrl<agg::rgba8>   m_test;
    agg::cbox_ctrl<agg::rgba8>   m_rotate;
    double m_start_angle;

public:
	typedef agg2::pixmap<blender_rgb<agg::rgba8, agg::order_bgr>, agg::rendering_buffer> pixfmt;
	typedef agg2::renderer_base<pixfmt> renderer_base;
	typedef aggx::rasterizer_scanline_aa<agg::rasterizer_sl_no_clip/*agg::rasterizer_sl_clip_int*/> rasterizer_scanline;
	typedef agg2::scanline_u8 scanline;

    the_application(agg::pix_format_e format, bool flip_y) :
        agg::platform_support(format, flip_y),
        m_step(10.0, 10.0 + 4.0, 150.0, 10.0 + 8.0 + 4.0, !flip_y),
        m_width(150.0 + 10.0, 10.0 + 4.0, 400 - 10.0, 10.0 + 8.0 + 4.0, !flip_y),
        m_test(10.0, 10.0 + 4.0 + 16.0,    "Test Performance", !flip_y),
        m_rotate(130 + 10.0, 10.0 + 4.0 + 16.0,    "Rotate", !flip_y),
        m_start_angle(0.0)
    {
        add_ctrl(m_step);
        m_step.range(0.0, 2.0);
        m_step.value(0.1);
        m_step.label("Step=%1.2f");
        m_step.no_transform();

        add_ctrl(m_width);
        m_width.range(0.0, 7.0);
        m_width.value(3.0);
        m_width.label("Width=%1.2f");
        m_width.no_transform();

        add_ctrl(m_test);
        m_test.text_size(9.0, 7.0);
        m_test.no_transform();

        add_ctrl(m_rotate);
        m_rotate.text_size(9.0, 7.0);
        m_rotate.no_transform();
    }


    class scanline_hit_test
    {
    public:
		typedef agg::int8u cover_type;
		typedef agg::int16 coord_type;

		struct span
		{
			coord_type  x;
			coord_type  len;
			cover_type* covers;
		};

		typedef const span* const_iterator;

        scanline_hit_test(int x) : m_x(x), m_hit(false) {}

		void reset(int xmin, int xmax) { }
        void reset_spans() {}
        void finalize(int) {}
		int y() const { return 0; }
        void add_cell(int x, int)
        {
            if(m_x == x) m_hit = true;
        }
        void add_span(int x, int len, int)
        {
            if(m_x >= x && m_x < x+len) m_hit = true;
        }
        bool hit() const { return m_hit; }

		unsigned num_spans() const { return 0; }
		const_iterator begin() const { return 0; }

    private:
        int  m_x;
        bool m_hit;
    };

	void draw_anti_aliased_scanline(rasterizer_scanline& ras, scanline& sl, renderer_base& ren)
	{
		spiral s4(width() / 2, height() / 2, 5, (std::min)(width(), height()) / 2 - 10, 1, m_start_angle);
		agg::conv_stroke<spiral> stroke(s4);

		stroke.width(m_width.value());
		stroke.line_cap(agg::round_cap);
		ras.add_path(stroke);

		agg2::render_scanlines_aa_solid(ras, sl, ren, agg::rgba(0.0, 0.6, 1));
	}

	rasterizer_scanline m_ras_aa;

    virtual void on_draw()
    {
        pixfmt pf(rbuf_window());
        renderer_base renderer(pf);
        scanline sl;

		m_ras_aa.reset();
        renderer.clear(agg::rgba(1.0, 1.0, 0.95));

        draw_anti_aliased_scanline(m_ras_aa, sl, renderer);

        agg::render_ctrl(m_ras_aa, sl, renderer, m_step);
        agg::render_ctrl(m_ras_aa, sl, renderer, m_width);
        agg::render_ctrl(m_ras_aa, sl, renderer, m_test);
        agg::render_ctrl(m_ras_aa, sl, renderer, m_rotate);
    }


    virtual void on_idle()
    {
        m_start_angle += agg::deg2rad(m_step.value());
        if(m_start_angle > agg::deg2rad(360.0)) m_start_angle -= agg::deg2rad(360.0);
        force_redraw();
    }

	virtual void on_ctrl_change()
	{
		wait_mode(!m_rotate.status());

		if(m_test.status())
		{
			on_draw();
			update_window();

			pixfmt pf(rbuf_window());
			renderer_base ren_base(pf);
			rasterizer_scanline ras_aa;
			scanline sl;

			unsigned i;


			m_start_angle = 0;
			start_timer();
			for(i = 0; i < 200; i++)
			{
				draw_anti_aliased_scanline(ras_aa, sl, ren_base);
				m_start_angle += agg::deg2rad(m_step.value());
			}
			double t = elapsed_time();
			m_start_angle = 0;

			m_test.status(false);
			force_redraw();
			char buf[256];
			sprintf(buf, "Scanline=%1.2fms", t);
			message(buf);
		}
	}
};

int agg_main(int argc, char* argv[])
{
    the_application app(agg::pix_format_bgr24, true);
    app.caption("AGG Example. Line Join");

    if(app.init(500, 450, 0))
    {
        return app.run();
    }

    return 1;
}
