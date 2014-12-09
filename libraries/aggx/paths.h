#pragma once

#include "basics.h"

namespace aggx
{
	template <class T>
	class poly_plain_adaptor
	{
	public:
		typedef T value_type;

	public:
		poly_plain_adaptor()
			: m_data(0), m_ptr(0), m_end(0), m_closed(false), m_stop(false)
		{	}

		poly_plain_adaptor(const T* data, unsigned num_points, bool closed)
			: m_data(data), m_ptr(data), m_end(data + num_points * 2), m_closed(closed), m_stop(false)
		{	}

		void init(const T* data, unsigned num_points, bool closed)
		{
			m_data = data;
			m_ptr = data;
			m_end = data + num_points * 2;
			m_closed = closed;
			m_stop = false;
		}

		void rewind(unsigned)
		{
			m_ptr = m_data;
			m_stop = false;
		}

		unsigned vertex(real* x, real* y)
		{
			if(m_ptr < m_end)
			{
				bool first = m_ptr == m_data;
				*x = *m_ptr++;
				*y = *m_ptr++;
				return first ? path_cmd_move_to : path_cmd_line_to;
			}
			*x = *y = 0.0;
			if(m_closed && !m_stop)
			{
				m_stop = true;
				return path_cmd_end_poly | path_flags_close;
			}
			return path_cmd_stop;
		}

	private:
		const T* m_data;
		const T* m_ptr;
		const T* m_end;
		bool     m_closed;
		bool     m_stop;
	};

	class line_adaptor
	{
	public:
		typedef real value_type;

	public:
		line_adaptor()
			: m_line(m_coord, 2, false)
		{	}

		line_adaptor(real x1, real y1, real x2, real y2)
			: m_line(m_coord, 2, false)
		{
			m_coord[0] = x1;
			m_coord[1] = y1;
			m_coord[2] = x2;
			m_coord[3] = y2;
		}

		void init(real x1, real y1, real x2, real y2)
		{
			m_coord[0] = x1;
			m_coord[1] = y1;
			m_coord[2] = x2;
			m_coord[3] = y2;
			m_line.rewind(0);
		}

		void rewind(unsigned)
		{
			m_line.rewind(0);
		}

		unsigned vertex(real* x, real* y)
		{
			return m_line.vertex(x, y);
		}

	private:
		real                     m_coord[4];
		poly_plain_adaptor<real> m_line;
	};
}
