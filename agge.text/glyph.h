#pragma once

#include <agge/types.h>

namespace agge
{
	class glyph
	{
	public:
		struct iterator;

	public:
		iterator get_iterator() const;

	public:
		real_t advance_x;
		real_t advance_y;
	};

	class glyph::iterator
	{
	public:
		void rewind(int id);
		int vertex(real_t *x, real_t *y);
	};
}
