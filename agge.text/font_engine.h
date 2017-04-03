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

	template <typename RasterizerT>
	class font_engine : public font_engine_base
	{
	public:
		font_engine(loader &loader_);

		void render_glyph(RasterizerT &target, const font &font_, uint16_t glyph_index, real_t x, real_t y);

	private:
		typedef hash_map<uint16_t, RasterizerT> rasters_map;

	private:
		rasters_map _glyph_rasters;
	};

	struct font_engine_base::loader
	{
		virtual font::accessor_ptr load(const wchar_t *typeface, int height, bool bold, bool italic,
			font_engine_base::grid_fit grid_fit) = 0;
	};



	template <typename RasterizerT>
	inline font_engine<RasterizerT>::font_engine(loader &loader_)
		: font_engine_base(loader_)
	{	}

	template <typename RasterizerT>
	inline void font_engine<RasterizerT>::render_glyph(RasterizerT &target, const font &font_, uint16_t glyph_index,
		real_t x, real_t y)
	{
		typename rasters_map::iterator i = _glyph_rasters.find(glyph_index);

		if (_glyph_rasters.end() == i)
		{
			const glyph *g = font_.get_glyph(glyph_index);
			glyph::path_iterator pi = g->get_outline();

			_glyph_rasters.insert(glyph_index, RasterizerT(), i);
			add_path(i->second, pi);
			i->second.sort();
		}
		target.append(i->second, (int)x, (int)y);
	}
}
