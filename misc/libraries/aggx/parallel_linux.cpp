#include "parallel.h"

#include <semaphore.h>
#include <thread>

using namespace std;

namespace aggx
{
	class parallel::event
	{
	public:
		event()
			: _lock_state(0)
		{	sem_init(&_native, 0, 0);	}

		event(const event &/*other*/)
			: _lock_state(0)
		{	sem_init(&_native, 0, 0);	}

		~event()
		{	sem_destroy(&_native);	}

		void set()
		{
			if (__sync_val_compare_and_swap(&_lock_state, 0 /*if was not locked...*/, 1 /*... flag*/) == 2 /*was blocked*/)
				sem_post(&_native);
		}

		void wait()
		{
			for (bool ready = false; !ready; )
			{
				for (long i = 20000; !ready && i; --i)
				{
					ready = !!__sync_lock_test_and_set(&_lock_state, 0);
	//					_yield();
				}
				if (!ready && __sync_val_compare_and_swap(&_lock_state, 0 /*if was not flagged...*/, 2 /*... block*/) == 0 /*was not flagged*/)
					sem_wait(&_native);
			}
		}

	private:
		sem_t _native;
		volatile long _lock_state;
	};

	struct parallel::worker_data
	{
		const unsigned int threadid;
		const parallel::kernel_function *kernel;
		parallel::event run;
		parallel::event done;
	};

	parallel::parallel(unsigned int threads)
	{
		_threads.reserve(threads - 1);

		while (--threads)
		{
			worker_data wd = { threads, nullptr };

			_threads.push_back(make_pair(nullptr, wd));

			thread_data *td = &_threads.back();

			td->first = make_shared<thread>([td]() {
				worker_data &data = td->second;

				for (; data.run.wait(), data.kernel; data.kernel = nullptr, data.done.set())
					(*data.kernel)(data.threadid);
			});
		}
	}

	parallel::~parallel()
	{
		for (threads::iterator i = _threads.begin(); i != _threads.end(); ++i)
		{
			i->second.run.set();
			static_pointer_cast<thread>(i->first)->join();
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
