#include "../../hybrid_event.h"

#include <semaphore.h>

namespace agge
{
	semaphore::semaphore()
		: _handle(sem_t())
	{	::sem_init(_handle.address_of<sem_t>(), 0, 0);	}

	semaphore::~semaphore()
	{	::sem_destroy(_handle.address_of<sem_t>());	}

	void semaphore::signal()
	{	::sem_post(_handle.address_of<sem_t>());	}
	
	void semaphore::wait()
	{	::sem_wait(_handle.address_of<sem_t>());	}
}
