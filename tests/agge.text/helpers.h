#pragma once

#include <agge.text/font.h>
#include <agge.text/layout.h>
#include <iterator>
#include <vector>

namespace agge
{
	namespace tests
	{
		template <typename T>
		inline bool equal(const T &lhs, const T &rhs)
		{	return lhs == rhs;	}

		template <>
		bool equal(const real_t &lhs, const real_t &rhs);

		template <typename Iterator>
		inline std::vector<typename std::iterator_traits<Iterator>::value_type> mkvector(Iterator begin, Iterator end)
		{	return std::vector<typename std::iterator_traits<Iterator>::value_type>(begin, end);	}

		template <typename T>
		inline point<T> mkpoint(T x, T y)
		{
			point<T> p = { x, y };
			return p;
		}
	}

	inline bool operator ==(const layout::positioned_glyph &lhs, const layout::positioned_glyph &rhs)
	{	return tests::equal(lhs.dx, rhs.dx) && tests::equal(lhs.dy, rhs.dy) && lhs.index == rhs.index;	}

	inline bool operator ==(const point_r &lhs, const point_r &rhs)
	{	return tests::equal(lhs.x, rhs.x) && tests::equal(lhs.y, rhs.y);	}

	inline bool operator ==(const glyph::path_point &lhs, const glyph::path_point &rhs)
	{	return lhs.command == rhs.command && tests::equal(lhs.x, rhs.x) && tests::equal(lhs.y, rhs.y);	}
}
