#pragma once

#include "agg_basics.h"

namespace agg2
{
	template <class Blender, class RenditionBuffer>
	class pixmap
	{
	public:
		typedef Blender blender_type;
		typedef typename blender_type::color_type color_type;
		typedef typename blender_type::order_type order_type;
		typedef typename color_type::value_type value_type;
		typedef typename color_type::calc_type calc_type;
		typedef RenditionBuffer rendition_buffer_type;
		typedef typename rendition_buffer_type::row_data row_data;

	public:
		explicit pixmap(rendition_buffer_type& buffer);

		unsigned width() const;
		unsigned height() const;

		void copy_hline(int x, int y, unsigned len, const color_type& c);
		void blend_hline(int x, int y, unsigned len, const color_type& c, agg::int8u cover);
		void blend_solid_hspan(int x, int y, unsigned len, const color_type& c, const agg::int8u* covers);

	private:
		enum base_scale_e 
		{
			base_shift = color_type::base_shift,
			base_scale = color_type::base_scale,
			base_mask  = color_type::base_mask,
			pix_width  = sizeof(value_type) * 3
		};

	private:
		blender_type m_blender;
		rendition_buffer_type& m_buffer;
	};



	template <class Blender, class RenditionBuffer>
	inline pixmap<Blender, RenditionBuffer>::pixmap(rendition_buffer_type& buffer)
		: m_buffer(buffer)
	{
	}

	template <class Blender, class RenditionBuffer>
	inline unsigned pixmap<Blender, RenditionBuffer>::width()  const
	{
		return m_buffer.width();
	}

	template <class Blender, class RenditionBuffer>
	inline unsigned pixmap<Blender, RenditionBuffer>::height() const
	{
		return m_buffer.height();
	}

	template <class Blender, class RenditionBuffer>
	inline void pixmap<Blender, RenditionBuffer>::copy_hline(int x, int y, unsigned len, const color_type& c)
	{
		value_type* p = (value_type*)m_buffer.row_ptr(x, y, len) + x + x + x;
		do
		{
			p[order_type::R] = c.r; 
			p[order_type::G] = c.g; 
			p[order_type::B] = c.b;
			p += 3;
		}
		while(--len);
	}

	template <class Blender, class RenditionBuffer>
	inline void pixmap<Blender, RenditionBuffer>::blend_hline(int x, int y, unsigned len, const color_type& c, agg::int8u cover)
	{
	}

	template <class Blender, class RenditionBuffer>
	inline void pixmap<Blender, RenditionBuffer>::blend_solid_hspan(int x, int y, unsigned len, const color_type& c, const agg::int8u* covers)
	{
		if (c.a)
		{
			value_type* p = (value_type*)
				m_buffer.row_ptr(x, y, len) + x + x + x;

			do 
			{
				calc_type alpha = (calc_type(c.a) * (calc_type(*covers) + 1)) >> 8;
				if(alpha == base_mask)
				{
					p[order_type::R] = c.r;
					p[order_type::G] = c.g;
					p[order_type::B] = c.b;
				}
				else
				{
					m_blender.blend_pix(p, c.r, c.g, c.b, alpha, *covers);
				}
				p += 3;
				++covers;
			}
			while(--len);
		}
	}
}
