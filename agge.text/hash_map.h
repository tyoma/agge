#pragma once

#include <unordered_map>

namespace agge
{
	struct knuth_hash
	{
		std::size_t operator ()(unsigned int key) const { return key * 2654435761u; }
		std::size_t operator ()(const void* key) const { return (*this)(static_cast<unsigned int>(reinterpret_cast<std::size_t>(key))); }
	};

	template <typename KeyT, typename ValueT, typename HashF = knuth_hash>
	class hash_map : private std::unordered_map<KeyT, ValueT, HashF>
	{
		typedef std::unordered_map<KeyT, ValueT, HashF> base_t;

	public:
		using typename base_t::const_iterator;
		using typename base_t::iterator;
		using typename base_t::value_type;

	public:
		using base_t::find;
		using base_t::begin;
		using base_t::end;
		using base_t::erase;
		using base_t::insert;

		bool insert(const KeyT &key, const ValueT &value, iterator &i);
	};



	template <typename KeyT, typename ValueT, typename HashF>
	inline bool hash_map<KeyT, ValueT, HashF>::insert(const KeyT &key, const ValueT &value, iterator &i)
	{
		const std::pair<iterator, bool> r = base_t::insert(typename base_t::value_type(key, value));

		i = r.first;
		return r.second;
	}
}
