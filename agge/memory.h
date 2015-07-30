#pragma once

#include "types.h"

namespace agge
{
	class raw_memory_object
	{
	public:
		raw_memory_object();
		~raw_memory_object();

		template <typename T>
		T *get(count_t size);

	private:
		raw_memory_object(const raw_memory_object &other);
		const raw_memory_object &operator =(const raw_memory_object &rhs);

	private:
		uint8_t *_buffer;
		count_t _size;
	};



	inline raw_memory_object::raw_memory_object()
		: _buffer(0), _size(0)
	{	}

	inline raw_memory_object::~raw_memory_object()
	{	delete []_buffer;	}

	template <typename T>
	inline T *raw_memory_object::get(count_t size)
	{
		size *= sizeof(T);
		size /= sizeof(uint8_t);
		if (size > _size)
		{
			uint8_t *buffer = new uint8_t[size];

			delete []_buffer;
			_buffer = buffer;
			_size = size;
			while (size--)
				*buffer++ = 0;
		}
		return reinterpret_cast<T *>(_buffer);
	}
}
