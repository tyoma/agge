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
	class parallel : noncopyable
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
		void destroy_threads();

	private:
		raw_memory_object _threads;
		count_t _thread_allocated;
	};
}
