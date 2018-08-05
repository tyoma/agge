#include <agge/thread.h>

#include <new>
#include <windows.h>

namespace agge
{
	thread::thread(thread_function_t thread_function, void *argument)
		: _thread(HANDLE()), _thread_function(thread_function), _argument(argument)
	{
		struct starter
		{
			static DWORD __stdcall thread_proc(void *self_)
			{
				static_cast<thread *>(self_)->_thread_function(static_cast<thread *>(self_)->_argument);
				return 0;
			}
		};

		if (HANDLE h = ::CreateThread(0, 0, &starter::thread_proc, this, 0, 0))
			_thread = h;
		else
			throw std::bad_alloc();
	}

	thread::~thread()
	{
		::WaitForSingleObject(_thread, INFINITE);
		::CloseHandle(_thread);
	}
}
