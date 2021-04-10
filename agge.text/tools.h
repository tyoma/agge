#pragma once

namespace agge
{
	inline bool is_space(char c)
	{	return c == ' ';	}

	template <typename IteratorT>
	inline void eat_spaces(IteratorT &i, IteratorT end)
	{
		while (i != end && is_space(*i))
			++i;
	}

	template <typename IteratorT>
	inline bool eat_lf(IteratorT &i)
	{	return *i == '\n' ? ++i, true : false;	}
}
