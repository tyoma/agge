#pragma once

#include "memory.h"

namespace agge
{
	class parallel : noncopyable
	{
	public:
		struct kernel_function;

	public:
		explicit parallel(count_t parallelism);
		~parallel();

		void call(kernel_function &kernel);

	private:
		struct thread;

	private:
		void destroy_threads();

	private:
		raw_memory_object _threads;
		count_t _threads_allocated;
	};

	struct parallel::kernel_function
	{
		virtual void operator ()(count_t threadid) = 0;
	};
}
