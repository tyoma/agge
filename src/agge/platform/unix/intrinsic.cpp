#include "../../intrinsic.h"

#include <agge/config.h>

#ifdef AGGE_ARCH_INTEL
	#include <xmmintrin.h>
#endif

namespace agge
{
	long interlocked_compare_exchange(volatile long *destination, long new_value, long comparand)
	{	return __sync_val_compare_and_swap(destination, comparand, new_value);	}

	long interlocked_exchange(volatile long *destination, long new_value)
	{	return __sync_lock_test_and_set(destination, new_value);	}

	void pause()
	{
#if defined(AGGE_ARCH_INTEL)
					_mm_pause();
#elif AGGE_ARCH_ARM >= 7
					asm volatile ("yield" ::: "memory");
#endif
	}
}
