#pragma once

#include <intrin.h>
#include <memory>
#include <windows.h>

#pragma warning(disable: 4355)

namespace agge
{
	class hybrid_event : noncopyable
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
		const HANDLE _native;
		volatile long _lock_state;
	};

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
		static DWORD __stdcall thread_proc(void *data);

	private:
		const count_t _id;
		const HANDLE _handle;
	};



	parallel::thread::thread(count_t id)
		: kernel(0), _id(id), _handle(::CreateThread(0, 0, &thread::thread_proc, this, 0, 0))
	{
		if (!_handle)
			throw std::bad_alloc();
	}

	parallel::thread::~thread()
	{
		kernel = 0;
		ready.set();
		::WaitForSingleObject(_handle, INFINITE);
		::CloseHandle(_handle);
	}

	DWORD parallel::thread::thread_proc(void *data)
	{
		thread *this_ = static_cast<thread *>(data);

		for (; this_->ready.wait(), this_->kernel; this_->kernel = 0, this_->done.set())
			(*this_->kernel)(this_->_id);
		return 0;
	}
}
