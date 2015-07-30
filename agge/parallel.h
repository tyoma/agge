#pragma once

#include "memory.h"

#include <functional>

#if defined(_MSC_VER) && _MSC_VER==1500
	namespace std
	{
		using tr1::function;
	}
#endif

namespace agge
{
	class parallel
	{
	public:
		typedef std::function<void(count_t threadid)> kernel_function;

	public:
		explicit parallel(count_t parallelism);
		~parallel();

		void call(const kernel_function &kernel);

	private:
		struct thread;

	private:
		parallel(const parallel &other);
		const parallel &operator =(const parallel &rhs);

		void destroy_threads();

	private:
		raw_memory_object _threads;
		count_t _thread_allocated;
	};
}
