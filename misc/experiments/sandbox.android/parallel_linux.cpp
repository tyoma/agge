#include <agge/parallel.h>

#include <semaphore.h>
#include <thread>

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
				: _lock_state(0)
			{	sem_init(&_native, 0, 0);	}

			~hybrid_event()
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
					for (long i = max_spin; !ready && i; --i)
					{
						ready = !!__sync_lock_test_and_set(&_lock_state, 0);
						asm volatile ("yield" ::: "memory");
					}
					if (!ready && __sync_val_compare_and_swap(&_lock_state, 0 /*if was not flagged...*/, 2 /*... block*/) == 0 /*was not flagged*/)
						sem_wait(&_native);
				}
			}

		private:
			sem_t _native;
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

	private:
		const count_t _id;
		shared_ptr<std::thread> _thread_object;
	};



	parallel::thread::thread(count_t id)
		: kernel(0), _id(id)
	{
		_thread_object = make_shared<std::thread>([this] () {
			for (; ready.wait(), kernel; kernel = 0, done.set())
				(*kernel)(_id);
		});
	}

	parallel::thread::~thread()
	{
		kernel = 0;
		ready.set();
		_thread_object->join();
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
