#include <samples/common/timing.h>

#include <time.h>

double stopwatch(long long &counter)
{
	auto c = clock();

	const auto dt = 1000.0f * (c - counter) / CLOCKS_PER_SEC;
	counter = c;
	return dt;
}
