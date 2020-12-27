#pragma once

#include "font.h"
#include "layout.h"

#include <agge/config.h>
#include <agge/math.h>
#include <limits>

namespace agge
{
	class text_engine_base : noncopyable
	{
	public:
		struct loader;

	public:
		explicit text_engine_base(loader &loader_, unsigned collection_cycles = 5);
		virtual ~text_engine_base();

		void collect();
		font::ptr create_font(const wchar_t *typeface, int height, bool bold, bool italic, font::key::grid_fit grid_fit);

	protected:
		class offset_conv;

	private:
		class cached_outline_accessor;

		struct font_key_hasher
		{
			size_t operator ()(const font::key &key) const;
		};

		typedef hash_map<font::key, weak_ptr<font>, font_key_hasher> fonts_cache;
		typedef hash_map<font::key, weak_ptr<font::accessor>, font_key_hasher> scalabale_fonts_cache;
		typedef hash_map<font::key, std::pair<font*, unsigned /*age*/>, font_key_hasher> garbage_container;

	private:
		std::pair<font::accessor_ptr, real_t> create_font_accessor(font::key fk);
		void on_released(const fonts_cache::value_type *entry, font *font_);
		void destroy(font *font_) throw();

		virtual void on_before_removed(font * /*font_*/) throw() {	}

	private:
		loader &_loader;
		const unsigned _collection_cycles;
		fonts_cache _fonts;
		scalabale_fonts_cache _scalable_fonts;
		garbage_container _garbage;
	};

	struct text_engine_base::loader
	{
		virtual font::accessor_ptr load(const wchar_t *typeface, int height, bool bold, bool italic,
			font::key::grid_fit grid_fit) = 0;
	};

	class text_engine_base::offset_conv
	{
	public:
		offset_conv(const glyph::path_iterator &source, real_t dx, real_t dy);

		void rewind(unsigned id);
		int vertex(real_t *x, real_t *y);

	private:
		glyph::path_iterator _source;
		real_t _dx, _dy;
	};


	template <typename RasterizerT>
	class text_engine : public text_engine_base
	{
	public:
		explicit text_engine(loader &loader_, uint8_t precision = 4);

		void render_glyph(RasterizerT &target, const font &font_, uint16_t glyph_index, real_t x, real_t y);
		template <typename IteratorT>
		void render_glyph_run(RasterizerT &target, const font &font_, IteratorT glyph_begin, IteratorT glyph_end,
			real_t x, real_t y);
		void render_layout(RasterizerT &target, const layout &layout_, real_t x, real_t y);
		void render_string(RasterizerT &target, const font &font_, const wchar_t *text, layout::halign halign,
			real_t x, real_t y, real_t max_width = (std::numeric_limits<real_t>::max)());

	private:
		typedef hash_map<int, RasterizerT> rasters_map;
		typedef hash_map<const font *, rasters_map> font_rasters_map;

	private:
		void render_glyph(RasterizerT &target, const font &font_, rasters_map &rasters, uint16_t glyph_index,
			real_t x, real_t y);
		void load_glyph_precise(const font &font_, uint16_t glyph_index, unsigned int precise_glyph_index, int xf, int yf,
			rasters_map &rasters, typename rasters_map::iterator &glyph_iterator);
		virtual void on_before_removed(font *font_) throw();

	private:
		font_rasters_map _cached_fonts;
		const int _precision, _factor;
		const real_t _rfactor;
	};



	template <typename RasterizerT>
	inline text_engine<RasterizerT>::text_engine(loader &loader_, uint8_t precision)
		: text_engine_base(loader_), _precision(precision), _factor(1 << precision), _rfactor(1.0f / _factor)
	{	}

	template <typename RasterizerT>
	inline void text_engine<RasterizerT>::render_glyph(RasterizerT &target, const font &font_, uint16_t glyph_index,
		real_t x, real_t y)
	{
		typename font_rasters_map::iterator rasters = _cached_fonts.find(&font_);

		if (_cached_fonts.end() == rasters)
			_cached_fonts.insert(&font_, rasters_map(), rasters);
		render_glyph(target, font_, rasters->second, glyph_index, x, y);
	}

	template <typename RasterizerT>
	template <typename IteratorT>
	inline void text_engine<RasterizerT>::render_glyph_run(RasterizerT &target, const font &font_,
		IteratorT glyph_begin, IteratorT glyph_end, real_t x, real_t y)
	{
		point_r ref = { x, y };
		typename font_rasters_map::iterator ri = _cached_fonts.find(&font_);

		if (_cached_fonts.end() == ri)
			_cached_fonts.insert(&font_, rasters_map(), ri);

		rasters_map &rasters = ri->second;

		for (layout::positioned_glyphs_container::const_iterator g = glyph_begin; g != glyph_end; ref += g->d, ++g)
			render_glyph(target, font_, rasters, g->index, ref.x, ref.y);
	}

	template <typename RasterizerT>
	inline void text_engine<RasterizerT>::render_layout(RasterizerT &target, const layout &layout_, real_t x, real_t y)
	{
		for (layout::const_iterator gr = layout_.begin(); gr != layout_.end(); ++gr)
			render_glyph_run(target, *gr->glyph_run_font, gr->begin(), gr->end(), x, gr->reference.y + y);
	}

	template <typename RasterizerT>
	inline void text_engine<RasterizerT>::render_string(RasterizerT &target, const font &font_, const wchar_t *text,
		layout::halign halign, real_t x, real_t y, real_t max_width)
	{
		typename font_rasters_map::iterator ri = _cached_fonts.find(&font_);

		if (_cached_fonts.end() == ri)
			_cached_fonts.insert(&font_, rasters_map(), ri);

		rasters_map &rasters = ri->second;
		real_t dx = 0.0f;

		if (layout::near_ != halign)
		{
			for (const wchar_t *c = text; *c; ++c)
			{
				const real_t dx2 = dx + font_.get_glyph(font_.map_single(*c))->metrics.advance_x;

				if (dx2 > max_width)
					break;
				dx = dx2;
			}
			x -= layout::center == halign ? 0.5f * dx : dx;
		}

		for (const wchar_t *c = text; *c; ++c)
		{
			const uint16_t index = font_.map_single(*c);
			const glyph *g = font_.get_glyph(index);

			if ((max_width -= g->metrics.advance_x) < 0.0f)
				break;
			render_glyph(target, font_, rasters, index, x, y);
			x += g->metrics.advance_x, y += g->metrics.advance_y;
		}
	}

	template <typename RasterizerT>
	AGGE_INLINE void text_engine<RasterizerT>::render_glyph(RasterizerT &target, const font &font_, rasters_map &rasters,
		uint16_t glyph_index, real_t x, real_t y)
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
	inline void text_engine<RasterizerT>::load_glyph_precise(const font &font_, uint16_t glyph_index,
		unsigned int precise_glyph_index, int xf, int yf, rasters_map &rasters,
		typename rasters_map::iterator &glyph_iterator)
	{
		const glyph *g = font_.get_glyph(glyph_index);
		offset_conv converted_pi(g->get_outline(), _rfactor * xf, _rfactor * yf);

		rasters.insert(precise_glyph_index, RasterizerT(), glyph_iterator);
		add_path(glyph_iterator->second, converted_pi);
		glyph_iterator->second.compact();
	}

	template <typename RasterizerT>
	inline void text_engine<RasterizerT>::on_before_removed(font *font_) throw()
	{	_cached_fonts.erase(font_);	}
}
