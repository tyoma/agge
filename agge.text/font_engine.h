#pragma once

#include "font.h"

namespace agge
{
	class font_engine_base : noncopyable
	{
	public:
		struct loader;
		enum grid_fit { gf_none = 0, gf_vertical = 1, gf_strong = 2 };

	public:
		explicit font_engine_base(loader &loader_);

		font::ptr create_font(const wchar_t *typeface, int height, bool bold, bool italic, grid_fit gf);

	protected:
		class offset_conv;

	private:
		struct font_key;
		struct font_key_hasher;
		typedef hash_map<font_key, font::ptr, font_key_hasher> fonts_cache;
		typedef hash_map<font_key, font::accessor_ptr, font_key_hasher> scalabale_fonts_cache;

	private:
		loader &_loader;
		shared_ptr<fonts_cache> _fonts;
		shared_ptr<scalabale_fonts_cache> _scalable_fonts;
	};

	class font_engine_base::offset_conv
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
	class font_engine : public font_engine_base
	{
	public:
		explicit font_engine(loader &loader_, uint8_t precision = 4);

		void render_glyph(RasterizerT &target, const font &font_, uint16_t glyph_index, real_t x, real_t y);

	private:
		typedef hash_map<count_t, RasterizerT> rasters_map;

	private:
		rasters_map _glyph_rasters;
		const int _precision, _factor;
		const real_t _rfactor;
	};

	struct font_engine_base::loader
	{
		virtual font::accessor_ptr load(const wchar_t *typeface, int height, bool bold, bool italic,
			font_engine_base::grid_fit grid_fit) = 0;
	};



	template <typename RasterizerT>
	inline font_engine<RasterizerT>::font_engine(loader &loader_, uint8_t precision)
		: font_engine_base(loader_), _precision(precision), _factor(1 << precision), _rfactor(1.0f / _factor)
	{	}

	template <typename RasterizerT>
	inline void font_engine<RasterizerT>::render_glyph(RasterizerT &target, const font &font_, uint16_t glyph_index,
		real_t x, real_t y)
	{
		const int xi = static_cast<int>(x * _factor) >> _precision, yi = static_cast<int>(y * _factor) >> _precision;
		const int xf = static_cast<int>((x - xi) * _factor), yf = static_cast<int>((y - yi) * _factor);
		const int precise_glyph_index = (glyph_index << (2 * _precision)) + (yf << _precision) + xf;
		typename rasters_map::iterator i = _glyph_rasters.find(precise_glyph_index);

		if (_glyph_rasters.end() == i)
		{
			const glyph *g = font_.get_glyph(glyph_index);
			offset_conv converted_pi(g->get_outline(), _rfactor * xf, _rfactor * yf);

			_glyph_rasters.insert(precise_glyph_index, RasterizerT(), i);
			add_path(i->second, converted_pi);
			i->second.sort();
		}
		target.append(i->second, xi, yi);
	}
}
