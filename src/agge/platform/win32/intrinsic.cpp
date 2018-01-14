#include "../../intrinsic.h"

#include <intrin.h>

namespace agge
{
	long interlocked_compare_exchange(volatile long *destination, long new_value, long comparand)
	{	return _InterlockedCompareExchange(destination, new_value, comparand);	}

	long interlocked_exchange(volatile long *destination, long new_value)
	{	return _InterlockedExchange(destination, new_value);	}

	void pause()
	{	_mm_pause();	}
}
