#include <samples/common/timing.h>

#include <windows.h>

namespace
{
	double counter_period()
	{
		LARGE_INTEGER frequency;

		::QueryPerformanceFrequency(&frequency);
		return 1000.0 / frequency.QuadPart;
	}

	const double c_counter_period = counter_period();
}

double stopwatch(long long &counter_)
{
	double value;
	LARGE_INTEGER counter, current;

	counter.QuadPart = counter_;
	::QueryPerformanceCounter(&current);
	value = c_counter_period * (current.QuadPart - counter.QuadPart);
	counter_ = current.QuadPart;
	return value;
}
