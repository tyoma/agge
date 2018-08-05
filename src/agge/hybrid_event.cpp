#include <agge/hybrid_event.h>

#include "intrinsic.h"
#include "semaphore.h"

namespace agge
{
	hybrid_event::hybrid_event()
		: _state(state_free), _semaphore(new semaphore)
	{	}

	hybrid_event::~hybrid_event()
	{	delete _semaphore;	}
	
	void hybrid_event::signal()
	{
		if (interlocked_compare_exchange(&_state, state_set, state_free) == state_blocked)
			_semaphore->signal();
	}

	void hybrid_event::wait()
	{
		for (bool ready = false; !ready; )
		{
			for (count_t i = max_spin; !ready && i; --i)
			{
				ready = interlocked_exchange(&_state, state_free) != state_free;
				if (!ready)
					pause();
			}
			if (!ready && interlocked_compare_exchange(&_state, state_blocked, state_free) == state_free)
				_semaphore->wait();
		}
	}
}
