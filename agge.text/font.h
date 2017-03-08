#pragma once

#include "hash_map.h"
#include "shared_ptr.h"

#include <agge/path.h>
#include <agge/pod_vector.h>

namespace agge
{
	class glyph
	{
	public:
		struct glyph_metrics
		{
			real_t advance_x, advance_y;
		};

		struct path_point;
		class path_iterator;
		typedef pod_vector<path_point> outline_storage;
		typedef shared_ptr<outline_storage> outline_ptr;

	public:
		path_iterator get_outline() const;

	public:
		glyph_metrics metrics;
		outline_ptr outline;
	};

	class font : noncopyable
	{
	public:
		struct accessor;
		typedef shared_ptr<accessor> accessor_ptr;
		typedef shared_ptr<font> ptr;

		struct metrics
		{
			real_t ascent;
			real_t descent;
			real_t leading;
		};

	public:
		font(const accessor_ptr &accessor_);

		metrics get_metrics() const;

		uint16_t map_single(wchar_t character) const;
		const glyph *get_glyph(uint16_t index) const;

	private:
		typedef hash_map<uint16_t, glyph> glyphs_cache_t;
		typedef hash_map<wchar_t, uint16_t> char2index_cache_t;

	private:
		const accessor_ptr _accessor;
		const metrics _metrics;
		mutable glyphs_cache_t _glyphs;
		mutable char2index_cache_t _char2glyph;
	};



	struct font::accessor
	{
		virtual ~accessor() { }
		virtual font::metrics get_metrics() const = 0;
		virtual uint16_t get_glyph_index(wchar_t character) const = 0;
		virtual glyph::outline_ptr load_glyph(uint16_t index, glyph::glyph_metrics &m) const = 0;
	};

	struct glyph::path_point
	{
		int command;
		real_t x, y;
	};

	class glyph::path_iterator
	{
	public:
		explicit path_iterator(const glyph::outline_ptr &outline);

		void rewind(int id);
		int vertex(real_t *x, real_t *y);

	private:
		glyph::outline_ptr _outline;
		glyph::outline_storage::const_iterator _i;
	};



	inline glyph::path_iterator glyph::get_outline() const
	{	return path_iterator(outline);	}


	inline glyph::path_iterator::path_iterator(const glyph::outline_ptr &outline)
		: _outline(outline), _i(_outline->begin())
	{	}

	inline void glyph::path_iterator::rewind(int /*id*/)
	{	_i = _outline->begin();	}

	inline int glyph::path_iterator::vertex(real_t *x, real_t *y)
	{
		if (_i == _outline->end())
			return path_command_stop;
		*x = _i->x, *y = _i->y;
		return _i++->command;
	}
}
