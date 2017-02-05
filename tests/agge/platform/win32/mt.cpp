#include "../../mt.h"

#include <windows.h>

namespace agge
{
	namespace tests
	{
		mutex::mutex()
		{	::InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(_buffer));	}

		mutex::~mutex()
		{	::DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(_buffer));	}

		void mutex::lock()
		{	::EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(_buffer));	}

		void mutex::unlock()
		{	::LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(_buffer));	}


		thread_id this_thread_id()
		{	return ::GetCurrentThreadId();	}
	}
}
