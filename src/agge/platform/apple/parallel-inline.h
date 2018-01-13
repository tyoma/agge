#pragma once

#include <agge/config.h>
#include <pthread.h>
#include <dispatch/dispatch.h>

#ifdef AGGE_ARCH_INTEL
	#include <xmmintrin.h>
#endif

namespace agge
{
	class hybrid_event
	{
	public:
		enum { max_spin = 20000 };

	public:
		hybrid_event()
			: _native(dispatch_semaphore_create(1)), _lock_state(0)
		{	dispatch_semaphore_wait(_native, DISPATCH_TIME_FOREVER);	}

		~hybrid_event()
		{
			dispatch_semaphore_signal(_native);
			dispatch_release(_native);
		}

		void set()
		{
			if (__sync_val_compare_and_swap(&_lock_state, 0 /*if was not locked...*/, 1 /*... flag*/) == 2 /*was blocked*/)
				dispatch_semaphore_signal(_native);
		}

		void wait()
		{
			for (bool ready = false; !ready; )
			{
				for (long i = max_spin; !ready && i; --i)
				{
					ready = !!__sync_lock_test_and_set(&_lock_state, 0);
#if defined(AGGE_ARCH_INTEL)
					_mm_pause();
#elif AGGE_ARCH_ARM >= 7
					asm volatile ("yield" ::: "memory");
#endif
				}
				if (!ready && __sync_val_compare_and_swap(&_lock_state, 0 /*if was not flagged...*/, 2 /*... block*/) == 0 /*was not flagged*/)
					dispatch_semaphore_wait(_native, DISPATCH_TIME_FOREVER);
			}
		}

	private:
		dispatch_semaphore_t _native;
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
		static void *thread_proc(void *data);

	private:
		const count_t _id;
		pthread_t _thread;
	};



	inline parallel::thread::thread(count_t id)
		: kernel(0), _id(id)
	{
		if (pthread_create(&_thread, 0, &parallel::thread::thread_proc, this))
			throw std::bad_alloc();
	}

	inline parallel::thread::~thread()
	{
		kernel = 0;
		ready.set();
		pthread_join(_thread, 0);
	}

	inline void *parallel::thread::thread_proc(void *data)
	{
		thread *this_ = static_cast<thread *>(data);

		for (; this_->ready.wait(), this_->kernel; this_->kernel = 0, this_->done.set())
			(*this_->kernel)(this_->_id);
		return 0;
	}
}
