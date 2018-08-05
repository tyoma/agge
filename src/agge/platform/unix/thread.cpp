#include <agge/thread.h>

#include <new>
#include <pthread.h>

namespace agge
{
	thread::thread(thread_function_t thread_function, void *argument)
		: _thread(reinterpret_cast<void *>(pthread_t())), _thread_function(thread_function), _argument(argument)
	{
		struct starter
		{
			static void *thread_proc(void *self_)
			{
				static_cast<thread *>(self_)->_thread_function(static_cast<thread *>(self_)->_argument);
				return 0;
			}
		};

		if (::pthread_create(reinterpret_cast<pthread_t *>(&_thread), 0, &starter::thread_proc, this))
			throw std::bad_alloc();
	}

	thread::~thread()
	{	::pthread_join(reinterpret_cast<pthread_t>(_thread), 0);	}
}
