#include "parallel.h"

#include <process.h>
#include <windows.h>

using namespace std;

namespace aggx
{
	class parallel::event
	{
	public:
		event()
			: _native(::CreateSemaphore(nullptr, 0, 1, nullptr), &::CloseHandle), _lock_state(0)
		{	}

		event(const event &other)
			: _native(::CreateSemaphore(nullptr, 0, 1, nullptr), &::CloseHandle), _lock_state(0)
		{	}

		void set()
		{
			if (_InterlockedCompareExchange(&_lock_state, 1 /*flag if...*/, 0 /*... was not locked*/) == 2 /*was locked*/)
				::ReleaseSemaphore(_native.get(), 1, nullptr);
		}

		void wait()
		{
			for (bool ready = false; !ready; )
			{
				for (long i = 20000; !ready && i; --i)
				{
					ready = !!_InterlockedExchange(&_lock_state, 0);
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

	struct parallel::worker_data
	{
		const unsigned int threadid;
		const parallel::kernel_function *kernel;
		parallel::event run;
		parallel::event done;
	};

	unsigned int __stdcall thread_proc(void *data_)
	{
		parallel::worker_data *data = reinterpret_cast<parallel::worker_data *>(data_);

		for (; data->run.wait(), data->kernel; data->kernel = nullptr, data->done.set())
			(*data->kernel)(data->threadid);
	}

	parallel::parallel(unsigned int threads)
	{
		_threads.reserve(threads - 1);
		while (--threads)
		{
			worker_data wd = { threads, nullptr };

			_threads.push_back(make_pair(nullptr, wd));

			thread_data &td = _threads.back();

			td.first = shared_ptr<void>((HANDLE)_beginthreadex(nullptr, 0, &thread_proc, &td.second, 0, nullptr), &::CloseHandle);
			::SetThreadPriority(td.first.get(), THREAD_PRIORITY_HIGHEST);
		}
	}

	parallel::~parallel()
	{
		for (threads::iterator i = _threads.begin(); i != _threads.end(); ++i)
		{
			i->second.run.set();
			::WaitForSingleObject(i->first.get(), INFINITE);
		}
	}

	void parallel::call(const kernel_function &kernel)
	{
		for (threads::iterator i = _threads.begin(); i != _threads.end(); ++i)
		{
			i->second.kernel = &kernel;
			i->second.run.set();
		}
		kernel(0);
		for (threads::iterator i = _threads.begin(); i != _threads.end(); ++i)
			i->second.done.wait();
	}
}
