#include <agge/parallel.h>

#include <agge/hybrid_event.h>
#include <agge/thread.h>

#include <memory>

#pragma warning(disable:4355)

namespace agge
{
	struct parallel::thread : noncopyable
	{
	public:
		explicit thread(count_t id);
		~thread();

		hybrid_event ready;
		hybrid_event done;

	public:
		parallel::kernel_function *kernel;

	private:
		static void thread_proc(void *self_);

	private:
		const count_t _id;
		const agge::thread _thread;
	};



	parallel::thread::thread(count_t id)
		: kernel(0), _id(id), _thread(&thread::thread_proc, this)
	{
	}

	parallel::thread::~thread()
	{
		kernel = 0;
		ready.signal();
	}

	void parallel::thread::thread_proc(void *self_)
	{
		thread *self = static_cast<thread *>(self_);

		for (; self->ready.wait(), self->kernel; self->kernel = 0, self->done.signal())
			(*self->kernel)(self->_id);
	}


	parallel::parallel(count_t parallelism)
		: _threads_allocated(0)
	{
		try
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
	}

	parallel::~parallel()
	{	destroy_threads();	}

	void parallel::call(kernel_function &kernel)
	{
		thread * const threads = _threads.get<thread>(0);

		for (count_t i = 0; i != _threads_allocated; ++i)
		{
			threads[i].kernel = &kernel;
			threads[i].ready.signal();
		}
		kernel(0);
		for (count_t i = 0; i != _threads_allocated; ++i)
			threads[i].done.wait();
	}

	void parallel::destroy_threads() throw()
	{
		thread * const threads = _threads.get<thread>(0);

		for (count_t i = 0; i != _threads_allocated; ++i)
			threads[i].~thread();
	}
}
