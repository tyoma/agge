#pragma once

#include "glyph.h"
#include "hash_map.h"
#include "shared_ptr.h"

namespace agge
{
	class font : noncopyable
	{
	public:
		struct accessor;
		struct key;
		typedef shared_ptr<accessor> accessor_ptr;
		typedef shared_ptr<font> ptr;

		struct metrics
		{
			real_t ascent;
			real_t descent;
			real_t leading;
		};

	public:
		explicit font(const accessor_ptr &accessor_, real_t factor = 1.0f);

		metrics get_metrics() const;

		uint16_t map_single(wchar_t character) const;
		const glyph *get_glyph(uint16_t index) const;

	private:
		typedef hash_map<uint16_t, glyph> glyphs_cache_t;
		typedef hash_map<wchar_t, uint16_t> char2index_cache_t;

	private:
		const accessor_ptr _accessor;
		metrics _metrics;
		mutable glyphs_cache_t _glyphs;
		mutable char2index_cache_t _char2glyph;
		real_t _factor;
	};

	struct font::key
	{
		enum grid_fit { gf_none = 0, gf_vertical = 1, gf_strong = 2 };

		explicit key(const std::wstring &typeface = std::wstring(), unsigned height = 0, bool bold = false,
			bool italic = false, grid_fit grid_fit = gf_none);

		std::wstring typeface;
		unsigned height : 20;
		unsigned bold : 1;
		unsigned italic : 1;
		grid_fit grid_fit_ : 3;
	};

	struct font::accessor
	{
		virtual ~accessor() { }
		virtual font::metrics get_metrics() const = 0;
		virtual uint16_t get_glyph_index(wchar_t character) const = 0;
		virtual glyph::outline_ptr load_glyph(uint16_t index, glyph::glyph_metrics &m) const = 0;
	};

	bool operator ==(const font::key &lhs, const font::key &rhs);
}
