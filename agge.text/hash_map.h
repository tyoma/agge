#pragma once

#include <unordered_map>

namespace std { namespace tr1 {} using namespace tr1; }

namespace agge
{
	struct knuth_hash
	{
		size_t operator ()(int key) const { return key * 2654435761; }
	};

	template <typename KeyT, typename ValueT>
	class hash_map : private std::unordered_map<KeyT, ValueT, knuth_hash>
	{
		typedef std::unordered_map<KeyT, ValueT, knuth_hash> base_t;

	public:
		using typename base_t::const_iterator;
		using typename base_t::iterator;

	public:
		using base_t::find;
		using base_t::begin;
		using base_t::end;

		bool insert(const KeyT &key, const ValueT &value, iterator &i);
	};



	template <typename KeyT, typename ValueT>
	inline bool hash_map<KeyT, ValueT>::insert(const KeyT &key, const ValueT &value, iterator &i)
	{
		const std::pair<iterator, bool> r = base_t::insert(typename base_t::value_type(key, value));

		i = r.first;
		return r.second;
	}
}
