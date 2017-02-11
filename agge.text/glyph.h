#pragma once

#include <agge/types.h>

namespace agge
{
	class glyph
	{
	public:
		class path_iterator;

	public:
		path_iterator get_path() const;

	public:
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
