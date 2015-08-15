#pragma once

#include "tools.h"
#include "types.h"

namespace agge
{
	enum clipping_flags {
		x1_clipped_shift = 2,
		x2_clipped_shift = 0,
		y1_clipped_shift = 3,
		y2_clipped_shift = 1,

		x1_clipped = 1 << x1_clipped_shift,
		x2_clipped = 1 << x2_clipped_shift,
		y1_clipped = 1 << y1_clipped_shift,
		y2_clipped = 1 << y2_clipped_shift,
		x_clipped = x1_clipped | x2_clipped,
		y_clipped = y1_clipped | y2_clipped
	};


	template <typename T>
	class clipper
	{
	public:
		clipper();

		void reset();		
		void set(const rect<T> &window);
		void move_to(T x, T y);

		template <typename RasterizerT>
		void line_to(RasterizerT &rasterizer, T x, T y);

	private:
		template <typename RasterizerT>
		void line_clip_y(RasterizerT &rasterizer, T x1, T y1, T x2, T y2) const;

	private:
		rect<T> _window;
		T _x1, _y1;
		int _f1;
		bool _enabled;
	};


	
	template <typename T>
	inline int clipping_y(T y, const rect<T> &window)
	{	return ((y < window.y1) << y1_clipped_shift) | ((y > window.y2) << y2_clipped_shift);	}

	template <typename T>
	inline int clipping(T x, T y, const rect<T> &window)
	{	return ((x < window.x1) << x1_clipped_shift) | ((x > window.x2) << x2_clipped_shift) | clipping_y(y, window);	}


	template <typename T>
	inline clipper<T>::clipper()
		: _enabled(false)
	{	}

	template <typename T>
	inline void clipper<T>::set(const rect<T> &window)
	{
		_window = window;
		_enabled = true;
	}

	template <typename T>
	inline void clipper<T>::move_to(T x, T y)
	{
		_x1 = x;
		_y1 = y;
		_f1 = clipping(x, y, _window);
	}

	template <typename T>
	template <typename RasterizerT>
	inline void clipper<T>::line_to(RasterizerT &rasterizer, T x2, T y2)
	{
		if (_enabled)
		{
			int f2 = clipping(x2, y2, _window);

			T x1 = _x1;
			T y1 = _y1;
			T y3, y4;
			int f1 = _f1;

			switch(((f1 & x_clipped) << 1) | (f2 & x_clipped))
			{
			case 0:
				line_clip_y(rasterizer, x1, y1, x2, y2);
				break;

			case (x2_clipped << 1) | x2_clipped:
				line_clip_y(rasterizer, _window.x2, y1, _window.x2, y2/*, f1, f2*/);
				break;

			case (x2_clipped << 1) | x1_clipped:
				y3 = y1 + muldiv(_window.x2 - x1, y2 - y1, x2 - x1);
				y4 = y1 + muldiv(_window.x1 - x1, y2 - y1, x2 - x1);
//				f3 = clipping_y(y3, _window);
//				f4 = clipping_y(y4, _window);
				line_clip_y(rasterizer, _window.x2, y1, _window.x2, y3/*, f1, f3*/);
				line_clip_y(rasterizer, _window.x2, y3, _window.x1, y4/*, f3, f4*/);
				line_clip_y(rasterizer, _window.x1, y4, _window.x1, y2/*, f4, f2*/);
				break;

			case (x1_clipped << 1) | x2_clipped:
				y3 = y1 + muldiv(_window.x1 - x1, y2 - y1, x2 - x1);
				y4 = y1 + muldiv(_window.x2 - x1, y2 - y1, x2 - x1);
//				f3 = clipping_y(y3, _window);
//				f4 = clipping_y(y4, _window);
//				line_clip_y(rasterizer, _window.x1, y1, _window.x1, y3/*, f1, f3*/);
				line_clip_y(rasterizer, _window.x1, y3, _window.x2, y4/*, f3, f4*/);
//				line_clip_y(rasterizer, _window.x2, y4, _window.x2, y2/*, f4, f2*/);
				break;

			default:
				throw 0;
			}
			_f1 = f2;
		}
		else
		{
			rasterizer.line(_x1, _y1, x2, y2);
		}
		_x1 = x2;
		_y1 = y2;
	}

	template <typename T>
	template <typename RasterizerT>
	inline void clipper<T>::line_clip_y(RasterizerT &rasterizer, T x1, T y1, T x2, T y2) const
	{
		rasterizer.line(x1, y1, x2, y2);
	}

}
