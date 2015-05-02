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
			: _native(::CreateEvent(nullptr, FALSE, FALSE, nullptr), &::CloseHandle)
		{	}

		event(const event &other)
			: _native(::CreateEvent(nullptr, FALSE, FALSE, nullptr), &::CloseHandle)
		{	}

		void set()
		{	::SetEvent(_native.get());	}

		void wait()
		{	::WaitForSingleObject(_native.get(), INFINITE);	}

	private:
		shared_ptr<void> _native;
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
