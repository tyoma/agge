#pragma once

namespace agge
{
	long interlocked_compare_exchange(volatile long *destination, long new_value, long comparand);
	long interlocked_exchange(volatile long *destination, long new_value);
	void pause();
}
