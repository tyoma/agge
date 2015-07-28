#pragma once

#include <agge/renderer.h>
#include <agge/scanline.h>
#include <agge/vector_rasterizer.h>
#include <agge/parallel.h>

#include "basics.h"

#include <algorithm>
#include <functional>

namespace aggx
{
	template<class Clip>
	class rasterizer_scanline_aa
	{
	public:
		typedef Clip clip_type;
		typedef typename Clip::conv_type conv_type;
		typedef typename Clip::coord_type coord_type;

		enum aa_scale_e
		{
			aa_shift = 8,
			aa_scale = 1 << aa_shift,
			aa_mask = aa_scale - 1,
			aa_scale2 = aa_scale * 2,
			aa_mask2 = aa_scale2 - 1
		};

		enum
		{
			thread_count = 1
		};

		rasterizer_scanline_aa() : 
			m_outline(),
			m_clipper(),
			m_filling_rule(fill_non_zero),
			m_auto_close(true),
			m_start_x(0),
			m_start_y(0),
			m_status(status_initial),
			m_parallel(thread_count)
		{
			for(int i = 0; i < aa_scale; i++)
				m_gamma[i] = i;
		}

		void reset(); 
		void reset_clipping();
		void clip_box(real x1, real y1, real x2, real y2);
		void filling_rule(filling_rule_e filling_rule);
		void auto_close(bool flag) { m_auto_close = flag; }

		template<class GammaF>
		void gamma(const GammaF& gamma_function)
		{ 
			int i;
			for(i = 0; i < aa_scale; i++)
			{
				m_gamma[i] = uround(gamma_function(real(i) / aa_mask) * aa_mask);
			}
		}

		unsigned apply_gamma(unsigned cover) const 
		{ 
			return m_gamma[cover]; 
		}

		void move_to(int x, int y);
		void line_to(int x, int y);
		void move_to_d(real x, real y);
		void line_to_d(real x, real y);
		void close_polygon();
		void add_vertex(real x, real y, unsigned cmd);

		template<class VertexSource>
		void add_path(VertexSource& vs, unsigned path_id = 0);

		int min_x() const { return m_outline.hrange().first; }
		int min_y() const { return m_outline.vrange().first; }
		int max_x() const { return m_outline.hrange().second; }
		int max_y() const { return m_outline.vrange().second; }

		void prepare();

		template <typename ScanlineAdapter, typename Renderer>
		void render(Renderer &r);

	private:
		enum status
		{
			status_initial,
			status_move_to,
			status_line_to,
			status_closed
		};

		struct calculate_alpha;

	private:
		agge::vector_rasterizer m_outline;
		clip_type m_clipper;
		int m_gamma[aa_scale];
		filling_rule_e m_filling_rule;
		bool m_auto_close;
		coord_type m_start_x;
		coord_type m_start_y;
		unsigned m_status;
		agge::raw_memory_object m_cover_buffers[thread_count];
		agge::parallel m_parallel;
	};


	template<class Clip>
	struct rasterizer_scanline_aa<Clip>::calculate_alpha
	{
		unsigned int operator ()(int area) const;
	};



	template<class Clip>
	inline void rasterizer_scanline_aa<Clip>::reset() 
	{
		m_outline.reset(); 
		m_status = status_initial;
	}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::reset_clipping()
	{
		reset();
		m_clipper.reset_clipping();
	}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::clip_box(real x1, real y1, real x2, real y2)
	{
		reset();
		m_clipper.clip_box(conv_type::upscale(x1), conv_type::upscale(y1),
			conv_type::upscale(x2), conv_type::upscale(y2));
	}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::move_to(int x, int y)
	{
		if (m_outline.sorted())
			reset();
		if (m_auto_close)
			close_polygon();
		m_clipper.move_to(m_start_x = conv_type::downscale(x), m_start_y = conv_type::downscale(y));
		m_status = status_move_to;
	}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::line_to(int x, int y)
	{
		m_clipper.line_to(m_outline, conv_type::downscale(x), conv_type::downscale(y));
		m_status = status_line_to;
	}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::move_to_d(real x, real y) 
	{ 
		if(m_outline.sorted())
			reset();
		if(m_auto_close)
			close_polygon();
		m_clipper.move_to(m_start_x = conv_type::upscale(x), m_start_y = conv_type::upscale(y)); 
		m_status = status_move_to;
	}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::line_to_d(real x, real y) 
	{ 
		m_clipper.line_to(m_outline, conv_type::upscale(x), conv_type::upscale(y)); 
		m_status = status_line_to;
	}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::close_polygon()
	{
		if(m_status == status_line_to)
		{
			m_clipper.line_to(m_outline, m_start_x, m_start_y);
			m_status = status_closed;
		}
	}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::add_vertex(real x, real y, unsigned cmd)
	{
		if (is_move_to(cmd)) 
			move_to_d(x, y);
		else if (is_vertex(cmd))
			line_to_d(x, y);
		else if(is_close(cmd))
			close_polygon();
	}

	template<class Clip>
	template<class VertexSource>
	inline void rasterizer_scanline_aa<Clip>::add_path(VertexSource& vs, unsigned path_id)
	{
		real x;
		real y;

		unsigned cmd;
		vs.rewind(path_id);
		if(m_outline.sorted()) reset();
		while (!is_stop(cmd = vs.vertex(&x, &y)))
			add_vertex(x, y, cmd);
	}

	//template<class Clip>
	//inline unsigned rasterizer_scanline_aa<Clip>::calculate_alpha(int area) const
	//{
	//	int cover = area >> (poly_subpixel_shift*2 + 1 - aa_shift);

	//	if(cover < 0) cover = -cover;
	//	if(m_filling_rule == fill_even_odd)
	//	{
	//		cover &= aa_mask2;
	//		if(cover > aa_scale)
	//		{
	//			cover = aa_scale2 - cover;
	//		}
	//	}
	//	if(cover > aa_mask) cover = aa_mask;
	//	return m_gamma[cover];
	//}

	template<class Clip> 
	inline void rasterizer_scanline_aa<Clip>::prepare()
	{
		if (m_auto_close)
			close_polygon();
		m_outline.sort();
	}

	template<class Clip>
	template <typename ScanlineAdapter, typename Renderer>
	inline void rasterizer_scanline_aa<Clip>::render(Renderer &r)
	{
		prepare();
		m_parallel.call([this, &r] (unsigned int threadid) {
			Renderer r_thread(r);
			ScanlineAdapter sl(r_thread, m_cover_buffers[threadid], this->max_x() - this->min_x() + 1);

			agge::render(sl, m_outline, rasterizer_scanline_aa<Clip>::calculate_alpha(), threadid, thread_count);
		});
		//ScanlineAdapter sl(r, m_cover_buffers[0], min_x(), max_x());

		//agge::render(sl, m_outline, rasterizer_scanline_aa<Clip>::calculate_alpha(), 0, 1);
	}


	template<class Clip>
	inline unsigned int rasterizer_scanline_aa<Clip>::calculate_alpha::operator ()(int area) const
	{
		area >>= agge::vector_rasterizer::_1_shift + 1;
		if (area < 0)
			area = -area;
		if (area > 255)
			area = 255;
		return area;
	}
}
