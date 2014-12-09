#pragma once

#include "agg_basics.h"

#include <vector>

namespace agg2
{
	class scanline_u8
	{
	public:
		typedef scanline_u8 self_type;
		typedef agg::int8u cover_type;
		typedef agg::int16 coord_type;

		struct span
		{
			coord_type  x;
			coord_type  len;
			cover_type* covers;
		};

		typedef const span* const_iterator;

		scanline_u8()
			: m_min_x(0), m_last_x(0x7FFFFFF0), m_cur_span(0)
		{ }

		void reset(int min_x, int max_x)
		{
			unsigned max_len = max_x - min_x + 2;
			
			m_spans.resize(max_len);
			m_covers.resize(max_len);

			m_last_x = 0x7FFFFFF0;
			m_min_x = min_x;
			m_cur_span = &m_spans[0];
		}

		void add_cell(int x, unsigned cover)
		{
			x -= m_min_x;
			m_covers[x] = (cover_type)cover;
			if (x == m_last_x + 1)
			{
				m_cur_span->len++;
			}
			else
			{
				m_cur_span++;
				m_cur_span->x = (coord_type)(x + m_min_x);
				m_cur_span->len = 1;
				m_cur_span->covers = &m_covers[x];
			}
			m_last_x = x;
		}

		void add_span(int x, unsigned len, unsigned cover)
		{
			x -= m_min_x;
			std::fill_n(m_covers.begin() + x, len, cover);
			if (x == m_last_x + 1)
			{
				m_cur_span->len += (coord_type)len;
			}
			else
			{
				m_cur_span++;
				m_cur_span->x = (coord_type)(x + m_min_x);
				m_cur_span->len = (coord_type)len;
				m_cur_span->covers = &m_covers[x];
			}
			m_last_x = x + len - 1;
		}

		void reset_spans()
		{
			m_last_x = 0x7FFFFFF0;
			m_cur_span = &m_spans[0];
		}

		unsigned num_spans() const { return unsigned(m_cur_span - &m_spans[0]); }
		const_iterator begin() const { return &m_spans[1]; }

	private:
		int m_min_x;
		int m_last_x;
		std::vector<cover_type> m_covers;
		std::vector<span> m_spans;
		span* m_cur_span;
	};
}
