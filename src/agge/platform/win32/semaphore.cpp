#include "../../semaphore.h"

#include <windows.h>

namespace agge
{
	semaphore::semaphore()
		: _handle(::CreateSemaphore(0, 0, 1, 0))
	{	}

	semaphore::~semaphore()
	{	::CloseHandle(_handle);	}

	void semaphore::signal()
	{	::ReleaseSemaphore(_handle, 1, 0);	}

	void semaphore::wait()
	{	::WaitForSingleObject(_handle, INFINITE);	}
}
