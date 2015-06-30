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
			unsigned int size = max_x - min_x + 16;

			if (m_covers.size() < size)
				m_covers.resize(size);
		}

		void add_cell(int x, cover_type cover)
		{
			if (x != m_next_x)
				commit(x);
			*m_cover++ = cover;
			++m_next_x;
		}

		void add_span(int x, unsigned len, cover_type cover)
		{
			if (x != m_next_x)
				commit(x);
			std::fill_n(m_cover, len, cover);
			m_cover += len;
			m_next_x += len;
		}

		void commit(int start_x = 0x7FFFFFF0)
		{
			cover_type *start = &m_covers[4];

			*reinterpret_cast<int*>(m_cover) = 0;
			m_renderer(m_start_x, m_current_y, m_next_x - m_start_x, start);
			
			m_next_x = m_start_x = start_x;
			m_cover = start;
		}

		void begin(int y)
		{
			m_next_x = m_start_x = 0x7FFFFFF0;
			m_cover = &m_covers[4];
			m_current_y = y;
		}

	private:
		Renderer &m_renderer;
		int m_current_y, m_start_x, m_next_x;
		covers_array &m_covers;
		cover_type *m_cover;
	};
}
