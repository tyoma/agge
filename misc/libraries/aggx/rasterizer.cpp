#include "rasterizer.h"

#include <process.h>
#include <windows.h>

using namespace std;

namespace aggx
{
	struct parallel::worker_data
	{
		parallel * const self;
		const parallel::kernel_func *kernel;
		const size_t threadid;
	};



	parallel::parallel(size_t n)
		: _exit(false),
			_run(::CreateSemaphore(nullptr, 0, n - 1, nullptr), &::CloseHandle)
	{
		_threads.reserve(n - 1);
		while (--n)
		{
			worker_data d = { this, nullptr, n };

			_threads.push_back(make_pair(shared_ptr<void>(), d));
			_threads.back().first = shared_ptr<void>(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0,
				&thread_proc, &_threads.back().second, 0, nullptr)), &::CloseHandle);
//			::SetThreadPriority(_threads.back().first.get(), THREAD_PRIORITY_HIGHEST);
		}
		::Sleep(500);
	}

	parallel::~parallel()
	{
		_exit = true;
		::ReleaseSemaphore(_run.get(), _threads.size(), nullptr);
		for (threads::const_iterator i = _threads.begin(); i != _threads.end(); ++i)
			::WaitForSingleObject(i->first.get(), INFINITE);
	}

	void parallel::call(const std::function<void(size_t threadid)> &kernel)
	{
		_semaphore = _threads.size();
		for (threads::iterator i = _threads.begin(); i != _threads.end(); ++i)
			i->second.kernel = &kernel;
		::ReleaseSemaphore(_run.get(), _semaphore, nullptr);
		kernel(0);
		while (_semaphore)
		{	}
	}

	unsigned int __stdcall parallel::thread_proc(void *self_)
	{
		worker_data *data = static_cast<worker_data *>(self_);

		while (::WaitForSingleObject(data->self->_run.get(), INFINITE) == WAIT_OBJECT_0 && !data->self->_exit)
		{
			(*data->kernel)(data->threadid);
			::InterlockedDecrement(&data->self->_semaphore);
		}
		return 0;
	}
}
