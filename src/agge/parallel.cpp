#include <agge/parallel.h>

#include <agge/config.h>
#include <memory>

#if defined(AGGE_PLATFORM_WINDOWS)
	#include "platform/win32/parallel-inline.h"
#elif defined(AGGE_PLATFORM_LINUX)
	#include "platform/linux/parallel-inline.h"
#elif defined(AGGE_PLATFORM_APPLE)
	#include "platform/apple/parallel-inline.h"
#endif

namespace agge
{
	parallel::parallel(count_t parallelism)
	try
		: _threads_allocated(0)
	{
		thread *p = _threads.get<thread>(parallelism - 1);

		for (count_t i = 1; i != parallelism; ++i, ++_threads_allocated, ++p)
			new (p) thread(i);
	}
	catch (...)
	{
		destroy_threads();
		throw;
	}

	parallel::~parallel()
	{	destroy_threads();	}

	void parallel::call(kernel_function &kernel)
	{
		thread * const threads = _threads.get<thread>(0);

		for (count_t i = 0; i != _threads_allocated; ++i)
		{
			threads[i].kernel = &kernel;
			threads[i].ready.set();
		}
		kernel(0);
		for (count_t i = 0; i != _threads_allocated; ++i)
			threads[i].done.wait();
	}

	void parallel::destroy_threads()
	{
		thread * const threads = _threads.get<thread>(0);

		for (count_t i = 0; i != _threads_allocated; ++i)
			threads[i].~thread();
	}
}
