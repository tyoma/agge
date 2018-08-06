#include <agge/hybrid_event.h>

#include "intrinsic.h"
#include "semaphore.h"

namespace agge
{
	hybrid_event::hybrid_event()
		: _state(state_reset), _semaphore(new semaphore)
	{	}

	hybrid_event::~hybrid_event()
	{	delete _semaphore;	}
	
	void hybrid_event::signal()
	{
		if (interlocked_exchange(&_state, state_set) == state_blocked)
			_semaphore->signal();
	}

	void hybrid_event::wait()
	{
		for (;;)
		{
			for (count_t i = max_spin; i; --i)
			{
				if (interlocked_exchange(&_state, state_reset) == state_set)
					return;
				pause();
			}
			if (interlocked_compare_exchange(&_state, state_blocked, state_reset) == state_reset)
				_semaphore->wait();
		}
	}
}
