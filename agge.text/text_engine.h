#pragma once

#include "layout.h"
#include "text_engine_base.h"

#include <agge/config.h>
#include <agge/math.h>
#include <limits>

namespace agge
{
	template <typename RasterizerT>
	class text_engine : public text_engine_base
	{
	public:
		explicit text_engine(loader &loader_, uint8_t precision = 4);

		void render_glyph(RasterizerT &target, const font &font_, glyph_index_t glyph_index, real_t x, real_t y);
		void render_string(RasterizerT &target, const font &font_, const std::string &text, text_alignment halign,
			real_t x, real_t y, real_t max_width = (std::numeric_limits<real_t>::max)());
		void render(RasterizerT &target, const glyph_run &glyphs, point_r reference);

		template <typename ContainerT>
		void render(RasterizerT &target, const ContainerT &container, const point_r &reference);

		template <typename LayoutT>
		void render(RasterizerT &target, const LayoutT &layout_, text_alignment halign, text_alignment valign,
			const rect_r &reference);

		template <typename LimitProcessorT>
		void render(RasterizerT &target, const richtext_t &text, text_alignment halign, text_alignment valign,
			const rect_r &reference, const LimitProcessorT &limit_processor);

		template <typename LimitProcessorT>
		agge::box_r measure(const richtext_t &text, const LimitProcessorT &limit_processor);

	private:
		typedef hash_map<int, RasterizerT> rasters_map;
		typedef hash_map<const font *, rasters_map> font_rasters_map;

	private:
		rasters_map &get_rasters_map(const font &font_);
		void render_glyph(RasterizerT &target, const font &font_, rasters_map &rasters, glyph_index_t glyph_index,
			real_t x, real_t y);
		void load_glyph_precise(const font &font_, glyph_index_t glyph_index, unsigned int precise_glyph_index,
			int xf, int yf, rasters_map &rasters, typename rasters_map::iterator &glyph_iterator);
		virtual void on_before_removed(font *font_) throw();

	private:
		font_rasters_map _cached_fonts;
		const int _precision, _factor;
		const real_t _rfactor;
		mutable layout _worker_layout;
	};



	template <typename RasterizerT>
	inline text_engine<RasterizerT>::text_engine(loader &loader_, uint8_t precision)
		: text_engine_base(loader_), _precision(precision), _factor(1 << precision), _rfactor(1.0f / _factor)
	{	}

	template <typename RasterizerT>
	inline void text_engine<RasterizerT>::render_glyph(RasterizerT &target, const font &font_, glyph_index_t glyph_index,
		real_t x, real_t y)
	{	render_glyph(target, font_, get_rasters_map(font_), glyph_index, x, y);	}

	template <typename RasterizerT>
	inline void text_engine<RasterizerT>::render(RasterizerT &target, const glyph_run &gr, point_r ref)
	{
		const font &font_ = *gr.font_.get();
		rasters_map &rasters = get_rasters_map(font_);

		for (positioned_glyphs_container_t::const_iterator g = gr.begin(), end = gr.end(); g != end; ref += g->d, ++g)
			render_glyph(target, font_, rasters, g->index, ref.x, ref.y);
	}

	template <typename RasterizerT>
	template <typename ContainerT>
	inline void text_engine<RasterizerT>::render(RasterizerT &target, const ContainerT &container, const point_r &ref)
	{
		for (typename ContainerT::const_iterator i = container.begin(), end = container.end(); i != end; ++i)
			render(target, *i, ref + i->offset);
	}

	template <typename RasterizerT>
	template <typename LayoutT>
	inline void text_engine<RasterizerT>::render(RasterizerT &target, const LayoutT &layout_, text_alignment halign,
		text_alignment valign, const rect_r &ref)
	{
		point_r running_ref = create_point(0.0f, align_near == valign ? ref.y1 : align_far == valign ? ref.y2 - layout_.get_box().h
			: 0.5f * (ref.y1 + ref.y2 - layout_.get_box().h));

		for (typename LayoutT::const_iterator i = layout_.begin(), end = layout_.end(); i != end; ++i)
		{
			running_ref.x = align_near == halign ? ref.x1 : align_far == halign ? ref.x2 - i->extent
				: 0.5f * (ref.x1 + ref.x2 - i->extent);
			render(target, *i, running_ref + i->offset);
		}
	}

