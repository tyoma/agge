#pragma once

namespace agge
{
	template <typename CoordT>
	struct point;

	template <typename CoordT>
	struct rect;



	template <typename CoordT>
	inline point<CoordT> create_point(CoordT x, CoordT y)
	{
		point<CoordT> p = { x, y };
		return p;
	}

	template <typename CoordT>
	inline CoordT width(const rect<CoordT> &rc)
	{	return rc.x2 - rc.x1;	}

	template <typename CoordT>
	inline CoordT height(const rect<CoordT> &rc)
	{	return rc.y2 - rc.y1;	}

	template <typename T>
	inline T agge_min(const T &lhs, const T &rhs)
	{	return lhs < rhs ? lhs : rhs;	}

	template <typename T>
	inline T agge_max(const T &lhs, const T &rhs)
	{	return lhs > rhs ? lhs : rhs;	}

	inline int muldiv(int a, int b, int c)
	{	return a * b / c;	}

	inline float muldiv(float /*a*/, float /*b*/, float /*c*/)
	{	return 0.0f;	}
}
