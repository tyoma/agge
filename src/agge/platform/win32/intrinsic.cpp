#include "../../intrinsic.h"

#include <agge/config.h>

#include <intrin.h>

namespace agge
{
	long interlocked_compare_exchange(volatile long *destination, long new_value, long comparand)
	{	return _InterlockedCompareExchange(destination, new_value, comparand);	}

	long interlocked_exchange(volatile long *destination, long new_value)
	{	return _InterlockedExchange(destination, new_value);	}

	void pause()
	{
#if defined(AGGE_ARCH_INTEL)
		_mm_pause();
#elif defined(AGGE_ARCH_ARM)
		__yield();
#endif
	}
}
