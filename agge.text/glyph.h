#pragma once

#include <agge/types.h>

namespace agge
{
	class glyph
	{
	public:
		class path_iterator;

	public:
		virtual ~glyph() { }

		path_iterator get_path() const;

	public:
		uint16_t index;
		real_t advance_x;
		real_t advance_y;
	};

	class glyph::path_iterator
	{
	public:
		void rewind(int id);
		int vertex(real_t *x, real_t *y);
	};
}
