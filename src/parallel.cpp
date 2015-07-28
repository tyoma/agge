#include <agge/parallel.h>

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
			enum { max_spin = 20000 };

		public:
			hybrid_event()
				: _native(::CreateSemaphore(0, 0, 1, 0), &::CloseHandle), _lock_state(0)
			{	}

			void set()
			{
				if (_InterlockedCompareExchange(&_lock_state, 1 /*flag if...*/, 0 /*... was not locked*/) == 2 /*was locked*/)
					::ReleaseSemaphore(_native.get(), 1, 0);
			}

			void wait()
			{
				for (bool ready = false; !ready; )
				{
					for (count_t i = max_spin; !ready && i; --i)
					{
						ready = !!InterlockedExchange(&_lock_state, 0);
						if (!ready)
							_mm_pause();
					}
					if (!ready && _InterlockedCompareExchange(&_lock_state, 2 /*lock if...*/, 0 /*... was not flagged*/) == 0 /*was not flagged*/)
						::WaitForSingleObject(_native.get(), INFINITE);
				}
			}

		private:
			shared_ptr<void> _native;
			volatile long _lock_state;
		};
	}

	struct parallel::thread
	{
	public:
		explicit thread(count_t id);
		~thread();

		hybrid_event ready;
		hybrid_event done;

	public:
		const parallel::kernel_function *kernel;

	private:
		thread(const thread &other);
		const thread &operator =(const thread &rhs);

		static unsigned int __stdcall thread_proc(void *data);

	private:
		const count_t _id;
		const shared_ptr<void> _handle;
	};



	parallel::thread::thread(count_t id)
		: kernel(0), _id(id), _handle((HANDLE)_beginthreadex(0, 0, &thread::thread_proc, this, 0, 0), &::CloseHandle)
	{	}

	parallel::thread::~thread()
	{
		kernel = 0;
		ready.set();
		::WaitForSingleObject(_handle.get(), INFINITE);
	}

	unsigned int parallel::thread::thread_proc(void *data)
	{
		thread *this_ = reinterpret_cast<thread *>(data);

		for (; this_->ready.wait(), this_->kernel; this_->kernel = 0, this_->done.set())
			(*this_->kernel)(this_->_id);
		return 0;
	}


	parallel::parallel(count_t parallelism)
	{
		for (count_t i = 1; i < parallelism; ++i)
			_threads.push_back(thread_ptr(new thread(i)));
	}

	parallel::~parallel()
	{	}

	void parallel::call(const kernel_function &kernel)
	{
		for (threads::iterator i = _threads.begin(); i != _threads.end(); ++i)
		{
			(*i)->kernel = &kernel;
			(*i)->ready.set();
		}
		kernel(0);
		for (threads::iterator i = _threads.begin(); i != _threads.end(); ++i)
			(*i)->done.wait();
	}
}
