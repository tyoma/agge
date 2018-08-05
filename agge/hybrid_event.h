#pragma once

#include <agge/types.h>

namespace agge
{
	class semaphore;
	
	class hybrid_event : noncopyable
	{
	public:
		hybrid_event();
		~hybrid_event();

		void signal();
		void wait();

	private:
		enum { max_spin = 20000 };
		enum { state_free, state_set, state_blocked };
		
	private:
		volatile long _state;
		semaphore *_semaphore;
	};
}
