#include "../../mt.h"

#include <memory>
#include <pthread.h>

namespace agge
{
	namespace tests
	{
		namespace
		{
			pthread_mutex_t *cast(char *buffer)
			{ return reinterpret_cast<pthread_mutex_t *>(buffer); }
		}

		mutex::mutex()
		{
			if (pthread_mutex_init(cast(_buffer), 0))
				throw std::bad_alloc();
		}

		mutex::~mutex()
		{	pthread_mutex_destroy(cast(_buffer));	}

		void mutex::lock()
		{	pthread_mutex_lock(cast(_buffer));	}

		void mutex::unlock()
		{	pthread_mutex_unlock(cast(_buffer));	}


		thread_id this_thread_id()
		{ return reinterpret_cast<thread_id>(pthread_self()); }
	}
}
