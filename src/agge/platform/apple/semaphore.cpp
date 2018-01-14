#include "../../hybrid_event.h"

#include <dispatch/dispatch.h>

namespace agge
{
	semaphore::semaphore()
		: _handle(::dispatch_semaphore_create(1))
	{	wait();	}

	semaphore::~semaphore()
	{
		signal();
		::dispatch_release(_handle);
	}

	void semaphore::signal()
	{	::dispatch_semaphore_signal(_handle);	}

	void semaphore::wait()
	{	::dispatch_semaphore_wait(_handle, DISPATCH_TIME_FOREVER);	}
}
