#pragma once

#include <agge.text/font.h>
#include <agge.text/layout.h>
#include <iterator>
#include <tests/common/helpers.h>
#include <vector>

namespace agge
{
	namespace tests
	{
		template <typename Iterator>
		inline std::vector<typename std::iterator_traits<Iterator>::value_type> mkvector(Iterator begin, Iterator end)
		{	return std::vector<typename std::iterator_traits<Iterator>::value_type>(begin, end);	}

		inline glyph::path_point mkppoint(int command, real_t x, real_t y)
		{
			glyph::path_point p = { command, x, y };
			return p;
		}

		template <typename IteratorT>
		pod_vector<glyph::path_point> convert(IteratorT &i)
		{
			real_t x, y;
			pod_vector<glyph::path_point> result;

			for (int command; command = i.vertex(&x, &y), path_command_stop != command; )
				result.push_back(mkppoint(command, x, y));
			return result;
		}

		template <typename IteratorT>
		pod_vector<glyph::path_point> convert_copy(IteratorT i)
		{	return convert(i);	}
	}

	inline bool operator ==(const vector_r &lhs, const vector_r &rhs)
	{	return tests::equal(lhs.dx, rhs.dx) && tests::equal(lhs.dy, rhs.dy);	}

	inline bool operator ==(const layout::positioned_glyph &lhs, const layout::positioned_glyph &rhs)
	{	return lhs.d == rhs.d && lhs.index == rhs.index;	}

	inline bool operator ==(glyph_index_t lhs, const layout::positioned_glyph &rhs)
	{	return lhs == rhs.index;	}

	inline bool operator ==(const glyph::path_point &lhs, const glyph::path_point &rhs)
	{	return lhs.command == rhs.command && tests::equal(lhs.x, rhs.x) && tests::equal(lhs.y, rhs.y);	}

	inline bool operator ==(const font::metrics &lhs, const font::metrics &rhs)
	{
		return tests::equal(lhs.ascent, rhs.ascent) && tests::equal(lhs.descent, rhs.descent)
			&& tests::equal(lhs.leading, rhs.leading);
	}

	inline font::metrics operator *(double lhs, font::metrics rhs)
	{
		rhs.ascent *= static_cast<real_t>(lhs);
		rhs.descent *= static_cast<real_t>(lhs);
		rhs.leading *= static_cast<real_t>(lhs);
		return rhs;
	}

	inline glyph::path_point operator *(double lhs, glyph::path_point rhs)
	{
		rhs.x *= static_cast<real_t>(lhs);
		rhs.y *= static_cast<real_t>(lhs);
		return rhs;
	}

	template <typename T>
	inline T operator *(double lhs, T rhs)
	{
		for (typename T::iterator i = rhs.begin(); i != rhs.end(); ++i)
			*i = lhs * *i;
		return rhs;
	}
}
