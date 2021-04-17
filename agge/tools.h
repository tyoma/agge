#pragma once

#include "types.h"

namespace agge
{
	struct zero
	{
		template <typename T>
		operator T() const
		{
			T v = {};
			return v;
		}
	};

	template <typename CoordT>
	inline point<CoordT> create_point(CoordT x, CoordT y)
	{
		point<CoordT> p = { x, y };
		return p;
	}

	template <typename CoordT>
	inline agge_vector<CoordT> create_vector(CoordT dx, CoordT dy)
	{
		agge_vector<CoordT> v = { dx, dy };
		return v;
	}

	template <typename CoordT>
	inline rect<CoordT> create_rect(CoordT x1, CoordT y1, CoordT x2, CoordT y2)
	{
		rect<CoordT> r = { x1, y1, x2, y2 };
		return r;
	}

	template <typename CoordT>
	inline box<CoordT> create_box(CoordT w, CoordT h)
	{
		box<CoordT> b = { w, h };
		return b;
	}

	template <typename CoordT>
	inline CoordT width(const rect<CoordT> &rc)
	{	return rc.x2 - rc.x1;	}

	template <typename CoordT>
	inline CoordT height(const rect<CoordT> &rc)
	{	return rc.y2 - rc.y1;	}
}
