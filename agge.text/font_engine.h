#pragma once

#include "font.h"

namespace agge
{
	class font_engine : noncopyable
	{
	public:
		struct loader;
		enum grid_fit { gf_none = 0, gf_vertical = 1, gf_strong = 2 };

	public:
		explicit font_engine(loader &loader_);

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

	struct font_engine::loader
	{
		virtual font::accessor_ptr load(const wchar_t *typeface, int height, bool bold, bool italic,
			font_engine::grid_fit grid_fit) = 0;
	};
}
