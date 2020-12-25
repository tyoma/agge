#pragma once

#include <algorithm>
#include <agge/pixel.h>
#include <agge/vector_rasterizer.h>

namespace agge
{
	namespace tests
	{
		template <typename T>
		inline point<T> mkpoint(T x, T y)
		{
			point<T> p = { x, y };
			return p;
		}

		template <typename T>
		inline bool equal(const T &lhs, const T &rhs)
		{	return lhs == rhs;	}
		
		template <>
		inline bool equal(const real_t &lhs, const real_t &rhs)
		{
			if (lhs == rhs)
				return true;

			const real_t tolerance = 2e-5f;
			const real_t d = (lhs - rhs) / (lhs + rhs);
			
			return -tolerance <= d && d <= tolerance;
		}

		template <typename T>
		inline typename T::const_iterator begin(const T &container)
		{	return container.begin();	}

		template <typename T>
		inline typename T::const_iterator end(const T &container)
		{	return container.end();	}

		template <typename T, size_t n>
		inline T *begin(T (&p)[n])
		{	return p;	}

		template <typename T, size_t n>
		inline T *end(T (&p)[n])
		{	return p + n;	}

		template <typename T, size_t n>
		size_t size(T (&)[n])
		{	return n;	}
	}
	
	inline bool operator ==(const vector_rasterizer::cell &lhs, const vector_rasterizer::cell &rhs)
	{	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.area == rhs.area && lhs.cover == rhs.cover;	}

	template <typename T1, typename T2>
	inline bool operator ==(const pod_vector<T1> &lhs, const pod_vector<T2> &rhs)
	{	return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());	}

	inline bool operator ==(const pixel32 &lhs, const pixel32 &rhs)
	{	return std::equal(tests::begin(lhs.components), tests::end(lhs.components), rhs.components);	}

	template <typename T>
	inline bool operator ==(const point<T> &lhs, const point<T> &rhs)
	{	return tests::equal(lhs.x, rhs.x) && tests::equal(lhs.y, rhs.y);	}

	template <typename T>
	inline bool operator ==(const agge_vector<T> &lhs, const agge_vector<T> &rhs)
	{	return tests::equal(lhs.dx, rhs.dx) && tests::equal(lhs.dy, rhs.dy);	}
}
