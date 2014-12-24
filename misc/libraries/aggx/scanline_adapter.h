#pragma once

#include "basics.h"

#include <vector>

namespace aggx
{
	template <typename Renderer>
	class scanline_adapter
	{
	public:
		typedef std::vector<cover_type> covers_array;
	public:
		scanline_adapter(Renderer &renderer, covers_array &covers, int min_x, int max_x)
			: m_renderer(renderer), m_covers(covers)
		{
			m_covers.resize(max_x - min_x + 16);
			reset_spans();
		}

		void add_cell(int x, unsigned cover)
		{
			if (x != m_length + m_start_x)
				commit(x);
			*m_cover++ = cover;
			++m_length;
		}

		void add_span(int x, unsigned len, unsigned cover)
		{
			if (x != m_length + m_start_x)
				commit(x);
			std::fill_n(m_cover, len, cover);
			m_length += len;
			m_cover += len;
		}

		void commit(int start_x = 0x7FFFFFF0)
		{
			cover_type *start = &m_covers[4];

			*reinterpret_cast<int*>(m_cover) = 0;
			m_renderer(m_start_x, m_current_y, m_length, start);

			m_start_x = start_x;
			m_length = 0;
			m_cover = start;
		}

		void reset_spans()
		{
			m_length = 0;
			m_start_x = 0x7FFFFFF0;
			m_cover = &m_covers[4];
		}

		void set_y(unsigned y)
		{
			m_current_y = y;
		}

	private:
		Renderer &m_renderer;
		int m_current_y, m_start_x, m_length;
		covers_array &m_covers;
		cover_type *m_cover;
	};
}
