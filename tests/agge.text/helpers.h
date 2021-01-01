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
		struct plural_
		{
			template <typename T>
			std::vector<T> operator +(const T &rhs) const
			{	return std::vector<T>(1, rhs);	}
		} const plural;

		template <typename T>
		inline std::vector<T> operator +(std::vector<T> lhs, const T &rhs)
		{	return lhs.push_back(rhs), lhs;	}


		template <typename Iterator>
		inline std::vector<typename std::iterator_traits<Iterator>::value_type> mkvector(Iterator begin, Iterator end)
		{	return std::vector<typename std::iterator_traits<Iterator>::value_type>(begin, end);	}

		template <typename Iterator>
		inline pod_vector<typename std::iterator_traits<Iterator>::value_type> mkpodvector(Iterator begin, Iterator end)
		{
			agge::pod_vector<typename std::iterator_traits<Iterator>::value_type> v;

			for (; begin != end; ++begin)
				v.push_back(*begin);
			return v;
		}

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

	inline bool operator ==(const box_r &lhs, const box_r &rhs)
	{	return tests::equal(lhs.w, rhs.w) && tests::equal(lhs.h, rhs.h);	}

	inline bool operator ==(const glyph::path_point &lhs, const glyph::path_point &rhs)
	{	return lhs.command == rhs.command && tests::equal(lhs.x, rhs.x) && tests::equal(lhs.y, rhs.y);	}

	inline bool operator ==(const font_metrics &lhs, const font_metrics &rhs)
	{
		return tests::equal(lhs.ascent, rhs.ascent) && tests::equal(lhs.descent, rhs.descent)
			&& tests::equal(lhs.leading, rhs.leading);
	}

	inline font_metrics operator *(double lhs, font_metrics rhs)
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

template <typename T1, typename T2>
inline bool operator ==(const std::vector<T1> &lhs, const std::vector<T2> &rhs)
{
	typename std::vector<T1>::const_iterator i = lhs.begin();
	typename std::vector<T2>::const_iterator j = rhs.begin();

	for (; i != lhs.end() && j != rhs.end(); ++i, ++j)
		if (!(*i == *j))
			return false;
	return i == lhs.end() && j == rhs.end();
}
