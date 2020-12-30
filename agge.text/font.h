#pragma once

#include "glyph.h"
#include "hash_map.h"
#include "shared_ptr.h"
#include "types.h"

#include <string>

namespace agge
{
	class font : noncopyable
	{
	public:
		struct accessor;
		typedef shared_ptr<accessor> accessor_ptr;
		typedef shared_ptr<font> ptr;

	public:
		explicit font(const font_descriptor &key_, const accessor_ptr &accessor_, real_t factor = 1.0f);

		font_descriptor get_key() const;
		font_metrics get_metrics() const;

		glyph_index_t map_single(wchar_t character) const;
		const glyph *get_glyph(glyph_index_t index) const;

	private:
		typedef hash_map<glyph_index_t, glyph> glyphs_cache_t;
		typedef hash_map<wchar_t, glyph_index_t> char2index_cache_t;

	private:
		glyph_index_t load_mapping(wchar_t character) const;
		const glyph* load_glyph(glyph_index_t index) const;

	private:
		const accessor_ptr _accessor;
		const font_descriptor _key;
		font_metrics _metrics;
		mutable glyphs_cache_t _glyphs;
		mutable char2index_cache_t _char2glyph;
		real_t _factor;
	};

	struct font::accessor
	{
		virtual ~accessor() { }
		virtual font_metrics get_metrics() const = 0;
		virtual glyph_index_t get_glyph_index(wchar_t character) const = 0;
		virtual glyph::outline_ptr load_glyph(glyph_index_t index, glyph::glyph_metrics &m) const = 0;
	};
}
