#include <agge/parallel.h>

#include <intrin.h>
#include <process.h>
#include <windows.h>

#pragma warning(disable: 4355)

using namespace std;

namespace agge
{
	namespace
	{
		class hybrid_event
		{
		public:
			enum { max_spin = 10000 };

		public:
			hybrid_event()
				: _native(::CreateSemaphore(0, 0, 1, 0)), _lock_state(0)
			{	}

			~hybrid_event()
			{	::CloseHandle(_native);	}

			void set()
			{
				if (_InterlockedCompareExchange(&_lock_state, 1 /*flag if...*/, 0 /*... was not locked*/) == 2 /*was locked*/)
					::ReleaseSemaphore(_native, 1, 0);
			}

			void wait()
			{
				for (bool ready = false; !ready; )
				{
					for (count_t i = max_spin; !ready && i; --i)
					{
						ready = !!_InterlockedExchange(&_lock_state, 0);
						if (!ready)
							_mm_pause();
					}
					if (!ready && _InterlockedCompareExchange(&_lock_state, 2 /*lock if...*/, 0 /*... was not flagged*/) == 0 /*was not flagged*/)
						::WaitForSingleObject(_native, INFINITE);
				}
			}

		private:
			hybrid_event(const hybrid_event &other);
			const hybrid_event &operator =(const hybrid_event &rhs);

		private:
			const HANDLE _native;
			volatile long _lock_state;
		};
	}

	struct parallel::thread : noncopyable
	{
	public:
		explicit thread(count_t id);
		~thread();

		hybrid_event ready;
		hybrid_event done;

	public:
		const parallel::kernel_function *kernel;

	private:
		static unsigned int __stdcall thread_proc(void *data);

	private:
		const count_t _id;
		const HANDLE _handle;
	};



	parallel::thread::thread(count_t id)
		: kernel(0), _id(id), _handle(reinterpret_cast<HANDLE>(_beginthreadex(0, 0, &thread::thread_proc, this, 0, 0)))
	{
		if (!_handle)
			throw bad_alloc();
	}

	parallel::thread::~thread()
	{
		kernel = 0;
		ready.set();
		::WaitForSingleObject(_handle, INFINITE);
		::CloseHandle(_handle);
	}

	unsigned int parallel::thread::thread_proc(void *data)
	{
		thread *this_ = static_cast<thread *>(data);

		for (; this_->ready.wait(), this_->kernel; this_->kernel = 0, this_->done.set())
			(*this_->kernel)(this_->_id);
		return 0;
	}


	parallel::parallel(count_t parallelism)
	try
		: _thread_allocated(0)
	{
		thread *p = _threads.get<thread>(parallelism - 1);

		for (count_t i = 1; i != parallelism; ++i, ++_thread_allocated, ++p)
			new (p) thread(i);
	}
	catch (...)
	{
		destroy_threads();
		throw;
	}

	parallel::~parallel()
	{	destroy_threads();	}

	void parallel::call(const kernel_function &kernel)
	{
		thread * const threads = _threads.get<thread>(0);

		for (count_t i = 0; i != _thread_allocated; ++i)
		{
			threads[i].kernel = &kernel;
			threads[i].ready.set();
		}
		kernel(0);
		for (count_t i = 0; i != _thread_allocated; ++i)
			threads[i].done.wait();
	}

	void parallel::destroy_threads()
	{
		thread * const threads = _threads.get<thread>(0);

		for (count_t i = 0; i != _thread_allocated; ++i)
			threads[i].~thread();
	}
}
