#pragma once

namespace agge
{
	template <typename T>
	T agge_min(const T &lhs, const T &rhs)
	{	return lhs < rhs ? lhs : rhs;	}
}
