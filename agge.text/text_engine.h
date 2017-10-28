#pragma once

#include "font.h"
#include "layout.h"

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
		void render_layout(RasterizerT &target, const layout &layout_, real_t x, real_t y);

	private:
		typedef hash_map<int, RasterizerT> rasters_map;
		typedef hash_map<const font *, rasters_map> font_rasters_map;

	private:
		virtual void on_before_removed(font * font_) throw();

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

		const int xi = static_cast<int>(x * _factor) >> _precision, yi = static_cast<int>(y * _factor) >> _precision;
		const int xf = static_cast<int>((x - xi) * _factor), yf = static_cast<int>((y - yi) * _factor);
		const int precise_glyph_index = (glyph_index << (2 * _precision)) + (yf << _precision) + xf;
		typename rasters_map::iterator i = rasters->second.find(precise_glyph_index);

		if (rasters->second.end() == i)
		{
			const glyph *g = font_.get_glyph(glyph_index);
			offset_conv converted_pi(g->get_outline(), _rfactor * xf, _rfactor * yf);

			rasters->second.insert(precise_glyph_index, RasterizerT(), i);
			add_path(i->second, converted_pi);
			i->second.sort();
		}
		target.append(i->second, xi, yi);
	}

	template <typename RasterizerT>
	inline void text_engine<RasterizerT>::render_layout(RasterizerT &target, const layout &layout_, real_t x, real_t y)
	{
		for (layout::const_iterator gr = layout_.begin(); gr != layout_.end(); ++gr)
		{
			real_t rx = x;
			const real_t ry = gr->reference.y + y;
			const font &f = *layout_.begin()->glyph_run_font;

			for (layout::positioned_glyphs_container::const_iterator g = gr->begin; g != gr->end; ++g)
			{
				rx += g->dx;
				render_glyph(target, f, g->index, rx, ry);
			}
		}
	}

	template <typename RasterizerT>
	inline void text_engine<RasterizerT>::on_before_removed(font * font_) throw()
	{	_cached_fonts.erase(font_);	}
}