	template <typename RasterizerT>
	template <typename LimitProcessorT>
	inline void text_engine<RasterizerT>::render(RasterizerT &target, const richtext_t &text, text_alignment halign,
		text_alignment valign, const rect_r &reference, const LimitProcessorT &limit_processor)
	{
		_worker_layout.process(text, limit_processor, *this);
		render(target, _worker_layout, halign, valign, reference);
	}

	template <typename RasterizerT>
	template <typename LimitProcessorT>
	inline agge::box_r text_engine<RasterizerT>::measure(const richtext_t &text, const LimitProcessorT &limit_processor)
	{
		_worker_layout.process(text, limit_processor, *this);
		return _worker_layout.get_box();
	}

	template <typename RasterizerT>
	inline void text_engine<RasterizerT>::render_string(RasterizerT &target, const font &font_, const std::string &text,
		text_alignment halign, real_t x, real_t y, real_t max_width)
	{
		std::string::const_iterator end = text.end();
		rasters_map &rasters = get_rasters_map(font_);
		real_t dx = 0.0f;

		for (std::string::const_iterator i = text.begin(); i != end; ++i)
		{
			const real_t dx2 = dx + font_.get_glyph_for_codepoint(*i)->metrics.advance_x;

			if (dx2 > max_width)
			{
				end = i;
				break;
			}
			dx = dx2;
		}
		if (align_near != halign)
			x -= align_center == halign ? 0.5f * dx : dx;

		for (std::string::const_iterator i = text.begin(); i != end; ++i)
		{
			const glyph *g = font_.get_glyph_for_codepoint(*i);

			render_glyph(target, font_, rasters, g->index, x, y);
			x += g->metrics.advance_x, y += g->metrics.advance_y;
		}
	}

	template <typename RasterizerT>
	AGGE_INLINE typename text_engine<RasterizerT>::rasters_map &text_engine<RasterizerT>::get_rasters_map(const font &font_)
	{
		typename font_rasters_map::iterator ri = _cached_fonts.find(&font_);

		return _cached_fonts.end() != ri
			? ri->second
			: _cached_fonts.insert(make_pair(&font_, rasters_map())).first->second;
	}

	template <typename RasterizerT>
	AGGE_INLINE void text_engine<RasterizerT>::render_glyph(RasterizerT &target, const font &font_, rasters_map &rasters,
		glyph_index_t glyph_index, real_t x, real_t y)
	{
		const int xi = static_cast<int>(x * _factor) >> _precision, yi = static_cast<int>(y * _factor) >> _precision;
		const int xf = static_cast<int>((x - xi) * _factor), yf = static_cast<int>((y - yi) * _factor);
		const int precise_glyph_index = (glyph_index << (2 * _precision)) + (yf << _precision) + xf;
		typename rasters_map::iterator i = rasters.find(precise_glyph_index);

		if (rasters.end() == i)
			load_glyph_precise(font_, glyph_index, precise_glyph_index, xf, yf, rasters, i);
		target.append(i->second, xi, yi);
	}

	template <typename RasterizerT>
	inline void text_engine<RasterizerT>::load_glyph_precise(const font &font_, glyph_index_t glyph_index,
		unsigned int precise_glyph_index, int xf, int yf, rasters_map &rasters,
		typename rasters_map::iterator &glyph_iterator)
	{
		const glyph *g = font_.get_glyph(glyph_index);
		offset_conv converted_pi(g->get_outline(), _rfactor * xf, _rfactor * yf);

		glyph_iterator = rasters.insert(make_pair(precise_glyph_index, RasterizerT())).first;
		add_path(glyph_iterator->second, converted_pi);
		glyph_iterator->second.compact();
	}

	template <typename RasterizerT>
	inline void text_engine<RasterizerT>::on_before_removed(font *font_) throw()
	{	_cached_fonts.erase(font_);	}
}
