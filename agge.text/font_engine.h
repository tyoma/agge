#pragma once

#include "font.h"

namespace agge
{
	class font_engine : noncopyable
	{
	public:
		struct loader;

	public:
		explicit font_engine(loader &loader_);

		font::ptr create_font(const wchar_t *typeface, int height, bool bold, bool italic);
//		void render_glyph(my_rasterizer &r, const font &font_, uint16_t index, real_t x, real_t y);

	private:
		struct font_key;
		struct font_key_hasher;
		typedef hash_map<font_key, font::ptr, font_key_hasher> fonts_cache;

	private:
		shared_ptr<fonts_cache> _fonts;
	};

	struct font_engine::loader
	{
		virtual font::accessor_ptr load(const wchar_t *typeface, int height, bool bold, bool italic) = 0;
	};
}
