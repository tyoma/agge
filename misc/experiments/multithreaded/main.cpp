#include <Windows.h>
#include <process.h>
#include <functional>
#include <memory>
#include <vector>

using namespace std;

class parallel
{
public:
	typedef std::function<void(size_t threadid)> kernel_func;

public:
	parallel(size_t n);
	~parallel();

	void call(const kernel_func &kernel);

private:
	struct worker_data;
	typedef std::shared_ptr<void> handle;
	typedef std::pair<handle, worker_data> thread_data;
	typedef std::vector<thread_data> threads;

private:
	static unsigned int __stdcall thread_proc(void *exit);
	static void __stdcall worker(ULONG_PTR data);

private:
	handle _exit;
	handle _controller_thread;
	threads _threads;
	volatile long _semaphore;
};


struct parallel::worker_data
{
	const parallel::kernel_func *kernel;
	const size_t threadid;
	volatile long * const semaphore;
	void * const controller_thread;
};



parallel::parallel(size_t n)
	: _exit(::CreateEvent(nullptr, TRUE, FALSE, nullptr), &::CloseHandle)		
{
	HANDLE x;

	::DuplicateHandle(::GetCurrentProcess(), ::GetCurrentThread(), ::GetCurrentProcess(), &x, 0, FALSE, DUPLICATE_SAME_ACCESS);
	_controller_thread.reset(x, &::CloseHandle);

	while (n--)
	{
		worker_data d = { nullptr, n, &_semaphore, _controller_thread.get() };
		_threads.push_back(make_pair(shared_ptr<void>(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0,
			&thread_proc, _exit.get(), 0, nullptr)), &::CloseHandle), d));
	}
}

parallel::~parallel()
{
	::SetEvent(_exit.get());
	for (threads::const_iterator i = _threads.begin(); i != _threads.end(); ++i)
		::WaitForSingleObject(i->first.get(), INFINITE);
}

void parallel::call(const std::function<void(size_t threadid)> &kernel)
{
	_semaphore = _threads.size();
	for (threads::iterator i = _threads.begin(); i != _threads.end(); ++i)
	{
		i->second.kernel = &kernel;
		::QueueUserAPC(&worker, i->first.get(), reinterpret_cast<ULONG_PTR>(&i->second));
	}
	::SleepEx(INFINITE, TRUE);
}

unsigned int __stdcall parallel::thread_proc(void *exit)
{
	while (::WaitForSingleObjectEx(exit, INFINITE, TRUE) != WAIT_OBJECT_0)
	{	}
	return 0;
}

void __stdcall parallel::worker(ULONG_PTR data_)
{
	struct local
	{
		static void __stdcall dummy(ULONG_PTR)
		{	}
	};

	const worker_data *data = reinterpret_cast<const worker_data *>(data_);

	(*data->kernel)(data->threadid);
	if (0 == ::InterlockedDecrement(data->semaphore))
		::QueueUserAPC(&local::dummy, data->controller_thread, 0);
}

int main()
{
	parallel p(4);

	for (size_t n = 1000000; n; --n)
	{
		p.call([] (size_t threadid) {
		});
	}
}
