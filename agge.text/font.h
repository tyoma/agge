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

		struct key
		{
			enum grid_fit { gf_none = 0, gf_vertical = 1, gf_strong = 2 };

			explicit key(const std::wstring &typeface, int height, bool bold = false, bool italic = false,
				grid_fit grid_fit = gf_none);

			std::wstring typeface;
			int height : 20;
			unsigned bold : 1;
			unsigned italic : 1;
			grid_fit grid_fit_ : 2;
		};

		struct metrics
		{
			real_t ascent;
			real_t descent;
			real_t leading;
		};

	public:
		explicit font(const key &key_, const accessor_ptr &accessor_, real_t factor = 1.0f);

		key get_key() const;
		metrics get_metrics() const;

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
		const key _key;
		metrics _metrics;
		mutable glyphs_cache_t _glyphs;
		mutable char2index_cache_t _char2glyph;
		real_t _factor;
	};

	struct font::accessor
	{
		virtual ~accessor() { }
		virtual font::metrics get_metrics() const = 0;
		virtual glyph_index_t get_glyph_index(wchar_t character) const = 0;
		virtual glyph::outline_ptr load_glyph(glyph_index_t index, glyph::glyph_metrics &m) const = 0;
	};

	bool operator ==(const font::key &lhs, const font::key &rhs);
}
