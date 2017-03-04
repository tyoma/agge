#pragma once

#include <agge/path.h>
#include <agge/pod_vector.h>
#include <memory>
#include <unordered_map>

namespace std { namespace tr1 {} using namespace tr1; }

namespace agge
{
	class glyph;

	class font : noncopyable
	{
	public:
		typedef std::shared_ptr<font> ptr;

		struct metrics
		{
			real_t ascent;
			real_t descent;
			real_t leading;
		};

	public:
		virtual ~font();

		metrics get_metrics() const;

		uint16_t map_single(wchar_t character) const;
		const glyph *get_glyph(uint16_t index) const;

	protected:
		struct kerning_pair;
		typedef std::unordered_map<uint16_t, const glyph *> glyphs_cache_t;
		typedef std::unordered_map<wchar_t, uint16_t> char2index_cache_t;

	protected:
		font(const metrics &metrics_);

	private:
		virtual uint16_t get_glyph_index(wchar_t character) const = 0;
		virtual const glyph *load_glyph(uint16_t index) const = 0;
		virtual pod_vector<kerning_pair> load_kerning() const = 0;

	private:
		const metrics _metrics;
		mutable glyphs_cache_t _glyphs;
		mutable char2index_cache_t _char2glyph;
	};

	class glyph
	{
	public:
		struct path_point { int command; real_t x, y; };
		class path_iterator;
		typedef pod_vector<path_point> outline_storage;
		typedef std::shared_ptr<outline_storage> outline_ptr; 

	public:
		virtual ~glyph() { }

		path_iterator get_outline() const;

	public:
		real_t advance_x;
		real_t advance_y;
		outline_ptr outline;
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
