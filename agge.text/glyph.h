#pragma once

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
		real_t factor;
		glyph_metrics metrics;
		outline_ptr outline;
	};

	struct glyph::path_point
	{
		int command;
		real_t x, y;
	};

	class glyph::path_iterator
	{
	public:
		explicit path_iterator(const glyph::outline_ptr &outline, real_t factor);

		void rewind(int id);
		int vertex(real_t *x, real_t *y);

	private:
		glyph::outline_ptr _outline;
		real_t _factor;
		glyph::outline_storage::const_iterator _i;
	};



	inline glyph::path_iterator glyph::get_outline() const
	{	return path_iterator(outline, factor);	}


	inline glyph::path_iterator::path_iterator(const glyph::outline_ptr &outline, real_t factor)
		: _outline(outline), _factor(factor), _i(_outline->begin())
	{	}

	inline void glyph::path_iterator::rewind(int /*id*/)
	{	_i = _outline->begin();	}

	inline int glyph::path_iterator::vertex(real_t *x, real_t *y)
	{
		if (_i == _outline->end())
			return path_command_stop;
		*x = _i->x * _factor, *y = _i->y * _factor;
		return _i++->command;
	}
}
